/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, University of Toronto
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the University of Toronto nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

// Authors: Jonathan Gammell, Marlin Strub

#include "esp_planning_contexts/reeds_shepp_random_rectangles.h"

#include <vector>

#include <ompl/base/StateValidityChecker.h>
#include <ompl/base/goals/GoalSpace.h>
#include <ompl/base/goals/GoalState.h>
#include <ompl/base/goals/GoalStates.h>

#include "esp_obstacles/base_obstacle.h"
#include "esp_obstacles/hyperrectangle.h"
#include "esp_planning_contexts/reeds_shepp_validity_checker.h"

using namespace std::string_literals;

namespace esp {

namespace ompltools {

ReedsSheppRandomRectangles::ReedsSheppRandomRectangles(
    const std::shared_ptr<ompl::base::SpaceInformation>& spaceInfo,
    const std::shared_ptr<const Configuration>& config, const std::string& name) :
    BaseContext(spaceInfo, config, name),
    bounds_(2u),
    numRectangles_(config->get<std::size_t>("context/" + name + "/numObstacles")),
    minSideLength_(config->get<double>("context/" + name + "/minSideLength")),
    maxSideLength_(config->get<double>("context/" + name + "/maxSideLength")),
    realVectorSubspaceInfo_(std::make_shared<ompl::base::SpaceInformation>(
        spaceInfo_->getStateSpace()->as<ompl::base::SE2StateSpace>()->getSubspace(0u))) {
  
  startGoalPair_ = makeStartGoalPair();

  if (minSideLength_ > maxSideLength_) {
    OMPL_ERROR("%s: Specified min side length is greater than specified max side length.",
               name.c_str());
    throw std::runtime_error("Context error.");
  }

  // Fill the state space bounds.
  auto sideLengths = config->get<std::vector<double>>("context/" + name + "/boundarySideLengths");
  assert(sideLengths.size() == 2u);
  for (std::size_t dim = 0u; dim < 2u; ++dim) {
    bounds_.low.at(dim) = -0.5 * sideLengths.at(dim);
    bounds_.high.at(dim) = 0.5 * sideLengths.at(dim);
  }

  // Create the obstacles and add them to the validity checker.
  createObstacles();

  // Create the validity checker.
  auto validityChecker = std::make_shared<ReedsSheppValidityChecker>(spaceInfo_);

  // Add the obstacles to the validity checker.
  validityChecker->addObstacles(obstacles_);

  // Set the validity checker and the check resolution.
  spaceInfo_->setStateValidityChecker(validityChecker);
  spaceInfo_->setStateValidityCheckingResolution(
      config->get<double>("context/" + name + "/collisionCheckResolution"));

  // Set up the space info.
  spaceInfo_->setup();
}

ompl::base::ScopedState<ompl::base::SE2StateSpace> ReedsSheppRandomRectangles::getStartState()
    const {
  return startGoalPair_.start.at(0);
}

ompl::base::RealVectorBounds ReedsSheppRandomRectangles::getBoundaries() const {
  return bounds_;
}

void ReedsSheppRandomRectangles::accept(const ContextVisitor& visitor) const {
  visitor.visit(*this);
}

void ReedsSheppRandomRectangles::createObstacles() {
  // Create a goal to make sure the obstacles don't invalidate it. Something seems funky here, there
  // has to be a better way to do this? The problem is that I want to create a new goal whenever I
  // create a new problem definition, so the goal can not be created in the base class and kept
  // around.
  auto goal = startGoalPair_.goal;

  // Instantiate obstacles.
  for (int i = 0; i < static_cast<int>(numRectangles_); ++i) {
    // Create a random anchor (uniform).
    ompl::base::ScopedState<> anchor(realVectorSubspaceInfo_);
    anchor.random();

    // Create random widths (uniform).
    std::vector<double> widths(2u, 0.0);
    for (std::size_t j = 0; j < 2u; ++j) {
      widths[j] = rng_.uniformReal(minSideLength_, maxSideLength_);
    }
    auto obstacle =
        std::make_shared<Hyperrectangle<BaseObstacle>>(realVectorSubspaceInfo_, anchor, widths);

    auto validityChecker = std::make_shared<ReedsSheppValidityChecker>(spaceInfo_);
    validityChecker->addObstacle(obstacle);

    // Add this to the obstacles if it doesn't invalidate the start or goal state.
    if (validityChecker->isValid(startGoalPair_.start.at(0).get())) {
      if (goalType_ == ompl::base::GoalType::GOAL_STATE) {
        if (validityChecker->isValid(goal->as<ompl::base::GoalState>()->getState())) {
          obstacles_.emplace_back(obstacle);
        } else {
          --i;
        }
      } else if (goalType_ == ompl::base::GoalType::GOAL_STATES) {
        auto invalidates = false;
        for (auto i = 0u; i < goal->as<ompl::base::GoalStates>()->getStateCount(); ++i) {
          if (validityChecker->isValid(goal->as<ompl::base::GoalStates>()->getState(i))) {
            invalidates = true;
            break;
          }
        }
        if (!invalidates) {
          obstacles_.emplace_back(obstacle);
        } else {
          --i;
        }
      }
    } else {
      --i;
    }
  }
}

std::shared_ptr<ompl::base::Goal> ReedsSheppRandomRectangles::createGoal() const {
  switch (goalType_) {
    case ompl::base::GoalType::GOAL_STATE: {
      // Get the goal position.
      const auto goalPosition = config_->get<std::vector<double>>("context/" + name_ + "/goal");

      // Check dimensionality of the goal state position.
      if (goalPosition.size() != dimensionality_) {
        OMPL_ERROR("%s: Dimensionality of problem and of goal specification does not match.",
                   name_.c_str());
        throw std::runtime_error("Context error.");
      }

      // Allocate a goal state and set the position.
      ompl::base::ScopedState<ompl::base::SE2StateSpace> goalState(spaceInfo_);

      // Fill the goal state's coordinates.
      goalState->setX(goalPosition.at(0u));
      goalState->setY(goalPosition.at(1u));
      goalState->setYaw(goalPosition.at(2u));

      // Register the goal state with the goal.
      auto goal = std::make_shared<ompl::base::GoalState>(spaceInfo_);
      goal->as<ompl::base::GoalState>()->setState(goalState);
      return goal;
    }
    case ompl::base::GoalType::GOAL_STATES: {
      const auto numGoals = config_->get<unsigned>("context/" + name_ + "/numGoals");
      ompl::base::ScopedState<ompl::base::SE2StateSpace> goalState(spaceInfo_);
      auto goal = std::make_shared<ompl::base::GoalStates>(spaceInfo_);
      for (auto i = 0u; i < numGoals; ++i) {
        goalState.random();
        goal->as<ompl::base::GoalStates>()->addState(goalState);
      }
      return goal;
    }
    case ompl::base::GoalType::GOAL_SPACE: {
      // Get the goal bounds.
      ompl::base::RealVectorBounds goalBounds(static_cast<unsigned>(2u));
      goalBounds.low = config_->get<std::vector<double>>("context/" + name_ + "/goalLowerBounds");
      goalBounds.high = config_->get<std::vector<double>>("context/" + name_ + "/goalUpperBounds");

      // Generate a goal space.
      auto goalSpace = std::make_shared<ompl::base::RealVectorStateSpace>(2u);

      // Set the goal bounds.
      goalSpace->setBounds(goalBounds);

      // Let the goal know about the goal space.
      auto goal = std::make_shared<ompl::base::GoalSpace>(spaceInfo_);
      goal->as<ompl::base::GoalSpace>()->setSpace(goalSpace);
      return goal;
    }
    default: { throw std::runtime_error("Goal type not implemented."); }
  }
}

std::vector<std::shared_ptr<BaseObstacle>> ReedsSheppRandomRectangles::getObstacles() const {
  return obstacles_;
}

std::vector<std::shared_ptr<BaseAntiObstacle>> ReedsSheppRandomRectangles::getAntiObstacles()
    const {
  return {};
}

}  // namespace ompltools

}  // namespace esp

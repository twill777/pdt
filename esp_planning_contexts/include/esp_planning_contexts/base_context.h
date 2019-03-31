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

#pragma once

#include <memory>
#include <utility>

#include <ompl/base/Cost.h>
#include <ompl/base/OptimizationObjective.h>
#include <ompl/base/ProblemDefinition.h>
#include <ompl/base/ScopedState.h>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/StateSpace.h>
#include <ompl/datastructures/NearestNeighborsGNAT.h>
#include <ompl/util/Console.h>
#include <ompl/util/Exception.h>

#include "esp_obstacles/base_obstacle.h"
#include "esp_planning_contexts/context_visitor.h"
#include "esp_time/time.h"

namespace esp {

namespace ompltools {

/** \brief The base class for an experiment */
class BaseContext {
 public:
  BaseContext(const unsigned int dim, const std::vector<std::pair<double, double>> limits,
              const double runSeconds, std::string name);

  // Use default virtual destructor.
  virtual ~BaseContext() = default;

  /** \brief Return the space information pointer */
  ompl::base::SpaceInformationPtr getSpaceInformation() const;

  /** \brief Return the space information pointer */
  ompl::base::StateSpacePtr getStateSpace() const;

  /** \brief Return a newly generated problem definition */
  ompl::base::ProblemDefinitionPtr newProblemDefinition() const;

  /** \brief Return the optimization objective */
  ompl::base::OptimizationObjectivePtr getOptimizationObjective() const;

  /** \brief Return the maximum experiment runtime */
  time::Duration getTargetDuration() const;

  /** \brief Get the goal */
  ompl::base::GoalPtr getGoalPtr() const;

  /** \brief Get the starts */
  std::vector<ompl::base::ScopedState<>> getStartStates() const;

  /** \brief Get the goals */
  std::vector<ompl::base::ScopedState<>> getGoalStates() const;

  /** \brief Get the name */
  std::string getName() const;

  /** \brief Get the state-space limit */
  std::vector<std::pair<double, double>> getLimits() const;

  /** \brief Get the dimensionality of the underlying search space */
  unsigned int getDimensions() const;

  /** \brief The global minimum that may or may not be attainable in a problem. */
  ompl::base::Cost getMinimum() const;

  /** \brief Print out summary of the experiment */
  void print(const bool verbose = false) const;

  // Accept a visitor.
  virtual void accept(const ContextVisitor& visitor) const = 0;

  /** \brief Whether the problem has an exact expression for the optimum */
  virtual bool knowsOptimum() const = 0;

  /** \brief Returns the global optimum if known, otherwise throws. */
  virtual ompl::base::Cost computeOptimum() const = 0;

  /** \brief Set the target cost. How this is defined depends on the specific experiment. */
  virtual void setTarget(double targetSpecifier) = 0;

  /** \brief Derived class specific information to include in the title line. */
  virtual std::string lineInfo() const = 0;

  /** \brief Derived class specific information to include at the end. */
  virtual std::string paraInfo() const = 0;

 protected:
  /** \brief The context name */
  std::string name_{"UnnamedContext"};
  /** \brief The problem dimension */
  unsigned int dimensionality_{0u};
  /** \brief The problem limits */
  std::vector<std::pair<double, double>> bounds_{};
  /** \brief The space information for the experiment */
  ompl::base::SpaceInformationPtr spaceInfo_{};
  /** \brief The optimization objective */
  ompl::base::OptimizationObjectivePtr optimizationObjective_{};
  /** \brief The runtime for the experiment */
  time::Duration targetDuration_{};
  /** \brief The start states */
  std::vector<ompl::base::ScopedState<>> startStates_{};
  /** \brief The goal states */
  std::vector<ompl::base::ScopedState<>> goalStates_{};
  /** \brief The goal as a pointer*/
  ompl::base::GoalPtr goalPtr_{};
};

using BaseContextPtr = std::shared_ptr<BaseContext>;

}  // namespace ompltools

}  // namespace esp

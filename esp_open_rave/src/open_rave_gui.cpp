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

// Authors: Marlin Strub

#include <dbg.h>

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <ompl/geometric/PathGeometric.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <openrave-core.h>
#include <openrave/viewer.h>
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include "esp_common/context_type.h"
#include "esp_common/planner_type.h"
#include "esp_configuration/configuration.h"
#include "esp_factories/context_factory.h"
#include "esp_factories/planner_factory.h"
#include "esp_open_rave/open_rave_base_context.h"
#include "esp_open_rave/open_rave_manipulator.h"
#include "esp_open_rave/open_rave_manipulator_validity_checker.h"
#include "esp_open_rave/open_rave_se3.h"
#include "esp_open_rave/open_rave_se3_validity_checker.h"
#include "esp_planning_contexts/all_contexts.h"
#include "esp_planning_contexts/context_visitor.h"

using namespace std::string_literals;
using namespace std::chrono_literals;

void planManipulator(std::shared_ptr<esp::ompltools::Configuration> config,
                     std::shared_ptr<esp::ompltools::OpenRaveManipulator> context) {
  esp::ompltools::PlannerFactory plannerFactory(config, context);
  auto [planner, plannerType] =
      plannerFactory.create(config->get<std::string>("Experiment/planner"));
  (void)plannerType;

  // Setup the planner.
  planner->setup();

  // Get the environment.
  auto environment = std::dynamic_pointer_cast<esp::ompltools::OpenRaveManipulatorValidityChecker>(
                         context->getSpaceInformation()->getStateValidityChecker())
                         ->getOpenRaveEnvironment();

  // Get the robot.
  auto robot =
      environment->GetRobot(config->get<std::string>("Contexts/" + context->getName() + "/robot"));

  // Create the vector to hold the current state.
  std::vector<double> openRaveState =
      config->get<std::vector<double>>("Contexts/" + context->getName() + "/start");

  double totalSolveDuration = 0.0;
  // Work it.
  while (true) {
    // Work on the problem.
    planner->solve(config->get<double>("Experiment/visualizationInterval"));

    // Update the total solve duration.
    totalSolveDuration += config->get<double>("Experiment/visualizationInterval");

    // Check if the planner found a solution yet.
    if (planner->getProblemDefinition()->hasExactSolution()) {
      // Get the solution of the planner.
      auto solution =
          planner->getProblemDefinition()->getSolutionPath()->as<ompl::geometric::PathGeometric>();

      // Report the cost of the solution.
      std::cout << "[ " << totalSolveDuration << "s ] "
                << config->get<std::string>("Experiment/planner") << " found a solution of cost "
                << solution->cost(planner->getProblemDefinition()->getOptimizationObjective())
                << '\n';

      // Interpolate ("approx to collision checking resolution").
      solution->interpolate();

      // Get the solution states.
      auto solutionStates = solution->getStates();

      // Visualize the solution.
      for (const auto solutionState : solutionStates) {
        for (std::size_t i = 0u; i < openRaveState.size(); ++i) {
          openRaveState[i] =
              solutionState->as<ompl::base::RealVectorStateSpace::StateType>()->operator[](i);
        }
        OpenRAVE::EnvironmentMutex::scoped_lock lock(environment->GetMutex());
        robot->SetActiveDOFValues(openRaveState);
        std::this_thread::sleep_for(0.05s);
      }
    } else {
      std::cout << "[ " << totalSolveDuration << "s ] "
                << config->get<std::string>("Experiment/planner")
                << " did not find a solution yet.\n";
    }
  }
}

void planMover(std::shared_ptr<esp::ompltools::Configuration> config,
               std::shared_ptr<esp::ompltools::OpenRaveSE3> context) {
  esp::ompltools::PlannerFactory plannerFactory(config, context);
  auto [planner, plannerType] =
      plannerFactory.create(config->get<std::string>("Experiment/planner"));
  (void)plannerType;

  // Setup the planner.
  planner->setup();

  // Get the environment.
  auto environment = std::dynamic_pointer_cast<esp::ompltools::OpenRaveSE3ValidityChecker>(
                         context->getSpaceInformation()->getStateValidityChecker())
                         ->getOpenRaveEnvironment();

  // Get the robot.
  auto robot =
      environment->GetRobot(config->get<std::string>("Contexts/" + context->getName() + "/robot"));

  // Create the vector to hold the current state.
  OpenRAVE::Transform raveState;

  double totalSolveDuration = 0.0;
  // Work it.
  while (true) {
    // Work on the problem.
    planner->solve(config->get<double>("Experiment/visualizationInterval"));

    // Update the total solve duration.
    totalSolveDuration += config->get<double>("Experiment/visualizationInterval");

    // Check if the planner found a solution yet.
    if (planner->getProblemDefinition()->hasExactSolution()) {
      // Get the solution of the planner.
      auto solution =
          planner->getProblemDefinition()->getSolutionPath()->as<ompl::geometric::PathGeometric>();

      // Report the cost of the solution.
      std::cout << "[ " << totalSolveDuration << "s ] "
                << config->get<std::string>("Experiment/planner") << " found a solution of cost "
                << solution->cost(planner->getProblemDefinition()->getOptimizationObjective())
                << '\n';

      // Interpolate ("approx to collision checking resolution").
      solution->interpolate();

      // Get the solution states.
      auto solutionStates = solution->getStates();

      // Visualize the solution.
      for (const auto solutionState : solutionStates) {
        auto se3State = solutionState->as<ompl::base::SE3StateSpace::StateType>();
        raveState.trans.Set3(se3State->getX(), se3State->getY(), se3State->getZ());
        raveState.rot.x = se3State->rotation().x;
        raveState.rot.y = se3State->rotation().y;
        raveState.rot.z = se3State->rotation().z;
        raveState.rot.w = se3State->rotation().w;

        OpenRAVE::EnvironmentMutex::scoped_lock lock(environment->GetMutex());
        robot->SetTransform(raveState);
        std::this_thread::sleep_for(0.05s);
      }
    } else {
      std::cout << "[ " << totalSolveDuration << "s ] "
                << config->get<std::string>("Experiment/planner")
                << " did not find a solution yet.\n";
    }
  }
}

int main(int argc, char** argv) {
  dbg("starting");
  // Instantiate the config.
  auto config = std::make_shared<esp::ompltools::Configuration>(argc, argv);
  config->registerAsExperiment();

  dbg("creating context");
  // Create the context.
  auto contextFactory = std::make_shared<esp::ompltools::ContextFactory>(config);
  auto context = contextFactory->create(config->get<std::string>("Experiment/context"));

  if (std::dynamic_pointer_cast<esp::ompltools::OpenRaveManipulator>(context)) {
    // Get the environment.
    auto environment =
        std::dynamic_pointer_cast<esp::ompltools::OpenRaveManipulatorValidityChecker>(
            context->getSpaceInformation()->getStateValidityChecker())
            ->getOpenRaveEnvironment();

    dbg("Manipulator context");
    // Create the viewer.
    auto viewer = OpenRAVE::RaveCreateViewer(environment, "qtosg");

    auto planThread =
        std::thread(&planManipulator, config,
                    std::dynamic_pointer_cast<esp::ompltools::OpenRaveManipulator>(context));

    viewer->main(true);

    while (!planThread.joinable()) {
      std::this_thread::sleep_for(0.5s);
    }

    planThread.join();
  } else if (std::dynamic_pointer_cast<esp::ompltools::OpenRaveSE3>(context)) {
    dbg("SE3 context");
    // Get the environment.
    auto environment = std::dynamic_pointer_cast<esp::ompltools::OpenRaveSE3ValidityChecker>(
                           context->getSpaceInformation()->getStateValidityChecker())
                           ->getOpenRaveEnvironment();

    // Create the viewer.
    auto viewer = OpenRAVE::RaveCreateViewer(environment, "qtosg");

    auto planThread = std::thread(&planMover, config,
                                  std::dynamic_pointer_cast<esp::ompltools::OpenRaveSE3>(context));

    viewer->main(true);

    while (!planThread.joinable()) {
      std::this_thread::sleep_for(0.5s);
    }

    planThread.join();
  } else {
    throw std::runtime_error("Cannot process non-openrave context.");
  }

  return 0;
}

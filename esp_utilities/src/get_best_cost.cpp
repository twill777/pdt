/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014--2022
 *  Estimation, Search, and Planning (ESP) Research Group
 *  All rights reserved
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
 *   * Neither the names of the organizations nor the names of its
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

#include "esp_utilities/get_best_cost.h"

#include <limits>
#include <string>

#include <ompl/geometric/planners/informedtrees/ABITstar.h>
#include <ompl/geometric/planners/informedtrees/AITstar.h>
#include <ompl/geometric/planners/informedtrees/BITstar.h>
#include <ompl/geometric/planners/informedtrees/EITstar.h>
#include <ompl/geometric/planners/informedtrees/EIRMstar.h>
#include <ompl/geometric/planners/prm/LazyPRMstar.h>
#include <ompl/geometric/planners/rrt/InformedRRTstar.h>
#include <ompl/geometric/planners/rrt/LBTRRT.h>
#include <ompl/geometric/planners/prm/PRMstar.h>
#include <ompl/geometric/planners/rrt/RRT.h>
#include <ompl/geometric/planners/rrt/RRTConnect.h>
#include <ompl/geometric/planners/rrt/RRTsharp.h>
#include <ompl/geometric/planners/rrt/RRTstar.h>
#include <ompl/geometric/planners/prm/SPARStwo.h>

#include "esp_common/planner_type.h"

namespace esp {

namespace pdt {

namespace utilities {

ompl::base::Cost getBestCost(const ompl::base::PlannerPtr& planner, PLANNER_TYPE plannerType) {
  switch (plannerType) {
    case PLANNER_TYPE::ABITSTAR: {
      return planner->as<ompl::geometric::ABITstar>()->bestCost();
    }
    case PLANNER_TYPE::AITSTAR: {
      return planner->as<ompl::geometric::AITstar>()->bestCost();
    }
    case PLANNER_TYPE::BITSTAR: {
      return planner->as<ompl::geometric::BITstar>()->bestCost();
    }
    case PLANNER_TYPE::EIRMSTAR: {
      return planner->as<ompl::geometric::EIRMstar>()->bestCost();
    }
    case PLANNER_TYPE::EITSTAR: {
      return planner->as<ompl::geometric::EITstar>()->bestCost();
    }
    case PLANNER_TYPE::FMTSTAR: {
      return ompl::base::Cost(std::numeric_limits<double>::infinity());
    }
    case PLANNER_TYPE::INFORMEDRRTSTAR: {
      return planner->as<ompl::geometric::InformedRRTstar>()->bestCost();
    }
    case PLANNER_TYPE::LAZYPRMSTAR: {
      return planner->as<ompl::geometric::LazyPRMstar>()->bestCost();
    }
    case PLANNER_TYPE::LBTRRT: {
      return planner->as<ompl::geometric::LBTRRT>()->bestCost();
    }
    case PLANNER_TYPE::PRMSTAR: {
      return planner->as<ompl::geometric::PRMstar>()->bestCost();
    }
    case PLANNER_TYPE::RRT: {
      return ompl::base::Cost(std::numeric_limits<double>::infinity());
    }
    case PLANNER_TYPE::RRTCONNECT: {
      return ompl::base::Cost(std::numeric_limits<double>::infinity());
    }
    case PLANNER_TYPE::RRTSHARP: {
      return planner->as<ompl::geometric::RRTsharp>()->bestCost();
    }
    case PLANNER_TYPE::RRTSTAR: {
      return planner->as<ompl::geometric::RRTstar>()->bestCost();
    }
    case PLANNER_TYPE::SPARSTWO: {
      return planner->as<ompl::geometric::SPARStwo>()->bestCost();
    }
    default: {
      throw std::runtime_error("Received request to get best cost of unknown planner type.");
    }
  }
}

}  // namespace utilities

}  // namespace pdt

}  // namespace esp

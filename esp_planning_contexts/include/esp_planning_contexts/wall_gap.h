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

/* Authors: Jonathan Gammell */

#pragma once

#include "esp_obstacles/cutout_obstacles.h"
#include "esp_planning_contexts/base_context.h"

namespace esp {

namespace ompltools {

/** \brief An two-homotopy-class experiment that scales to N dimensions. I.e., an obstacle with a
 * gap that is anchored to the map limit on one side. */
class WallGap : public BaseContext {
 public:
  /** \brief Constructor */
  WallGap(const unsigned int dim, const bool onlyFindGap, const double gapWidth,
          const double gapOffset, const double flankWidth, const double runSeconds,
          const double checkResolution);

  /** \brief This problem knows its optimum */
  virtual bool knowsOptimum() const;

  /** \brief Returns the global optimum, which is through the gap, regardless of the stopping
   * condition specified at construction. */
  virtual ompl::base::Cost getOptimum() const;

  /** \brief If \e not stopping when a path is found through the gap, set the target cost as the
   * specified multiplier of the optimum. Otherwise throws*/
  virtual void setTarget(double targetSpecifier);

  /** \brief Derived class specific information to include in the title line. */
  virtual std::string lineInfo() const;

  /** \brief Derived class specific information to include at the end. */
  virtual std::string paraInfo() const;

  /** \brief Get the minimum cost that is flanking the obstacle. */
  ompl::base::Cost minFlankingCost() const;

  /** \brief Get the maximum (straight line path) cost that is through the gap. */
  ompl::base::Cost maxGapCost() const;

 protected:
  /** \brief Whether to stop on class switch */
  bool stopClassSwitch_{false};
  /** \brief The obstacle world */
  std::shared_ptr<CutoutObstacles> rectObs_{};
  /** \brief The lower-left corners of the obstacles*/
  std::shared_ptr<ompl::base::ScopedState<>> gapLowerLeftCorner_{};
  std::shared_ptr<ompl::base::ScopedState<>> obstacleLowerLeftCorner_{};
  /** The widths of the obstacles */
  std::vector<double> gapWidths_{};
  std::vector<double> obstacleWidths_{};

  /** \brief The basic thickness of the obstacle. */
  double obsThickness_{0.25};
  /** \brief The gap width. */
  double gapWidth_{0.0};
  /** \brief The gap offset. */
  double gapOffset_{0.0};
  /** \brief The flank width. */
  double flankWidth_{0.0};
  /** \brief The start and goal positions */
  double startPos_{-0.5};
  double goalPos_{0.5};
};

}  // namespace ompltools

}  // namespace esp

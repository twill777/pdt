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

#pragma once

#include <experimental/filesystem>
#include <memory>
#include <string>

#include "esp_configuration/configuration.h"
#include "esp_plotters/latex_plotter.h"
#include "esp_statistics/multiquery_statistics.h"
#include "esp_tikz/pgf_axis.h"

namespace pdt {

namespace plotters {

class SuccessAtTimeVsQueryLinePlotter : public LatexPlotter {
 public:
  SuccessAtTimeVsQueryLinePlotter(const std::shared_ptr<const config::Configuration>& config, const statistics::MultiqueryStatistics& stats);
  ~SuccessAtTimeVsQueryLinePlotter() = default;

  // Creates a pgf axis that holds the success rate at a given percentage of the total time for all planners.
  // 'percentage' currently only allows for [25, 50, 75, 100] as values, i.e. quartiles.
  std::shared_ptr<pgftikz::PgfAxis> createSuccessRateQueryAxis(const unsigned int percentage) const;

  // Creates a pgf axis that holds the success rate at a given percentage of the total time for the specified planner.
  // 'percentage' currently only allows for [25, 50, 75, 100] as values, i.e. quartiles.
  std::shared_ptr<pgftikz::PgfAxis> createSuccessRateQueryAxis(
      const std::string& plannerName, const unsigned int percentage) const;

  // Creates a tikz picture that contains the success rate axis of all planners.
  // 'percentage' currently only allows for [25, 50, 75, 100] as values, i.e. quartiles.
  std::experimental::filesystem::path createSuccessRateQueryPicture(const unsigned int percentage) const;

  // Creates a tikz picture that contains the success rate axis of the specified planner.
  // 'percentage' currently only allows for [25, 50, 75, 100] as values, i.e. quartiles.
  std::experimental::filesystem::path createSuccessRateQueryPicture(const std::string& plannerName, const unsigned int percentage) const;

 private:
  std::shared_ptr<pgftikz::PgfPlot> createSuccessRateQueryPercentPlot(
      const std::string& plannerName, const unsigned int percentage) const;

  void setSuccessRateQueryAxisOptions(std::shared_ptr<pgftikz::PgfAxis> axis) const;

  const statistics::MultiqueryStatistics& stats_;
};

}  // namespace plotters

}  // namespace pdt

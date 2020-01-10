/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2018, University of Oxford
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
 *   * Neither the name of the University of Oxford nor the names of its
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

#include "esp_tikz/overview_plotter.h"

#include "esp_tikz/median_cost_evolution_plotter.h"
#include "esp_tikz/pgf_axis.h"
#include "esp_tikz/pgf_fillbetween.h"
#include "esp_tikz/pgf_plot.h"
#include "esp_tikz/pgf_table.h"
#include "esp_tikz/success_plotter.h"
#include "esp_tikz/tikz_picture.h"

namespace esp {

namespace ompltools {

using namespace std::string_literals;
namespace fs = std::experimental::filesystem;

OverviewPlotter::OverviewPlotter(const std::shared_ptr<const Configuration>& config,
                                 const Statistics& stats) :
    LatexPlotter(config),
    stats_(stats) {
}

fs::path OverviewPlotter::createCombinedPicture() const {
  // Create the success axis and override some options.
  SuccessPlotter successPlotter(config_, stats_);
  auto successAxis = successPlotter.createSuccessAxis();
  successAxis->options.name = "AllPlannersCombinedSuccessAxis"s;
  successAxis->options.xlabel = "{\\empty}"s;
  successAxis->options.xticklabel = "{\\empty}"s;

  // Create the median cost axis and override some options.
  MedianCostEvolutionPlotter medianCostPlotter(config_, stats_);
  auto medianCostAxis = medianCostPlotter.createMedianCostEvolutionAxis();
  medianCostAxis->options.at = "($(AllPlannersCombinedSuccessAxis.south) - (0.0em, 0.3em)$)";
  medianCostAxis->options.anchor = "north";
  medianCostAxis->options.name = "AllPlannersCombinedMedianCostAxis"s;

  // Make sure these axis cover the same domain.
  PgfAxis::alignAbszissen(medianCostAxis.get(), successAxis.get());
  
  // Create the picture and add the axes.
  TikzPicture picture(config_);
  picture.addAxis(successAxis);
  picture.addAxis(medianCostAxis);

  // Generate the tikz file.
  auto picturePath = fs::path(config_->get<std::string>("experiment/results")).parent_path() /
                     fs::path("tikz/all_planners_combined_success_median_cost_plot.tikz");
  picture.write(picturePath);
  return picturePath;
}

fs::path OverviewPlotter::createCombinedPicture(const std::string& plannerName) const {
  // Create the success axis and override some options.
  SuccessPlotter successPlotter(config_, stats_);
  auto successAxis = successPlotter.createSuccessAxis(plannerName);
  successAxis->options.name = plannerName + "CombinedSuccessAxis"s;
  successAxis->options.xlabel = "{\\empty}"s;
  successAxis->options.xticklabel = "{\\empty}"s;

  // Create the median cost axis and override some options.
  MedianCostEvolutionPlotter medianCostPlotter(config_, stats_);
  auto medianCostAxis = medianCostPlotter.createMedianCostEvolutionAxis(plannerName);
  medianCostAxis->options.at =
      "($("s + plannerName + "CombinedSuccessAxis.south) - (0.0em, 0.3em)$)"s;
  medianCostAxis->options.anchor = "north";
  medianCostAxis->options.name = plannerName + "CombinedMedianCostAxis"s;

  // Create the picture and add the axes.
  TikzPicture picture(config_);
  picture.addAxis(successAxis);
  picture.addAxis(medianCostAxis);

  // Generate the tikz file.
  auto picturePath = fs::path(config_->get<std::string>("experiment/results")).parent_path() /
                     fs::path("tikz/"s + plannerName + "_combined_success_median_cost_plot.tikz"s);
  picture.write(picturePath);
  return picturePath;
}

}  // namespace ompltools

}  // namespace esp

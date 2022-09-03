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

// Authors: Valentin Hartmann

#include "esp_tikz/multiquery_report.h"

#include <stdlib.h>
#include <algorithm>
#include <fstream>

#include <ompl/util/Console.h>

#include "esp_factories/context_factory.h"
#include "esp_tikz/kpi_table.h"
#include "esp_tikz/mq_kpi_table.h"
#include "esp_tikz/pgf_axis.h"
#include "esp_tikz/pgf_fillbetween.h"
#include "esp_tikz/pgf_plot.h"
#include "esp_tikz/pgf_table.h"

#include "esp_tikz/median_cost_evolution_plotter.h"
#include "esp_tikz/median_initial_solution_plotter.h"
#include "esp_tikz/success_plotter.h"

namespace esp {

namespace ompltools {

using namespace std::string_literals;
namespace fs = std::experimental::filesystem;

MultiqueryReport::MultiqueryReport(const std::shared_ptr<Configuration>& config,
                                   const MultiqueryStatistics& stats) :
    BaseReport(config),
    medianCumulativeCostPlotter_(config, stats),
    medianCumulativeDurationPlotter_(config, stats),
    medianInitialDurationQueryPlotter_(config, stats),
    medianInitialCostQueryPlotter_(config, stats),
    medianFinalCostQueryPlotter_(config, stats),
    queryCostScatterPlotter_(config, stats),
    successRateQueryPlotter_(config, stats),
    stats_(stats) {
}

fs::path MultiqueryReport::generateReport() {
  auto reportPath =
      fs::path(config_->get<std::string>("experiment/experimentDirectory")) / "report.tex"s;
  // Open the filestream.
  std::ofstream report;
  report.open(reportPath.c_str());

  // Check on the failbit.
  if (report.fail() == true) {
    auto msg = "ExperimentReport failed to create a report at '" + reportPath.string() + "'."s;
    throw std::ios_base::failure(msg);
  }

  // Write the preamble.
  report << preamble().str();

  // Start the document.
  report << "\\begin{document}\n";

  // Make the title.
  report << "\\maketitle\n";

  // overview
  report << overview().str();

  // individual results
  report << individualResults().str();

  // Add the appendix.
  report << appendix().str();

  // End the document.
  report << "\\end{document}\n";

  return reportPath;
}

std::stringstream MultiqueryReport::overview() const {
  std::stringstream overview;

  // We often refer to the planner names, this reference just makes it more convenient.
  const auto& plannerNames = config_->get<std::vector<std::string>>("experiment/planners");

  // Create a section for every planner.
  overview << "\n\\pagebreak\n";

  // Provide some basic info about this experiment.
  overview << "\\section{" << "Overview" << "}\\label{sec:" << "all" << "}\n";
  overview << "This report was automatically generated using ESP "
              "OMPLtools. It presents the results "
              "for the "
           << experimentName_ << " experiment, which executed "
           << config_->get<std::size_t>("experiment/numRuns") << " runs of "
           << config_->get<std::size_t>("context/" + config_->get<std::string>("experiment/context")  + "/starts/numGenerated") << " queries each with ";
  for (std::size_t i = 0u; i < plannerNames.size() - 1u; ++i) {
    overview << plotPlannerNames_.at(plannerNames.at(i)) << ", ";
  }
  overview << " and " << plotPlannerNames_.at(plannerNames.back()) << " on the \\texttt{"
           << config_->get<std::string>("experiment/context")
           << "} planning context. See appendix~\\ref{sec:experiment-configuration} for more "
              "information about the "
              "experiment setup. ";

  if (config_->get<std::string>("context/" + config_->get<std::string>("experiment/context") + "/starts/type") == "specified"){
    overview << "Start/goals: prespecified.";
  }
  else{
    overview << "Start/goals: " << config_->get<std::string>("context/"+config_->get<std::string>("experiment/context")+"/starts/generativeModel") << " sampled.";
  }

  // Create the results summary section.
  overview << "\\subsection{Results Summary}\\label{sec:overview-results-summary}\n";
  // Create the KPI table.
  MqKpiTable mqKpiTable(config_, stats_);
  for (const auto& name : config_->get<std::vector<std::string>>("experiment/planners")) {
    mqKpiTable.addKpi(name, plotPlannerNames_.at(name));
  }
  overview << mqKpiTable.string() << '\n';

  overview << "\\subsection{" << "Initial solution time" << "}\\label{sec:" << "soltime" << "}\n";
  auto legend =
      latexPlotter_.createLegendAxis(config_->get<std::vector<std::string>>("experiment/planners"));

  auto medianQueryDurationAxis = medianInitialDurationQueryPlotter_.createMedianInitialDurationAxis();
  auto medianCumulativeDurationAxis = medianCumulativeDurationPlotter_.createMedianCumulativeDurationAxis();

  // Stack the axes
  latexPlotter_.stack(medianQueryDurationAxis, medianCumulativeDurationAxis, legend);

  overview << "\\begin{center}\n\\input{"
          << latexPlotter_.createPicture(medianQueryDurationAxis, medianCumulativeDurationAxis, legend).string()
          << "}\n\\captionof{figure}{\\footnotesize (Top) Median duration per query of the initial solution of "
          << "all planners with "
          << 100.0 * config_->get<double>("medianInitialDurationPlots/confidence")
          << "\\% confidence interval. "
          << "(Bottom) Cumulative median duration per query of the initial solution of "
          << "all planners with "
          << 100.0 * config_->get<double>("medianCumulativeInitialDurationPlots/confidence")
          << "\\% confidence interval. "
          << "}\\end{center}\n";

  auto medianQueryInitialCostAxis = medianInitialCostQueryPlotter_.createMedianInitialCostAxis();
  auto medianCumulativeInitialCostAxis = medianCumulativeCostPlotter_.createMedianCumulativeCostAxis();

  // Stack the axes
  latexPlotter_.stack(medianQueryInitialCostAxis, medianCumulativeInitialCostAxis, legend);

  overview << "\n\\pagebreak\n";
  overview << "\\subsection{" << "Initial solution cost" << "}\\label{sec:" << "cost" << "}\n";
  overview << "\\begin{center}\n\\input{"
          << latexPlotter_.createPicture(medianQueryInitialCostAxis, medianCumulativeInitialCostAxis, legend).string()
          << "}\n\\captionof{figure}{\\footnotesize (Top) Median initial cost per query for "
          << "all planners" << " with "
          << 100.0 * config_->get<double>("medianInitialCostPerQueryPlots/confidence")
          << "\\% confidence interval. "
          << "(Bottom) Cumulative median cost per query of the initial solution of "
          << "all planners with "
          << 100.0 * config_->get<double>("medianCumulativeCostPlots/confidence")
          << "\\% confidence interval. "
          << "}\\end{center}\n";

  auto medianQueryLastCostAxis = medianFinalCostQueryPlotter_.createMedianFinalCostAxis();
  auto medianCumulativeLastCostAxis = medianCumulativeCostPlotter_.createMedianCumulativeCostAxis(false);

  medianCumulativeLastCostAxis->options.name += "_final";

  // Stack the axes
  latexPlotter_.stack(medianQueryLastCostAxis, medianCumulativeLastCostAxis, legend);

  overview << "\\subsection{" << "Final solution cost" << "}\\label{sec:" << "cost" << "}\n";
  overview << "\\begin{center}\n\\input{"
          << latexPlotter_.createPicture(medianQueryLastCostAxis, medianCumulativeLastCostAxis, legend).string()
          << "}\n\\captionof{figure}{\\footnotesize (Top) Median final cost per query for "
          << "all planners" << " with "
          << 100.0 * config_->get<double>("medianFinalCostPerQueryPlots/confidence")
          << "\\% confidence interval. "
          << "(Bottom) Cumulative median cost per query of the final solution of "
          << "all planners with "
          << 100.0 * config_->get<double>("medianCumulativeCostPlots/confidence")
          << "\\% confidence interval. "
          << "}\\end{center}\n";

  auto successRateQueryAxis = successRateQueryPlotter_.createSuccessRateQueryAxis();

  // Stack the axes
  latexPlotter_.stack(successRateQueryAxis, legend);

  overview << "\\subsection{" << "Success Rates" << "}\\label{sec:succ" << "}\n";
  overview << "\\begin{center}\n\\input{"
          << latexPlotter_.createPicture(successRateQueryAxis, legend).string()
          << "}\n\\captionof{figure}{\\footnotesize Success rate "
          << " of all planners" << " at "
          << "the maximum solve time.} \\end{center}\n";

  auto successRateHalfTimeQueryAxis = successRateQueryPlotter_.createSuccessRateQueryAxis(50);

  // Stack the axes
  latexPlotter_.stack(successRateHalfTimeQueryAxis, legend);

  overview << "\\begin{center}\n\\input{"
          << latexPlotter_.createPicture(successRateHalfTimeQueryAxis, legend).string()
          << "}\n\\captionof{figure}{\\footnotesize Success rate "
          << " of all planners" << " at "
          << "50\\% of the maximal solve time.} \\end{center}\n";

  overview << "\n\\pagebreak\n";
  overview << "\\subsection{" << "Breakout cost convergence plots" << "}\\label{sec:" << "cost" << "}\n";
  const unsigned int numCostPlots = 10;
  for (auto i=0u; i<numCostPlots; ++i){
    const auto n = static_cast<unsigned int>(std::floor(static_cast<double>(i*(stats_.getNumQueries()-1)) / static_cast<double>(numCostPlots-1)));

    auto nthQueryStatistics = stats_.getQueryStatistics(n);
    MedianCostEvolutionPlotter medianCostEvolutionPlotter(config_, nthQueryStatistics);
    MedianInitialSolutionPlotter medianInitialSolutionPlotter(config_, nthQueryStatistics);
    SuccessPlotter successPlotter(config_, nthQueryStatistics);

    auto medianCostEvolutionAxis = medianCostEvolutionPlotter.createMedianCostEvolutionAxis();
    auto medianInitialSolutionAxis = medianInitialSolutionPlotter.createMedianInitialSolutionAxis();
    auto successAxis = successPlotter.createSuccessAxis();

    medianCostEvolutionAxis->options.name += std::to_string(n);

    // Merge the intial solution axis into the cost evolution axis.
    medianCostEvolutionAxis->mergePlots(medianInitialSolutionAxis);

    // Align the success and median cost evolution axes.
    latexPlotter_.alignAbszissen(successAxis, medianCostEvolutionAxis);

    // Stack the axes
    latexPlotter_.stack(successAxis, medianCostEvolutionAxis, legend);

    overview << "\\subsubsection{Query " << std::to_string(n) << "}\n";

    // Create the KPI table.
    KpiTable kpiTable(config_, nthQueryStatistics);
    for (const auto& name : config_->get<std::vector<std::string>>("experiment/planners")) {
      kpiTable.addKpi(name, plotPlannerNames_.at(name));
    }
    overview << kpiTable.string() << '\n';

    overview << "\\begin{center}\n\\input{"
            << latexPlotter_.createPicture(successAxis, medianCostEvolutionAxis, legend).string()
            << "}\n\\captionof{figure}{\\footnotesize (Top) Percentage of runs that found a solution "
               "at any given time. (Bottom) Median cost evolution for query "
            << std::to_string(n) + " of all planners" << " with "
            << 100.0 * config_->get<double>("medianCostPlots/confidence")
            << "\\% confidence interval.} \\end{center}\n";

    overview << "\n\\pagebreak\n";
  }

  return overview;
}

std::stringstream MultiqueryReport::individualResults() const {
  std::stringstream results;

  // Create a section for every planner.
  const auto& plannerNames = config_->get<std::vector<std::string>>("experiment/planners");
  for (const auto& name : plannerNames) {
    // Create the section title on a new page.
    results << "\n\\pagebreak\n";
    results << "\\section{" << plotPlannerNames_.at(name) << "}\\label{sec:" << name << "}\n";

    results << "\\subsection{Duration per Query}\\label{sec:" << name << "-query-duration}\n";
    results << "\\begin{center}\n\\input{"
            << medianInitialDurationQueryPlotter_.createMedianInitialDurationPicture(name).string()
            << "}\n\\captionof{figure}{\\footnotesize (Top) Median duration per query of the initial solution of "
            << plotPlannerNames_.at(name) << " with "
            << 100.0 * config_->get<double>("medianInitialDurationPlots/confidence")
            << "\\% confidence interval.} \\end{center}\n";
  }

  return results;
}

}  // namespace ompltools

}  // namespace esp

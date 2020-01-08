/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2019, University of Oxford
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

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <experimental/filesystem>

#include <ompl/util/Console.h>

#include "nlohmann/json.hpp"

namespace esp {

namespace ompltools {

namespace json = nlohmann;

// A class to manage configuration for repeatable experiments.
class Configuration {
 public:
  Configuration(int argc, char** argv);
  ~Configuration() = default;

  // Load from command line options.
  void load(int argc, char** argv);

  // Check if a key exists.
  bool contains(const std::string& key) const;

  // Print a parameter.
  std::string dump(const std::string& key = "") const;

  // Get a parameter by its key.
  template <typename T>
  T get(const std::string& key) const;

  template <typename T>
  void add(const std::string& key, const T& value);

  // This adds to or creates an "experiment" entry in the accessed parameters with various
  // information about the state of the working directory and the OMPL seed.
  void registerAsExperiment();

  // Dump the parameters.
  void dumpAll(std::ostream& out = std::cout) const;
  void dumpAll(const std::string& filename) const;
  void dumpAccessed(std::ostream& out = std::cout) const;
  void dumpAccessed(const std::string& filename) const;

 private:
  // Recursive implementation of public contains method.
  bool contains(const std::string& key, const json::json& parameters) const;

  // Recursive implementation of public dump method.
  std::string dump(const std::string& key, const json::json& parameters) const;

  // Recursive implementation of public get method.
  template <typename T>
  T get(const std::string& key, const json::json& parameters, const std::string& prevNs) const;

  template <typename T>
  void add(const std::string& key, const T& value, json::json* parameters);

  // Recursively register an accessed parameter.
  template <typename T>
  void registerAccess(const std::string& key, const T& value, json::json* accessedParameters) const;

  // Helper to load the default config from the default path.
  void loadDefaultConfigs(bool loadDefaultContextConfigs = true,
                          bool loadDefaultPlannerConfigs = true,
                          bool loadDefaultObjectiveConfigs = true);

  void loadReportConfig(const std::experimental::filesystem::path& path);

  // Check if the name is nested, i.e., contains a '/'.
  bool isNestedKey(const std::string& name) const;

  // Split a nested name, throws if it isn't nested.
  std::pair<const std::string, const std::string> split(const std::string& name) const;

  // If any config file specifies the seed, we need to set it in OMPL, otherwise we store OMPL's
  // seed.
  void handleSeedSpecification();

  std::string executable_{};

  // All parameters as a big json structure.
  json::json parameters_{};

  // The parameters that were actually accessed.
  mutable json::json accessedParameters_{};
};

template <typename T>
T Configuration::get(const std::string& key) const {
  return get<T>(key, parameters_, "");
}

template <typename T>
T Configuration::get(const std::string& key, const json::json& parameters,
                     const std::string& path) const {
  using namespace std::string_literals;
  if (!isNestedKey(key)) {
    if (parameters.contains(key)) {
      registerAccess<T>(path + key, parameters[key].get<T>(), &accessedParameters_);
      return parameters[key].get<T>();
    } else {
      auto msg = "Requested nonexisting parameter '"s + path + key + "'."s;
      throw std::invalid_argument(msg);
    }
  } else {
    auto [ns, rest] = split(key);
    if (parameters.contains(ns)) {
      auto nestedParameters = parameters[ns];
      return get<T>(rest, nestedParameters, path + ns + "/");
    } else {
      auto msg = "Requested nonexisting parameter '"s + path + key + "'."s;
      throw std::invalid_argument(msg);
    }
  }
}

template <typename T>
void Configuration::add(const std::string& key, const T& value) {
  // We allow overwriting the results/name field of the experiment. Conceptually this seems ok, but from
  // a software architecture standpoint this hints at a flaw. Would it be cleaner to have an
  // "Accessed" element in parameters_ rather than having accessedParameters_?
  if (contains(key) && key == std::string("experiment/results")) {
    parameters_["experiment"].erase("results");
  } else if (contains(key) && key == std::string("experiment/name")) {
    parameters_["experiment"].erase("name");
  }
  // We should prevent overwriting any other parameter to ensure reproducibility.
  if (contains(key) && get<T>(key) != value) {
    OMPL_ERROR("'%s': Parameter already exists with a different value.", key.c_str());
    throw std::runtime_error("Reproducibility error.");
  }
  add<T>(key, value, &parameters_);
  registerAccess(key, value, &accessedParameters_);
}

template <typename T>
void Configuration::add(const std::string& key, const T& value, json::json* parameters) {
  if (!isNestedKey(key)) {
    (*parameters)[key] = value;
  } else {
    auto [ns, rest] = split(key);
    if (!parameters->contains(ns)) {
      (*parameters)[ns] = json::json();
    }
    add<T>(rest, value, &((*parameters)[ns]));
  }
}

template <typename T>
void Configuration::registerAccess(const std::string& key, const T& value,
                                   json::json* accessedParameters) const {
  if (!isNestedKey(key)) {
    if (accessedParameters->contains(key) && (*accessedParameters)[key] != value) {
      OMPL_ERROR("Value of parameter '%s' being accessed has changed.", key.c_str());
      throw std::runtime_error("Reproducibility error.");
    }
    (*accessedParameters)[key] = value;
  } else {
    auto [ns, rest] = split(key);
    if (!accessedParameters->contains(ns)) {
      (*accessedParameters)[ns] = json::json();
    }
    registerAccess(rest, value, &((*accessedParameters)[ns]));
  }
}

}  // namespace ompltools

}  // namespace esp

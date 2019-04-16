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

#include "esp_tikz/pgf_table.h"

#include <sstream>

namespace esp {

namespace ompltools {

std::string PgfTableOptions::string() const {
  std::ostringstream stream{};
  stream << "\n  row sep=" << rowSep
         << ",\n  col sep=" << colSep;
  return stream.str();
}

void PgfTable::addColumn(const std::vector<double>& column) {
  if (!data_.empty() && column.size() != data_.at(0u).size()) {
    throw std::runtime_error("Number of elements in column does not match table.");
  }
  data_.push_back(column);
}

void PgfTable::addRow(const std::vector<double>& row) {
  // Sanity checks.
  if (data_.size() != row.size()) {
    throw std::runtime_error("Number of elements in row does not match table.");
  }
  for (std::size_t i = 1; i < data_.size(); ++i) {
    if (data_.at(i).size() != data_.at(0u).size()) {
      throw std::runtime_error("Cannot add row; columns have unequal entries.");
    }
  }

  // Append row to table.
  for (std::size_t i = 0u; i < row.size(); ++i) {
    data_.at(i).emplace_back(row.at(i));
  }
}

void PgfTable::setOptions(const PgfTableOptions& options) {
  options_ = options;
}

std::string PgfTable::string() const {
  if (data_.empty()) {
    return {};
  }
  std::ostringstream stream{};
  stream << "table [" << options_.string() << "\n]{\n";
  for (std::size_t row = 0u; row < data_.at(0u).size(); ++row) {
    for (std::size_t col = 0u; col < data_.size(); ++col) {
      stream << data_.at(col).at(row);
      if (col == data_.size() - 1u) {
        stream << ' ' << options_.rowSep << '\n';
      } else {
        stream << ' ' << options_.colSep << ' ';
      }
    }
  }
  stream << "};\n";

  return stream.str();
}

}  // namespace ompltools

}  // namespace esp

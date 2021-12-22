// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef ABSTRACT_TUPLE_POINT_DATA_H
#define ABSTRACT_TUPLE_POINT_DATA_H

#include <vector>
#include <limits>

#include "SlopeSums.h"

using namespace std;

class AbstractTuplePointData
{
 public:
  virtual ~AbstractTuplePointData();

  virtual AbstractTuplePointData* clone() const = 0;

  virtual bool isFullSparseTube(const vector<unsigned int>& elementsToAdd) const;

  virtual void setTuplePoints(const vector<vector<unsigned int>>::const_iterator dimensionIt, const pair<double, double>& point) = 0;
  virtual void sortTubesAndGetMinCoordinates(double& minX, double& minY) = 0;
  virtual void translate(const double deltaX, const double deltaY) = 0;
  void sortTubesAndTranslateToPositiveQuadrant();
  virtual void setSlopeSums(SlopeSums& slopeSums) const = 0;
  virtual void increaseSlopeSums(const vector<vector<unsigned int>>& present, const vector<unsigned int>::const_iterator dimensionIdIt, SlopeSums& slopeSums) const = 0;
  virtual void decreaseSlopeSums(const vector<vector<unsigned int>>& presentAndpotential, const vector<unsigned int>::const_iterator dimensionIdIt, SlopeSums& slopeSums) const = 0;
  virtual void increaseSlopeSums(const vector<vector<unsigned int>>& present, const vector<unsigned int>& elementsSetPresent, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator presentDimensionIdIt, SlopeSums& slopeSums) const = 0;
  virtual void decreaseSlopeSums(const vector<vector<unsigned int>>& presentAndPotential, const vector<unsigned int>& elementsSetAbsent, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator absentDimensionIdIt, SlopeSums& slopeSums) const = 0;
};

#endif /*ABSTRACT_TUPLE_POINT_DATA_H*/

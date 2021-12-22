// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUPLE_POINT_DENSE_TUBE_H
#define TUPLE_POINT_DENSE_TUBE_H

#include <cmath>

#include "TuplePointTube.h"

class TuplePointDenseTube final : public TuplePointTube
{
 public:
  TuplePointDenseTube(const vector<pair<unsigned int, pair<double, double>>>& sparseTube);

  void setTuplePoints(const vector<vector<unsigned int>>::const_iterator dimensionIt, const pair<double, double>& point);
  void sortTubesAndGetMinCoordinates(double& minX, double& minY);
  void translate(const double deltaX, const double deltaY);
  void setSlopeSums(SlopeSums& slopeSums) const;
  void increaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const;
  void decreaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const;

  static void setSize(const unsigned int size);

 private:
  vector<pair<double, double>> tube;

  static unsigned int size;

  TuplePointDenseTube* clone() const;
};

#endif /*TUPLE_POINT_DENSE_TUBE_H*/

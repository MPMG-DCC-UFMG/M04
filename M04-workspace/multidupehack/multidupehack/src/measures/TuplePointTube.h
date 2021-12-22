// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUPLE_POINT__TUBE_H
#define TUPLE_POINT__TUBE_H

#include "AbstractTuplePointData.h"

class TuplePointTube : public AbstractTuplePointData
{
 public:
  virtual ~TuplePointTube();

  void increaseSlopeSums(const vector<vector<unsigned int>>& present, const vector<unsigned int>::const_iterator dimensionIdIt, SlopeSums& slopeSums) const;
  void decreaseSlopeSums(const vector<vector<unsigned int>>& presentAndPotential, const vector<unsigned int>::const_iterator dimensionIdIt, SlopeSums& slopeSums) const;
  void increaseSlopeSums(const vector<vector<unsigned int>>& present, const vector<unsigned int>& elementsSetPresent, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator presentDimensionIdIt, SlopeSums& slopeSums) const;
  void decreaseSlopeSums(const vector<vector<unsigned int>>& presentAndPotential, const vector<unsigned int>& elementsSetAbsent, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator absentDimensionIdIt, SlopeSums& slopeSums) const;
  virtual void increaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const = 0;
  virtual void decreaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const = 0;
};

#endif /*TUPLE_POINT_TUBE_H*/

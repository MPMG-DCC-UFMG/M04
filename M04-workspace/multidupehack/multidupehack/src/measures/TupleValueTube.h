// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUPLE_VALUE_TUBE_H
#define TUPLE_VALUE_TUBE_H

#include "AbstractTupleValueData.h"

class TupleValueTube : public AbstractTupleValueData
{
 public:
  virtual ~TupleValueTube();

  void decreaseSum(const vector<vector<unsigned int>>& pattern, const vector<unsigned int>::const_iterator dimensionIdIt, double& sum) const;
  void decreaseSum(const vector<vector<unsigned int>>& pattern, const vector<unsigned int>& elements, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator absentDimensionIdIt, double& sum) const;
  virtual void decreaseSum(const vector<unsigned int>& dimension, double& sum) const = 0;
};
#endif /*TUPLE_VALUE_TUBE_H*/

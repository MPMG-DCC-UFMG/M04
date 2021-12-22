// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUPLE_VALUE_DENSE_TUBE_H
#define TUPLE_VALUE_DENSE_TUBE_H

#include "TupleValueTube.h"

class TupleValueDenseTube final : public TupleValueTube
{
 public:
  TupleValueDenseTube(const vector<pair<unsigned int, double>>& sparseTube);

  void setTupleValues(const vector<vector<unsigned int>>::const_iterator dimensionIt, const double value);
  void sortTubes();
  void sortTubesAndSetSum(double& sum);
  void decreaseSum(const vector<unsigned int>& dimension, double& sum) const;

  static void setSize(const unsigned int size);

 private:
  vector<double> tube;

  static unsigned int size;

  TupleValueDenseTube* clone() const;
};
#endif /*TUPLE_VALUE_DENSE_TUBE_H*/

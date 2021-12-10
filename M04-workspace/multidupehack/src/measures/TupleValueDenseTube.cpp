// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "TupleValueDenseTube.h"

unsigned int TupleValueDenseTube::size;

TupleValueDenseTube::TupleValueDenseTube(const vector<pair<unsigned int, double>>& sparseTube): tube(size)
{
  for (const pair<unsigned int, double>& entry : sparseTube)
    {
      tube[entry.first] = entry.second;
    }
}

TupleValueDenseTube* TupleValueDenseTube::clone() const
{
  return new TupleValueDenseTube(*this);
}

void TupleValueDenseTube::setTupleValues(const vector<vector<unsigned int>>::const_iterator dimensionIt, const double value)
{
  for (const unsigned int hyperplaneId : *dimensionIt)
    {
      tube[hyperplaneId] = value;
    }
}

void TupleValueDenseTube::sortTubes()
{
}

void TupleValueDenseTube::sortTubesAndSetSum(double& sum)
{
  for (const double keyValue : tube)
    {
      sum += keyValue;
    }
}

void TupleValueDenseTube::decreaseSum(const vector<unsigned int>& dimension, double& sum) const
{
  for (const unsigned int element : dimension)
    {
      sum -= tube[element];
    }
}

void TupleValueDenseTube::setSize(const unsigned int sizeParam)
{
  size = sizeParam;
}

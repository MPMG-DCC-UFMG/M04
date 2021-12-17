// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "TupleValueSparseTube.h"

unsigned int TupleValueSparseTube::sizeLimit;

TupleValueSparseTube::TupleValueSparseTube(): tube()
{
}

TupleValueSparseTube* TupleValueSparseTube::clone() const
{
  return new TupleValueSparseTube(*this);
}

bool TupleValueSparseTube::isFullSparseTube(const vector<unsigned int>& elementsToAdd) const
{
  return elementsToAdd.size() + tube.size() > sizeLimit;
}

TupleValueDenseTube* TupleValueSparseTube::getDenseRepresentation() const
{
  return new TupleValueDenseTube(tube);
}

void TupleValueSparseTube::setTupleValues(const vector<vector<unsigned int>>::const_iterator dimensionIt, const double value)
{
  for (const unsigned int elementId : *dimensionIt)
    {
      tube.emplace_back(elementId, value);
    }
}

vector<pair<unsigned int, double>>::const_iterator TupleValueSparseTube::begin() const
{
  return tube.begin();
}

vector<pair<unsigned int, double>>::const_iterator TupleValueSparseTube::end() const
{
  return tube.end();
}

void TupleValueSparseTube::sortTubes()
{
  tube.shrink_to_fit();
  sort(tube.begin(), tube.end(), [](const pair<unsigned int, double>& entry1, const pair<unsigned int, double>& entry2) {return entry1.first < entry2.first;});
}

void TupleValueSparseTube::sortTubesAndSetSum(double& sum)
{
  sortTubes();
  for (const pair<unsigned int, double>& keyValue : tube)
    {
      sum += keyValue.second;
    }
}

void TupleValueSparseTube::decreaseSum(const vector<unsigned int>& dimension, double& sum) const
{
  const vector<pair<unsigned int, double>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, double>>::const_iterator tubeBegin = tube.begin();
  for (const unsigned int element : dimension)
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, element, [](const pair<unsigned int, double>& entry, const unsigned int element) {return entry.first < element;});
      if (tubeBegin != tubeEnd && tubeBegin->first == element)
	{
	  sum -= tubeBegin->second;
	}
    }
}

void TupleValueSparseTube::setSizeLimit(const unsigned int sizeLimitParam)
{
  sizeLimit = sizeLimitParam;
}

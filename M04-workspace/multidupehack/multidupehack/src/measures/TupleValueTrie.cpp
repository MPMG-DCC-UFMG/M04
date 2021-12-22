// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "TupleValueTrie.h"

TupleValueTrie::TupleValueTrie(const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator dimensionIdsEnd, const vector<unsigned int>& cardinalities): hyperplanes()
{
  const unsigned int cardinality = cardinalities[*dimensionIdIt];
  hyperplanes.reserve(cardinality);
  if (dimensionIdIt + 2 == dimensionIdsEnd)
    {
      for (unsigned int hyperplaneId = 0; hyperplaneId != cardinality; ++hyperplaneId)
	{
	  hyperplanes.push_back(new TupleValueSparseTube());
	}
      return;
    }
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  for (unsigned int hyperplaneId = 0; hyperplaneId != cardinality; ++hyperplaneId)
    {
      hyperplanes.push_back(new TupleValueTrie(nextDimensionIdIt, dimensionIdsEnd, cardinalities));
    }
}

TupleValueTrie::TupleValueTrie(const TupleValueTrie& otherTupleValueTrie): hyperplanes()
{
  copy(otherTupleValueTrie);
}

TupleValueTrie::TupleValueTrie(TupleValueTrie&& otherTupleValueTrie): hyperplanes(std::move(otherTupleValueTrie.hyperplanes))
{
}

TupleValueTrie::~TupleValueTrie()
{
  for (AbstractTupleValueData* hyperplane : hyperplanes)
    {
      delete hyperplane;
    }
}

TupleValueTrie* TupleValueTrie::clone() const
{
  return new TupleValueTrie(*this);
}

TupleValueTrie& TupleValueTrie::operator=(const TupleValueTrie& otherTupleValueTrie)
{
  copy(otherTupleValueTrie);
  return *this;
}

TupleValueTrie& TupleValueTrie::operator=(TupleValueTrie&& otherTupleValueTrie)
{
  hyperplanes = std::move(otherTupleValueTrie.hyperplanes);
  return *this;
}

void TupleValueTrie::copy(const TupleValueTrie& otherTupleValueTrie)
{
  hyperplanes.reserve(otherTupleValueTrie.hyperplanes.size());
  for (const AbstractTupleValueData* hyperplane : otherTupleValueTrie.hyperplanes)
    {
      hyperplanes.push_back(hyperplane->clone());
    }
}

void TupleValueTrie::setTupleValues(const vector<vector<unsigned int>>::const_iterator dimensionIt, const double value)
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  for (const unsigned int element : *dimensionIt)
    {
      AbstractTupleValueData*& hyperplane = hyperplanes[element];
      if (hyperplane->isFullSparseTube(*nextDimensionIt))
      	{
      	  TupleValueDenseTube* newHyperplane = static_cast<TupleValueSparseTube*>(hyperplane)->getDenseRepresentation();
      	  delete hyperplane;
	  hyperplane = newHyperplane;
      	}
      hyperplane->setTupleValues(nextDimensionIt, value);
    }
}

void TupleValueTrie::sortTubes()
{
  for (AbstractTupleValueData* hyperplane : hyperplanes)
    {
      hyperplane->sortTubes();
    }
}

void TupleValueTrie::sortTubesAndSetSum(double& sum)
{
  for (AbstractTupleValueData* hyperplane : hyperplanes)
    {
      hyperplane->sortTubesAndSetSum(sum);
    }
}

void TupleValueTrie::decreaseSum(const vector<vector<unsigned int>>& pattern, const vector<unsigned int>& elements, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator absentDimensionIdIt, double& sum) const
{
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  if (dimensionIdIt == absentDimensionIdIt)
    {
      for (const unsigned int element : elements)
	{
	  hyperplanes[element]->decreaseSum(pattern, nextDimensionIdIt, sum);
	}
      return;
    }
  for (const unsigned int element : pattern[*dimensionIdIt])
    {
      hyperplanes[element]->decreaseSum(pattern, elements, nextDimensionIdIt, absentDimensionIdIt, sum);
    }
}

void TupleValueTrie::decreaseSum(const vector<vector<unsigned int>>& pattern, const vector<unsigned int>::const_iterator dimensionIdIt, double& sum) const
{
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  for (const unsigned int element : pattern[*dimensionIdIt])
    {
      hyperplanes[element]->decreaseSum(pattern, nextDimensionIdIt, sum);
    }
}

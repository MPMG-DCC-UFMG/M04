// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "TuplePointTrie.h"

TuplePointTrie::TuplePointTrie(const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator dimensionIdsEnd, const vector<unsigned int>& cardinalities): hyperplanes()
{
  const unsigned int cardinality = cardinalities[*dimensionIdIt];
  hyperplanes.reserve(cardinality);
  if (dimensionIdIt + 2 == dimensionIdsEnd)
    {
      for (unsigned int hyperplaneId = 0; hyperplaneId != cardinality; ++hyperplaneId)
	{
	  hyperplanes.push_back(new TuplePointSparseTube());
	}
      return;
    }
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  for (unsigned int hyperplaneId = 0; hyperplaneId != cardinality; ++hyperplaneId)
    {
      hyperplanes.push_back(new TuplePointTrie(nextDimensionIdIt, dimensionIdsEnd, cardinalities));
    }
}

TuplePointTrie::TuplePointTrie(const TuplePointTrie& otherTuplePointTrie): hyperplanes()
{
  copy(otherTuplePointTrie);
}

TuplePointTrie::TuplePointTrie(TuplePointTrie&& otherTuplePointTrie): hyperplanes(std::move(otherTuplePointTrie.hyperplanes))
{
}

TuplePointTrie::~TuplePointTrie()
{
  for (AbstractTuplePointData* hyperplane : hyperplanes)
    {
      delete hyperplane;
    }
}

TuplePointTrie* TuplePointTrie::clone() const
{
  return new TuplePointTrie(*this);
}

TuplePointTrie& TuplePointTrie::operator=(const TuplePointTrie& otherTuplePointTrie)
{
  copy(otherTuplePointTrie);
  return *this;
}

TuplePointTrie& TuplePointTrie::operator=(TuplePointTrie&& otherTuplePointTrie)
{
  hyperplanes = std::move(otherTuplePointTrie.hyperplanes);
  return *this;
}

void TuplePointTrie::copy(const TuplePointTrie& otherTuplePointTrie)
{
  hyperplanes.reserve(otherTuplePointTrie.hyperplanes.size());
  for (const AbstractTuplePointData* hyperplane : otherTuplePointTrie.hyperplanes)
    {
      hyperplanes.push_back(hyperplane->clone());
    }
}

void TuplePointTrie::setTuplePoints(const vector<vector<unsigned int>>::const_iterator dimensionIt, const pair<double, double>& point)
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  for (const unsigned int element : *dimensionIt)
    {
      AbstractTuplePointData*& hyperplane = hyperplanes[element];
      if (hyperplane->isFullSparseTube(*nextDimensionIt))
	{
	  TuplePointDenseTube* newHyperplane = static_cast<TuplePointSparseTube*>(hyperplane)->getDenseRepresentation();
	  delete hyperplane;
	  hyperplane = newHyperplane;
	}
      hyperplane->setTuplePoints(nextDimensionIt, point);
    }
}

void TuplePointTrie::sortTubesAndGetMinCoordinates(double& minX, double& minY)
{
  for (AbstractTuplePointData* hyperplane : hyperplanes)
    {
      hyperplane->sortTubesAndGetMinCoordinates(minX, minY);
    }
}

void TuplePointTrie::translate(const double deltaX, const double deltaY)
{
  for (AbstractTuplePointData* hyperplane : hyperplanes)
    {
      hyperplane->translate(deltaX, deltaY);
    }
}

void TuplePointTrie::setSlopeSums(SlopeSums& slopeSums) const
{
  for (AbstractTuplePointData* hyperplane : hyperplanes)
    {
      hyperplane->setSlopeSums(slopeSums);
    }
}

void TuplePointTrie::increaseSlopeSums(const vector<vector<unsigned int>>& present, const vector<unsigned int>::const_iterator dimensionIdIt, SlopeSums& slopeSums) const
{
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  for (const unsigned int presentElement : present[*dimensionIdIt])
    {
      hyperplanes[presentElement]->increaseSlopeSums(present, nextDimensionIdIt, slopeSums);
    }
}

void TuplePointTrie::decreaseSlopeSums(const vector<vector<unsigned int>>& presentAndPotential, const vector<unsigned int>::const_iterator dimensionIdIt, SlopeSums& slopeSums) const
{
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  for (const unsigned int element : presentAndPotential[*dimensionIdIt])
    {
      hyperplanes[element]->decreaseSlopeSums(presentAndPotential, nextDimensionIdIt, slopeSums);
    }
}

void TuplePointTrie::increaseSlopeSums(const vector<vector<unsigned int>>& present, const vector<unsigned int>& elementsSetPresent, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator presentDimensionIdIt, SlopeSums& slopeSums) const
{
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  if (dimensionIdIt == presentDimensionIdIt)
    {
      for (const unsigned int elementSetPresent : elementsSetPresent)
	{
	  hyperplanes[elementSetPresent]->increaseSlopeSums(present, nextDimensionIdIt, slopeSums);
	}
      return;
    }
  for (const unsigned int presentElement : present[*dimensionIdIt])
    {
      hyperplanes[presentElement]->increaseSlopeSums(present, elementsSetPresent, nextDimensionIdIt, presentDimensionIdIt, slopeSums);
    }
}

void TuplePointTrie::decreaseSlopeSums(const vector<vector<unsigned int>>& presentAndPotential, const vector<unsigned int>& elementsSetAbsent, const vector<unsigned int>::const_iterator dimensionIdIt, const vector<unsigned int>::const_iterator absentDimensionIdIt, SlopeSums& slopeSums) const
{
  const vector<unsigned int>::const_iterator nextDimensionIdIt = dimensionIdIt + 1;
  if (dimensionIdIt == absentDimensionIdIt)
    {
      for (const unsigned int elementSetAbsent : elementsSetAbsent)
	{
	  hyperplanes[elementSetAbsent]->decreaseSlopeSums(presentAndPotential, nextDimensionIdIt, slopeSums);
	}
      return;
    }
  for (const unsigned int element : presentAndPotential[*dimensionIdIt])
    {
      hyperplanes[element]->decreaseSlopeSums(presentAndPotential, elementsSetAbsent, nextDimensionIdIt, absentDimensionIdIt, slopeSums);
    }
}

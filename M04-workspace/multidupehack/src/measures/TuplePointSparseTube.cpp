// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "TuplePointSparseTube.h"

unsigned int TuplePointSparseTube::sizeLimit;

TuplePointSparseTube::TuplePointSparseTube(): tube()
{
}

TuplePointSparseTube* TuplePointSparseTube::clone() const
{
  return new TuplePointSparseTube(*this);
}

bool TuplePointSparseTube::isFullSparseTube(const vector<unsigned int>& elementsToAdd) const
{
  return elementsToAdd.size() + tube.size() > sizeLimit;
}

TuplePointDenseTube* TuplePointSparseTube::getDenseRepresentation() const
{
  return new TuplePointDenseTube(tube);
}

void TuplePointSparseTube::setTuplePoints(const vector<vector<unsigned int>>::const_iterator dimensionIt, const pair<double, double>& point)
{
  for (const unsigned int elementId : *dimensionIt)
    {
      tube.emplace_back(elementId, point);
    }
}

void TuplePointSparseTube::sortTubesAndGetMinCoordinates(double& minX, double& minY)
{
  tube.shrink_to_fit();
  sort(tube.begin(), tube.end(), [](const pair<unsigned int, pair<double, double>>& entry1, const pair<unsigned int, pair<double, double>>& entry2) {return entry1.first < entry2.first;});
  for (const pair<unsigned int, pair<double, double>>& entry : tube)
    {
      if (entry.second.first < minX)
	{
	  minX = entry.second.first;
	}
      if (entry.second.second < minY)
	{
	  minY = entry.second.second;
	}
    }
}

void TuplePointSparseTube::translate(const double deltaX, const double deltaY)
{
  for (pair<unsigned int, pair<double, double>>& entry : tube)
    {
      entry.second.first += deltaX;
      entry.second.second += deltaY;
    }
}

void TuplePointSparseTube::setSlopeSums(SlopeSums& slopeSums) const
{
  slopeSums.nbOfPoints += tube.size();
  for (const pair<unsigned int, pair<double, double>>& entry : tube)
    {
      slopeSums.sumX += entry.second.first;
      slopeSums.sumXSquared += entry.second.first * entry.second.first;
      slopeSums.sumY += entry.second.second;
      slopeSums.sumXY += entry.second.first * entry.second.second;
    }
}

void TuplePointSparseTube::increaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const
{
  const vector<pair<unsigned int, pair<double, double>>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, pair<double, double>>>::const_iterator tubeBegin = tube.begin();
  for (const unsigned int element : dimension)
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, element, [](const pair<unsigned int, pair<double, double>>& entry, const unsigned int element) {return entry.first < element;});
      if (tubeBegin != tubeEnd && tubeBegin->first == element)
	{
	  ++slopeSums.nbOfPoints;
	  slopeSums.sumX += tubeBegin->second.first;
	  slopeSums.sumXSquared += tubeBegin->second.first * tubeBegin->second.first;
	  slopeSums.sumY += tubeBegin->second.second;
	  slopeSums.sumXY += tubeBegin->second.first * tubeBegin->second.second;
	}
    }
}

void TuplePointSparseTube::decreaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const
{
  const vector<pair<unsigned int, pair<double, double>>>::const_iterator tubeEnd = tube.end();
  vector<pair<unsigned int, pair<double, double>>>::const_iterator tubeBegin = tube.begin();
  for (const unsigned int element : dimension)
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, element, [](const pair<unsigned int, pair<double, double>>& entry, const unsigned int element) {return entry.first < element;});
      if (tubeBegin != tubeEnd && tubeBegin->first == element)
	{
	  --slopeSums.nbOfPoints;
	  slopeSums.sumX -= tubeBegin->second.first;
	  slopeSums.sumXSquared -= tubeBegin->second.first * tubeBegin->second.first;
	  slopeSums.sumY -= tubeBegin->second.second;
	  slopeSums.sumXY -= tubeBegin->second.first * tubeBegin->second.second;
	}
    }
}

void TuplePointSparseTube::setSizeLimit(const unsigned int sizeLimitParam)
{
  sizeLimit = sizeLimitParam;
}

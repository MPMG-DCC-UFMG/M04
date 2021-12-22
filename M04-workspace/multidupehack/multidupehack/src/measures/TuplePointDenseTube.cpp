// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "TuplePointDenseTube.h"

unsigned int TuplePointDenseTube::size;

TuplePointDenseTube::TuplePointDenseTube(const vector<pair<unsigned int, pair<double, double>>>& sparseTube): tube(size)
{
  for (const pair<unsigned int, pair<double, double>>& entry : sparseTube)
    {
      tube[entry.first] = entry.second;
    }
}

TuplePointDenseTube* TuplePointDenseTube::clone() const
{
  return new TuplePointDenseTube(*this);
}

void TuplePointDenseTube::setTuplePoints(const vector<vector<unsigned int>>::const_iterator dimensionIt, const pair<double, double>& point)
{
  for (const unsigned int hyperplaneId : *dimensionIt)
    {
      tube[hyperplaneId] = point;
    }
}

void TuplePointDenseTube::sortTubesAndGetMinCoordinates(double& minX, double& minY)
{
  for (const pair<double, double>& point : tube)
    {
      if (point.first < minX)
	{
	  minX = point.first;
	}
      if (point.second < minY)
	{
	  minY = point.second;
	}
    }
}

void TuplePointDenseTube::translate(const double deltaX, const double deltaY)
{
  for (pair<double, double>& point : tube)
    {
      point.first += deltaX;
      point.second += deltaY;
    }
}

void TuplePointDenseTube::setSlopeSums(SlopeSums& slopeSums) const
{
  for (const pair<double, double>& point : tube)
    {
      if (!std::isnan(point.first))
	{
	  ++slopeSums.nbOfPoints;
	  slopeSums.sumX += point.first;
	  slopeSums.sumXSquared += point.first * point.first;
	  slopeSums.sumY += point.second;
	  slopeSums.sumXY += point.first * point.second;
	}
    }
}

void TuplePointDenseTube::increaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const
{
  for (const unsigned int element : dimension)
    {
      const pair<double, double>& point = tube[element];
      if (!std::isnan(point.first))
	{
	  ++slopeSums.nbOfPoints;
	  slopeSums.sumX += point.first;
	  slopeSums.sumXSquared += point.first * point.first;
	  slopeSums.sumY += point.second;
	  slopeSums.sumXY += point.first * point.second;
	}
    }
}

void TuplePointDenseTube::decreaseSlopeSums(const vector<unsigned int>& dimension, SlopeSums& slopeSums) const
{
  for (const unsigned int element : dimension)
    {
      const pair<double, double>& point = tube[element];
      if (!std::isnan(point.first))
	{
	  --slopeSums.nbOfPoints;
	  slopeSums.sumX -= point.first;
	  slopeSums.sumXSquared -= point.first * point.first;
	  slopeSums.sumY -= point.second;
	  slopeSums.sumXY -= point.first * point.second;
	}
    }
}

void TuplePointDenseTube::setSize(const unsigned int sizeParam)
{
  size = sizeParam;
}

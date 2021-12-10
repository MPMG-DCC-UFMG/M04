// Copyright (C) 2014-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "AbstractTuplePointData.h"

AbstractTuplePointData::~AbstractTuplePointData()
{
}

bool AbstractTuplePointData::isFullSparseTube(const vector<unsigned int>& elementsToAdd) const
{
  return false;
}

void AbstractTuplePointData::sortTubesAndTranslateToPositiveQuadrant()
{
  double minX = numeric_limits<double>::infinity();
  double minY = numeric_limits<double>::infinity();
  sortTubesAndGetMinCoordinates(minX, minY);
  translate(-minX, -minY);
}

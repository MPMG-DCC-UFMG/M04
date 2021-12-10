// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinArea.h"

unsigned int MinArea::threshold;

MinArea::MinArea(const vector<unsigned int>& nbOfElementsInDimensionsParam, const unsigned int thresholdParam): nbOfElementsInDimensions(nbOfElementsInDimensionsParam), minArea(0)
{
  threshold = thresholdParam;
}

MinArea* MinArea::clone() const
{
  return new MinArea(*this);
}

bool MinArea::monotone() const
{
  return true;
}

bool MinArea::violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  minArea = 1;
  unsigned int dimensionId = 0;
  for (unsigned int& nbOfElementsInDimension : nbOfElementsInDimensions)
    {
      if (dimensionId++ == dimensionIdOfElementsSetAbsent)
	{
	  nbOfElementsInDimension -= elementsSetAbsent.size();
	}
      minArea *= nbOfElementsInDimension;
    }
#ifdef DEBUG
  if (minArea < threshold)
    {
      cout << threshold << "-minimal area constraint cannot be satisfied -> Prune!\n";
    }
#endif
  return minArea < threshold;
}

float MinArea::optimisticValue() const
{
  return minArea;
}

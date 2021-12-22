// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MaxArea.h"

unsigned int MaxArea::threshold;

MaxArea::MaxArea(const unsigned int nbOfDimensions, const unsigned int thresholdParam): nbOfElementsInDimensions(nbOfDimensions), maxArea(0)
{
  threshold = thresholdParam;
}

MaxArea* MaxArea::clone() const
{
  return new MaxArea(*this);
}

bool MaxArea::violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  maxArea = 1;
  unsigned int dimensionId = 0;
  for (unsigned int& nbOfElementsInDimension : nbOfElementsInDimensions)
    {
      if (dimensionId++ == dimensionIdOfElementsSetPresent)
	{
	  nbOfElementsInDimension += elementsSetPresent.size();
	}
      maxArea *= nbOfElementsInDimension;
    }
#ifdef DEBUG
  if (maxArea > threshold)
    {
      cout << threshold << "-maximal area constraint cannot be satisfied -> Prune!\n";
    }
#endif
  return maxArea > threshold;
}

float MaxArea::optimisticValue() const
{
  return -maxArea;
}

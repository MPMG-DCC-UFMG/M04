// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MaxSize.h"

vector<unsigned int> MaxSize::thresholds;

MaxSize::MaxSize(const unsigned int dimensionIdParam, const unsigned int threshold): dimensionId(dimensionIdParam), maxSize(0)
{
  if (dimensionId >= thresholds.size())
    {
      thresholds.resize(dimensionId + 1);
    }
  thresholds[dimensionId] = threshold;
}

MaxSize* MaxSize::clone() const
{
  return new MaxSize(*this);
}

bool MaxSize::violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  if (dimensionIdOfElementsSetPresent == dimensionId)
    {
      maxSize += elementsSetPresent.size();
#ifdef DEBUG
      if (maxSize > thresholds[dimensionId])
	{
	  cout << thresholds[dimensionId] << "-maximal size constraint on attribute " << internal2ExternalAttributeOrder[dimensionId] << " cannot be satisfied -> Prune!\n";
	}
#endif
      return maxSize > thresholds[dimensionId];
    }
  return false;
}

float MaxSize::optimisticValue() const
{
  return -maxSize;
}

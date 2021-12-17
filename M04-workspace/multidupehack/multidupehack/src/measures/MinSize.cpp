// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinSize.h"

vector<unsigned int> MinSize::thresholds;

MinSize::MinSize(const unsigned int dimensionIdParam, const unsigned int minSizeParam, const unsigned int threshold): dimensionId(dimensionIdParam), minSize(minSizeParam)
{
  if (dimensionId >= thresholds.size())
    {
      thresholds.resize(dimensionId + 1);
    }
  thresholds[dimensionId] = threshold;
}

MinSize* MinSize::clone() const
{
  return new MinSize(*this);
}

bool MinSize::monotone() const
{
  return true;
}

bool MinSize::violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  if (dimensionIdOfElementsSetAbsent == dimensionId)
    {
      minSize -= elementsSetAbsent.size();
#ifdef DEBUG
      if (minSize < thresholds[dimensionId])
	{
	  cout << thresholds[dimensionId] << "-minimal size constraint on attribute " << internal2ExternalAttributeOrder[dimensionId] << " cannot be satisfied -> Prune!\n";
	}
#endif
      return minSize < thresholds[dimensionId];
    }
  return false;
}

float MinSize::optimisticValue() const
{
  return minSize;
}

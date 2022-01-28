// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MaxGroupCover.h"

vector<unsigned int> MaxGroupCover::thresholds;

MaxGroupCover::MaxGroupCover(const unsigned int groupIdParam, const unsigned int threshold): GroupMeasure(), groupId(groupIdParam)
{
  isSomeMeasureAntiMonotone = true;
  if (groupId >= thresholds.size())
    {
      thresholds.resize(groupId + 1);
    }
  thresholds[groupId] = threshold;
}

MaxGroupCover* MaxGroupCover::clone() const
{
  return new MaxGroupCover(*this);
}

bool MaxGroupCover::violationAfterMinCoversIncreased() const
{
#ifdef DEBUG
  if (minCoverOfGroup(groupId) > thresholds[groupId])
    {
      cout << thresholds[groupId] << "-maximal size constraint on group " << groupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return minCoverOfGroup(groupId) > thresholds[groupId];
}

float MaxGroupCover::optimisticValue() const
{
  return -minCoverOfGroup(groupId);
}

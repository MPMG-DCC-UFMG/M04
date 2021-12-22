// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinGroupCover.h"

vector<unsigned int> MinGroupCover::thresholds;

MinGroupCover::MinGroupCover(const unsigned int groupIdParam, const unsigned int threshold): GroupMeasure(), groupId(groupIdParam)
{
  isSomeMeasureMonotone = true;
  if (groupId >= thresholds.size())
    {
      thresholds.resize(groupId + 1);
    }
  thresholds[groupId] = threshold;
}

MinGroupCover* MinGroupCover::clone() const
{
  return new MinGroupCover(*this);
}

bool MinGroupCover::monotone() const
{
  return true;
}

bool MinGroupCover::violationAfterMaxCoversDecreased() const
{
#ifdef DEBUG
  if (maxCoverOfGroup(groupId) < thresholds[groupId])
    {
      cout << thresholds[groupId] << "-minimal size constraint on group " << groupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return maxCoverOfGroup(groupId) < thresholds[groupId];
}

float MinGroupCover::optimisticValue() const
{
  return maxCoverOfGroup(groupId);
}

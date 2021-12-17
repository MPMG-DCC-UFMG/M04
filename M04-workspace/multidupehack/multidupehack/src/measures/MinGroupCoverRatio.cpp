// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinGroupCoverRatio.h"

vector<vector<float>> MinGroupCoverRatio::thresholds;

MinGroupCoverRatio::MinGroupCoverRatio(const unsigned int numeratorGroupIdParam, const unsigned int denominatorGroupIdParam, const float threshold): GroupMeasure(), numeratorGroupId(numeratorGroupIdParam), denominatorGroupId(denominatorGroupIdParam)
{
  isSomeMeasureMonotone = true;
  isSomeMeasureAntiMonotone = true;
  if (numeratorGroupId >= thresholds.size())
    {
      thresholds.resize(numeratorGroupId + 1);
    }
  if (denominatorGroupId >= thresholds[numeratorGroupId].size())
    {
      thresholds[numeratorGroupId].resize(denominatorGroupId + 1);
    }
  thresholds[numeratorGroupId][denominatorGroupId] = threshold;
}

MinGroupCoverRatio* MinGroupCoverRatio::clone() const
{
  return new MinGroupCoverRatio(*this);
}

bool MinGroupCoverRatio::violationAfterMinCoversIncreased() const
{
#ifdef DEBUG
  if (maxCoverOfGroup(numeratorGroupId) < thresholds[numeratorGroupId][denominatorGroupId] * minCoverOfGroup(denominatorGroupId))
    {
      cout << thresholds[numeratorGroupId][denominatorGroupId] << "-minimal cover ratio constraint between group " << numeratorGroupId << " and group " << denominatorGroupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return maxCoverOfGroup(numeratorGroupId) < thresholds[numeratorGroupId][denominatorGroupId] * minCoverOfGroup(denominatorGroupId);
}

bool MinGroupCoverRatio::violationAfterMaxCoversDecreased() const
{
#ifdef DEBUG
  if (maxCoverOfGroup(numeratorGroupId) < thresholds[numeratorGroupId][denominatorGroupId] * minCoverOfGroup(denominatorGroupId))
    {
      cout << thresholds[numeratorGroupId][denominatorGroupId] << "-minimal cover ratio constraint between group " << numeratorGroupId << " and group " << denominatorGroupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return maxCoverOfGroup(numeratorGroupId) < thresholds[numeratorGroupId][denominatorGroupId] * minCoverOfGroup(denominatorGroupId);
}

float MinGroupCoverRatio::optimisticValue() const
{
  const unsigned int numerator = maxCoverOfGroup(numeratorGroupId);
  if (numerator)
    {
      return static_cast<float>(numerator) / minCoverOfGroup(denominatorGroupId);
    }
  return 0;
}

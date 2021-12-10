// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinGroupCoverYulesY.h"

vector<vector<float>> MinGroupCoverYulesY::thresholds;
vector<unsigned int> MinGroupCoverYulesY::nbOfElementsInGroups;

MinGroupCoverYulesY::MinGroupCoverYulesY(const unsigned int numeratorGroupIdParam, const unsigned int denominatorGroupIdParam, const float threshold): GroupMeasure(), numeratorGroupId(numeratorGroupIdParam), denominatorGroupId(denominatorGroupIdParam)
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
  const unsigned int maxGroupId = max(numeratorGroupId, denominatorGroupId);
  if (maxGroupId >= nbOfElementsInGroups.size())
    {
      nbOfElementsInGroups.resize(maxGroupId + 1);
    }
  nbOfElementsInGroups[numeratorGroupId] = maxCoverOfGroup(numeratorGroupId);
  nbOfElementsInGroups[denominatorGroupId] = maxCoverOfGroup(denominatorGroupId);
}

MinGroupCoverYulesY* MinGroupCoverYulesY::clone() const
{
  return new MinGroupCoverYulesY(*this);
}

bool MinGroupCoverYulesY::violationAfterMinCoversIncreased() const
{
#ifdef DEBUG
  if (optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId])
    {
      cout << thresholds[numeratorGroupId][denominatorGroupId] << "-minimal cover Yule's Y constraint between group " << numeratorGroupId << " and group " << denominatorGroupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId];
}

bool MinGroupCoverYulesY::violationAfterMaxCoversDecreased() const
{
#ifdef DEBUG
  if (optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId])
    {
      cout << thresholds[numeratorGroupId][denominatorGroupId] << "-minimal cover Yule's Y constraint between group " << numeratorGroupId << " and group " << denominatorGroupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId];
}

float MinGroupCoverYulesY::optimisticValue() const
{
  const unsigned int maxCoverOfNumeratorGroup = maxCoverOfGroup(numeratorGroupId);
  if (!maxCoverOfNumeratorGroup)
    {
      return -1;
    }
  const float tmp = sqrt(minCoverOfGroup(denominatorGroupId) * (nbOfElementsInGroups[numeratorGroupId] - maxCoverOfNumeratorGroup));
  const unsigned int nbOfElementsInBothGroups = nbOfElementsInGroups[numeratorGroupId] + nbOfElementsInGroups[denominatorGroupId];
  const unsigned int minCoverOfNumeratorGroup = minCoverOfGroup(numeratorGroupId);
  const float denominator = sqrt(minCoverOfNumeratorGroup * (nbOfElementsInBothGroups - maxCoverOfNumeratorGroup)) + tmp;
  if (denominator)
    {
      return (sqrt(maxCoverOfNumeratorGroup * (nbOfElementsInBothGroups - minCoverOfNumeratorGroup)) - tmp) / denominator;
    }
  return 1;
}

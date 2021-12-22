// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "MinGroupCoverForce.h"

vector<vector<float>> MinGroupCoverForce::thresholds;

MinGroupCoverForce::MinGroupCoverForce(const unsigned int numeratorGroupIdParam, const unsigned int denominatorGroupIdParam, const float threshold): GroupMeasure(), numeratorGroupId(numeratorGroupIdParam), denominatorGroupId(denominatorGroupIdParam)
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
  const unsigned int nbOfElementsInNumeratorGroup = maxCoverOfGroup(numeratorGroupId);
  const unsigned int nbOfElementsInDenominatorGroup = maxCoverOfGroup(denominatorGroupId);
  thresholds[numeratorGroupId][denominatorGroupId] = threshold * nbOfElementsInNumeratorGroup / (nbOfElementsInDenominatorGroup * (nbOfElementsInNumeratorGroup + nbOfElementsInDenominatorGroup));
}

MinGroupCoverForce* MinGroupCoverForce::clone() const
{
  return new MinGroupCoverForce(*this);
}

bool MinGroupCoverForce::violationAfterMinCoversIncreased() const
{
#ifdef DEBUG
  if (optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId])
    {
      cout << thresholds[numeratorGroupId][denominatorGroupId] << "-minimal cover force constrint between group " << numeratorGroupId << " and group " << denominatorGroupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId];
}

bool MinGroupCoverForce::violationAfterMaxCoversDecreased() const
{
#ifdef DEBUG
  if (optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId])
    {
      cout << thresholds[numeratorGroupId][denominatorGroupId] << "-minimal cover force constrint between group " << numeratorGroupId << " and group " << denominatorGroupId << " cannot be satisfied -> Prune!\n";
    }
#endif
  return optimisticValue() < thresholds[numeratorGroupId][denominatorGroupId];
}

float MinGroupCoverForce::optimisticValue() const
{
  const unsigned int maxCoverOfNumeratorGroup = maxCoverOfGroup(numeratorGroupId);
  if (!maxCoverOfNumeratorGroup)
    {
      return 0;
    }
  const unsigned int minCoverOfDenominatorGroup = minCoverOfGroup(denominatorGroupId);
  return static_cast<float>(maxCoverOfNumeratorGroup * maxCoverOfNumeratorGroup) / ((minCoverOfGroup(numeratorGroupId) + minCoverOfDenominatorGroup) * minCoverOfDenominatorGroup);
}

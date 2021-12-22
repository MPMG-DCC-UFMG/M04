// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MIN_GROUP_COVER_YULE_S_Y_H_
#define MIN_GROUP_COVER_YULE_S_Y_H_

#include "GroupMeasure.h"

class MinGroupCoverYulesY final : public GroupMeasure
{
 public:
  MinGroupCoverYulesY(const unsigned int numeratorGroupId, const unsigned int denominatorGroupId, const float threshold);
  MinGroupCoverYulesY* clone() const;

  bool violationAfterMinCoversIncreased() const;
  bool violationAfterMaxCoversDecreased() const;
  float optimisticValue() const;

 private:
  const unsigned int numeratorGroupId;
  const unsigned int denominatorGroupId;

  static vector<vector<float>> thresholds;
  static vector<unsigned int> nbOfElementsInGroups;
};

#endif /*MIN_GROUP_COVER_YULE_S_Y_H_*/

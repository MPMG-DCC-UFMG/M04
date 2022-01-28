// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef GROUP_MEASURE_H
#define GROUP_MEASURE_H

#include "Measure.h"
#include "GroupCovers.h"

class GroupMeasure : public Measure
{
 public:
  GroupMeasure();
  GroupMeasure(const GroupMeasure& otherGroupMeasure);
  GroupMeasure(GroupMeasure&& otherGroupMeasure);
  virtual ~GroupMeasure();

  GroupMeasure& operator=(const GroupMeasure& otherGroupMeasure);
  GroupMeasure& operator=(GroupMeasure&& otherGroupMeasure);

  static void initGroups(const vector<string>& groupFileNames, const char* groupElementSeparator, const char* groupDimensionElementsSeparator, const vector<unsigned int>& cardinalities, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder);
  static void allMeasuresSet(vector<Measure*>& mereConstraints); /* must be called after all measures are sorted in Tree::initMeasures */
  static void allMeasuresSet(vector<Measure*>& mereConstraints, vector<Measure*>& nonMinSizeMeasuresToMaximize); /* must be called after all measures are sorted in SkyPatternTree::initMeasures */
  static unsigned int minCoverOfGroup(const unsigned int groupId);
  static unsigned int maxCoverOfGroup(const unsigned int groupId);

  bool violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  bool violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);

 protected:
  static bool isSomeMeasureMonotone;
  static bool isSomeMeasureAntiMonotone;

 private:
  static vector<GroupMeasure*> firstMeasures;
  static vector<GroupCovers*> groupCovers;

  virtual bool violationAfterMinCoversIncreased() const;
  virtual bool violationAfterMaxCoversDecreased() const;

  static void clearUselessGroupCovers();
};

#endif /*GROUP_MEASURE_H*/

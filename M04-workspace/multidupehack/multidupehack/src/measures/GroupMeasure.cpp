// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include "GroupMeasure.h"

bool GroupMeasure::isSomeMeasureMonotone = false;
bool GroupMeasure::isSomeMeasureAntiMonotone = false;
vector<GroupMeasure*> GroupMeasure::firstMeasures;
vector<GroupCovers*> GroupMeasure::groupCovers;

GroupMeasure::GroupMeasure()
{
}

GroupMeasure::GroupMeasure(const GroupMeasure& otherGroupMeasure)
{
  if (&otherGroupMeasure == firstMeasures.back())
    {
      firstMeasures.push_back(this);
      groupCovers.push_back(new GroupCovers(*(groupCovers.back())));
    }
}

GroupMeasure::GroupMeasure(GroupMeasure&& otherGroupMeasure)
{
}

GroupMeasure::~GroupMeasure()
{
  if (!firstMeasures.empty() && firstMeasures.back() == this)
    {
      firstMeasures.pop_back();
      if (!groupCovers.empty())
	{
	  delete groupCovers.back();
	  groupCovers.pop_back();
	}
    }
}

GroupMeasure& GroupMeasure::operator=(const GroupMeasure& otherGroupMeasure)
{
  if (&otherGroupMeasure == firstMeasures.back())
    {
      groupCovers.push_back(new GroupCovers(*(groupCovers.back())));
    }
  return *this;
}

GroupMeasure& GroupMeasure::operator=(GroupMeasure&& otherGroupMeasure)
{
  return *this;
}

void GroupMeasure::initGroups(const vector<string>& groupFileNames, const char* groupElementSeparator, const char* groupDimensionElementsSeparator, const vector<unsigned int>& cardinalities, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder)
{
  groupCovers.push_back(new GroupCovers(groupFileNames, groupElementSeparator, groupDimensionElementsSeparator, cardinalities, labels2Ids, dimensionOrder));
}

void GroupMeasure::clearUselessGroupCovers()
{
  // If unnecessary, clear the maximal group covers (in this way, do not copy them at every copy of the group cover measures)
  if (!isSomeMeasureAntiMonotone)
    {
      groupCovers.back()->clearMinCovers();
      return;
    }
  if (!isSomeMeasureMonotone)
    {
      groupCovers.back()->clearMaxCovers();
    }
}

void GroupMeasure::allMeasuresSet(vector<Measure*>& mereConstraints)
{
  if (isSomeMeasureMonotone || isSomeMeasureAntiMonotone)
    {
      clearUselessGroupCovers();
      // Set the first groupMeasure* in firstMeasures
      vector<Measure*>::iterator measureIt = mereConstraints.begin();
      for (; !dynamic_cast<GroupMeasure*>(*measureIt); ++measureIt)
	{
	}
      firstMeasures.push_back(static_cast<GroupMeasure*>(*measureIt));
    }
}

void GroupMeasure::allMeasuresSet(vector<Measure*>& mereConstraints, vector<Measure*>& nonMinSizeMeasuresToMaximize)
{
  if (isSomeMeasureMonotone || isSomeMeasureAntiMonotone)
    {
      clearUselessGroupCovers();
      // Set the first groupMeasure* in firstMeasures
      for (Measure* measure : mereConstraints)
	{
	  if (dynamic_cast<GroupMeasure*>(measure))
	    {
	      firstMeasures.push_back(static_cast<GroupMeasure*>(measure));
	      return;
	    }
	}
      vector<Measure*>::iterator measureIt = nonMinSizeMeasuresToMaximize.begin();
      for (; !dynamic_cast<GroupMeasure*>(*measureIt); ++measureIt)
	{
	}
      firstMeasures.push_back(static_cast<GroupMeasure*>(*measureIt));
    }
}

unsigned int GroupMeasure::minCoverOfGroup(const unsigned int groupId)
{
  return groupCovers.back()->minCoverOfGroup(groupId);
}

unsigned int GroupMeasure::maxCoverOfGroup(const unsigned int groupId)
{
  return groupCovers.back()->maxCoverOfGroup(groupId);
}

bool GroupMeasure::violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  if (isSomeMeasureAntiMonotone)
    {
      if (this == firstMeasures.back())
	{
	  groupCovers.back()->add(dimensionIdOfElementsSetPresent, elementsSetPresent);
	}
      return violationAfterMinCoversIncreased();
    }
  return false;
}

bool GroupMeasure::violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  if (isSomeMeasureMonotone)
    {
      if (this == firstMeasures.back())
	{
	  groupCovers.back()->remove(dimensionIdOfElementsSetAbsent, elementsSetAbsent);
	}
      return violationAfterMaxCoversDecreased();
    }
  return false;
}

bool GroupMeasure::violationAfterMinCoversIncreased() const
{
  return false;
}

bool GroupMeasure::violationAfterMaxCoversDecreased() const
{
  return false;
}

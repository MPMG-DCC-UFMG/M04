// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef GROUP_COVERS_H
#define GROUP_COVERS_H

#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/dynamic_bitset.hpp>

#include "../../Parameters.h"
#include "../utilities/NoInputException.h"
#include "../utilities/DataFormatException.h"

using namespace boost;

class GroupCovers
{
 public:
  GroupCovers(const vector<string>& groupFileNames, const char* groupElementSeparator, const char* groupDimensionElementsSeparator, const vector<unsigned int>& cardinalities, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder);

  friend ostream& operator<<(ostream& out, const GroupCovers& groupCovers);

  unsigned int minCoverOfGroup(const unsigned int groupId) const;
  unsigned int maxCoverOfGroup(const unsigned int groupId) const;

  void clearMinCovers();
  void clearMaxCovers();

  void add(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  void remove(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);

 private:
  vector<unsigned int> minCovers;
  vector<unsigned int> maxCovers;

  static vector<vector<dynamic_bitset<>>> groups; /* never modified after initialized */
};

#endif /*GROUP_COVERS_H*/

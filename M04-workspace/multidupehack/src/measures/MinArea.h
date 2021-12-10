// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MIN_AREA_H_
#define MIN_AREA_H_

#include "Measure.h"

class MinArea final : public Measure
{
 public:
  MinArea(const vector<unsigned int>& nbOfElementsInDimensions, const unsigned int threshold);
  MinArea* clone() const;

  bool monotone() const;
  bool violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);
  float optimisticValue() const;

 private:
  vector<unsigned int> nbOfElementsInDimensions;
  unsigned int minArea;

  static unsigned int threshold;
};

#endif /*MIN_AREA_H_*/

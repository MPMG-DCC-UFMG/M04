// Copyright (C) 2013-2019 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MEASURE_H
#define MEASURE_H

#include "../../Parameters.h"

#include <vector>

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

class Measure
{
 public:
  virtual ~Measure();
  virtual Measure* clone() const = 0;

  virtual bool monotone() const;
  virtual bool violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  virtual bool violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);
  virtual float optimisticValue() const = 0;

#ifdef DEBUG
  static void setInternal2ExternalAttributeOrder(const vector<unsigned int>& internal2ExternalAttributeOrder);

 protected:
  static vector<unsigned int> internal2ExternalAttributeOrder;
#endif
};

#endif /*MEASURE_H*/

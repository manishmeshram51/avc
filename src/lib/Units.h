/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LIBPAGEMAKER_UNITS_H
#define LIBPAGEMAKER_UNITS_H

#include <string>

// TODO: remove this pointless abstraction.

namespace libpagemaker
{
const unsigned SHAPE_UNITS_PER_INCH = 1440;

template <unsigned PER_INCH> class LengthUnit
{
  typedef LengthUnit<PER_INCH> T;
public:
  int m_value;

  LengthUnit(int value) : m_value(value) { }

  double toInches() const
  {
    return m_value / ((double)PER_INCH);
  }
};

template<unsigned PER_INCH> const LengthUnit<PER_INCH>
operator+(LengthUnit<PER_INCH> left, LengthUnit<PER_INCH> right)
{
  return LengthUnit<PER_INCH>(left.m_value + right.m_value);
}

template<unsigned PER_INCH> const LengthUnit<PER_INCH>
operator*(LengthUnit<PER_INCH> left, int right)
{
  return LengthUnit<PER_INCH>(left.m_value * right);
}

template<unsigned PER_INCH> const LengthUnit<PER_INCH>
operator*(int left, LengthUnit<PER_INCH> right)
{
  return right * left;
}

template<unsigned PER_INCH> const LengthUnit<PER_INCH>
operator-(LengthUnit<PER_INCH> left, LengthUnit<PER_INCH> right)
{
  return LengthUnit<PER_INCH>(left.m_value - right.m_value);
}

typedef LengthUnit<SHAPE_UNITS_PER_INCH> PMDShapeUnit;

}

#endif /* LIBPAGEMAKER_UNITS_H */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

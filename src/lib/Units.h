#pragma once
#include <string>
#include "yaml_utils.h"

namespace libpagemaker
{
const unsigned PAGE_UNITS_PER_INCH = 720;
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

  void emitYaml(yaml_emitter_t *emitter) const
  {
    yamlTryPutScalar(emitter, m_value);
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

typedef LengthUnit<PAGE_UNITS_PER_INCH> PMDPageUnit;
typedef LengthUnit<SHAPE_UNITS_PER_INCH> PMDShapeUnit;

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

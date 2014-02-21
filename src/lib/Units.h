#pragma once
#include <string>
#include "yaml_utils.h"

namespace libpagemaker
{
const unsigned PAGE_UNITS_PER_INCH = 720;
const unsigned SHAPE_UNITS_PER_INCH = 1440;

const char UNKNOWN_UNIT[] = "Unknown Unit";
template <unsigned PER_INCH, const char *UNIT_NAME=UNKNOWN_UNIT> class LengthUnit
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

template<unsigned PER_INCH, const char *UNIT_NAME> const LengthUnit<PER_INCH, UNIT_NAME> 
  operator+(LengthUnit<PER_INCH> left, LengthUnit<PER_INCH> right)
{
  return LengthUnit<PER_INCH, UNIT_NAME>(left.m_value + right.m_value);
}

template<unsigned PER_INCH, const char *UNIT_NAME> const LengthUnit<PER_INCH, UNIT_NAME>
  operator*(LengthUnit<PER_INCH, UNIT_NAME> left, int right)
{
  return LengthUnit<PER_INCH, UNIT_NAME>(left.m_value * right);
}

template<unsigned PER_INCH, const char *UNIT_NAME> const LengthUnit<PER_INCH, UNIT_NAME>
  operator*(int left, LengthUnit<PER_INCH, UNIT_NAME> right)
{
  return right * left;
}

template<unsigned PER_INCH, const char *UNIT_NAME> const LengthUnit<PER_INCH, UNIT_NAME>
  operator-(LengthUnit<PER_INCH, UNIT_NAME> left, LengthUnit<PER_INCH, UNIT_NAME> right)
{
  return LengthUnit<PER_INCH, UNIT_NAME>(left.m_value - right.m_value);
}

const char PMD_PAGE_UNIT[] = "PMD Page Unit";
const char PMD_SHAPE_UNIT[] = "PMD Shape Unit";

typedef LengthUnit<PAGE_UNITS_PER_INCH, PMD_PAGE_UNIT> PMDPageUnit;
typedef LengthUnit<SHAPE_UNITS_PER_INCH, PMD_SHAPE_UNIT> PMDShapeUnit;

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

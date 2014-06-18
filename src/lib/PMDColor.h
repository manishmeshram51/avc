#pragma once
#include "Units.h"
namespace libpagemaker
{

class PMDColor
{
public:
  unsigned m_i;
  uint16_t m_red;
  uint16_t m_green;
  uint16_t m_blue;

  PMDColor(const unsigned i,const uint16_t red,const uint16_t green,const uint16_t blue)
    : m_i(i), m_red(red), m_green(green), m_blue(blue)
  { }

  virtual ~PMDColor()
  {
  }

};
}

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDTYPES_H__
#define __PMDTYPES_H__

#include "Units.h"
namespace libpagemaker
{

struct PMDColor
{
  unsigned m_i;
  uint16_t m_red;
  uint16_t m_green;
  uint16_t m_blue;

  PMDColor(const unsigned i,const uint16_t red,const uint16_t green,const uint16_t blue)
    : m_i(i), m_red(red), m_green(green), m_blue(blue)
  { }
};

struct PMDFont
{
public:
  unsigned m_i;
  std::string m_fontName;

  PMDFont(const unsigned i,const std::string &fontName)
    : m_i(i), m_fontName(fontName)
  { }
};

struct PMDFillProperties
{
  uint8_t m_fillType;
  uint8_t m_fillColor;
  uint8_t m_fillOverprint;
  uint8_t m_fillTint;

  PMDFillProperties(const uint8_t fillType,const uint8_t fillColor,const uint8_t fillOverprint,const uint8_t fillTint)
    : m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint)
  { }
};

struct PMDStrokeProperties
{
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

  PMDStrokeProperties(const uint8_t strokeType,const uint8_t strokeWidth,const uint8_t strokeColor,const uint8_t strokeOverprint,const uint8_t strokeTint)
    : m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }
};

}

#endif // __PMDTYPES_H__

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

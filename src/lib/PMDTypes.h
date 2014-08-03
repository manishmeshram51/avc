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
  uint16_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

  PMDStrokeProperties(const uint8_t strokeType,const uint16_t strokeWidth,const uint8_t strokeColor,const uint8_t strokeOverprint,const uint8_t strokeTint)
    : m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }
};

struct PMDParaProperties
{
  uint16_t m_length;
  uint8_t m_align;
  uint16_t m_leftIndent;
  uint16_t m_firstIndent;
  uint16_t m_rightIndent;
  uint16_t m_beforeIndent;
  uint16_t m_afterIndent;

  PMDParaProperties(const uint16_t length, const uint8_t align,const uint16_t leftIndent,const uint16_t firstIndent,const uint16_t rightIndent,const uint16_t beforeIndent,const uint16_t afterIndent)
    : m_length(length), m_align(align), m_leftIndent(leftIndent), m_firstIndent(firstIndent), m_rightIndent(rightIndent), m_beforeIndent(beforeIndent), m_afterIndent(afterIndent)
  { }
};

struct PMDCharProperties
{
  uint16_t m_length;
  uint16_t m_fontFace;
  uint16_t m_fontSize;
  uint8_t m_fontColor;
  uint8_t m_boldItalicUnderline;
  uint8_t m_superSubscript;
  int16_t m_kerning;
  uint16_t m_superSubSize;
  uint16_t m_superPos;
  uint16_t m_subPos;
  uint8_t m_tint;

  PMDCharProperties(const uint16_t length, const uint16_t fontFace, const uint16_t fontSize, const uint8_t fontColor, const uint8_t boldItalicUnderline, const uint8_t superSubscript, const int16_t kerning, const uint16_t superSubSize, const uint16_t superPos, const uint16_t subPos, const uint16_t tint)
    : m_length(length), m_fontFace(fontFace), m_fontSize(fontSize), m_fontColor(fontColor), m_boldItalicUnderline(boldItalicUnderline), m_superSubscript(superSubscript), m_kerning(kerning), m_superSubSize(superSubSize), m_superPos(superPos), m_subPos(subPos), m_tint(tint)
  { }
};

}

#endif // __PMDTYPES_H__

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

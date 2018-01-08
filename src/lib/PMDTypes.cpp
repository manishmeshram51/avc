/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "PMDTypes.h"

#include "constants.h"

namespace libpagemaker
{

PMDFillProperties::PMDFillProperties()
  : m_fillType(FILL_NONE)
  , m_fillColor()
  , m_fillOverprint(0)
  , m_fillTint(0)
{
}

PMDStrokeProperties::PMDStrokeProperties()
  : m_strokeType(STROKE_NORMAL)
  , m_strokeWidth(0)
  , m_strokeColor(0)
  , m_strokeOverprint(0)
  , m_strokeTint(0)
{
}

PMDParaProperties::PMDParaProperties()
  : m_length(0)
  , m_align(0)
  , m_leftIndent(0)
  , m_firstIndent(0)
  , m_rightIndent(0)
  , m_beforeIndent(0)
  , m_afterIndent(0)
  , m_orphans(0)
  , m_widows(0)
  , m_keepWithNext(0)
  , m_keepTogether(false)
  , m_hyphenate(false)
  , m_hyphensCount(0)
{
}

PMDCharProperties::PMDCharProperties()
  : m_length(0)
  , m_fontFace(0)
  , m_fontSize(0)
  , m_fontColor(0)
  , m_bold(false)
  , m_italic(false)
  , m_underline(false)
  , m_outline(false)
  , m_shadow(false)
  , m_strike(false)
  , m_super(false)
  , m_sub(false)
  , m_smallCaps(false)
  , m_allCaps(false)
  , m_kerning(0)
  , m_superSubSize(0)
  , m_superPos(0)
  , m_subPos(0)
  , m_tint(0)
{
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

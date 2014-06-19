/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDCOLOR_H__
#define __PMDCOLOR_H__

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
}

#endif // __PMDCOLOR_H__

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

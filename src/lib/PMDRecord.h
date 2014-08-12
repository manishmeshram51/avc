/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDRECORD_H__
#define __PMDRECORD_H__

#include <stdint.h>
#include <librevenge/librevenge.h>
#include <boost/optional.hpp>
#include <vector>

#include "constants.h"

namespace libpagemaker
{

struct PMDRecordContainer
{
  uint16_t m_recordType;
  uint32_t m_offset;
  unsigned m_seqNum;
  uint16_t m_numRecords;

  PMDRecordContainer(uint16_t recordType, uint32_t offset, unsigned seqNum, uint16_t numRecords)
    : m_recordType(recordType), m_offset(offset), m_seqNum(seqNum),
      m_numRecords(numRecords)
  { }
};

inline boost::optional<unsigned> getRecordSize(uint16_t recType)
{
  switch (recType)
  {
  case SHAPE:
    return 258;
  case GLOBAL_INFO:
    return 2496;
  case PAGE:
    return 472;
  case LINE_SET:
    return 4;
  case XFORM:
    return 26;
  case TEXT_BLOCK:
    return 36;
  case CHARS:
    return 30;
  case PARA:
    return 80;
  case FONTS:
    return 94;
  case FONTS_PARENT:
    return 10;
  case COLORS:
    return 210;
  default:
    return boost::none;
  }
}

}

#endif /* __PMDRECORD_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

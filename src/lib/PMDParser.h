/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDPARSER_H__
#define __PMDPARSER_H__

#include <librevenge/librevenge.h>

#include <stdint.h>
#include <map>

#include "PMDRecord.h"

namespace libpagemaker
{

class PMDCollector;
class PMDParser
{
  librevenge::RVNGInputStream *m_input;
  PMDCollector *m_collector;
  std::map<uint16_t, std::vector<unsigned> > m_records;
  bool m_bigEndian;
  std::vector<PMDRecordContainer> m_recordsInOrder;

  /* Private functions. */
  void parseGlobalInfo(PMDRecordContainer);
  void parseColors(PMDRecordContainer);
  void parsePages(PMDRecordContainer);
  void parseShapes(uint16_t seqNum, unsigned pageID);
  void parseLine(PMDRecordContainer, unsigned recordIndex, unsigned pageID);
  void parseTextBox(PMDRecordContainer, unsigned recordIndex, unsigned pageID);
  void parseRectangle(PMDRecordContainer, unsigned recordIndex, unsigned pageID);
  void parsePolygon(PMDRecordContainer, unsigned recordIndex, unsigned pageID);
  void parseEllipse(PMDRecordContainer, unsigned recordIndex, unsigned pageID);
  void parseHeader(uint32_t *tocOffset, uint16_t *tocLength);
  unsigned readNextRecordFromTableOfContents(unsigned seqNum);
  void parseTableOfContents(uint32_t offset, uint16_t length);

  /* Prevent copy and assignment */
  PMDParser &operator=(const PMDParser &);
  PMDParser(const PMDParser &);
public:
  PMDParser(librevenge::RVNGInputStream *, PMDCollector *);
  void parse();
};

}

#endif /* __PMDPARSER_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

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
#include "geometry.h"

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
  std::map<uint32_t, PMDXForm> m_xFormMap;

  /* Private functions. */
  void parseGlobalInfo(const PMDRecordContainer &container);
  void parseFonts();
  void parseColors();
  void parsePages(const PMDRecordContainer &container);
  void parseShapes(uint16_t seqNum, unsigned pageID);
  void parseLine(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID);
  void parseTextBox(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID);
  void parseRectangle(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID);
  void parsePolygon(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID);
  void parseEllipse(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID);
  void parseBitmap(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID);
  void parseHeader(uint32_t *tocOffset, uint16_t *tocLength);
  unsigned readNextRecordFromTableOfContents(unsigned seqNum);
  void parseTableOfContents(uint32_t offset, uint16_t length);
  std::vector<PMDRecordContainer> getRecordsBySeqNum(const uint16_t seqNum);
  std::vector<PMDRecordContainer> getRecordsByRecType(const uint16_t recType);
  const PMDRecordContainer &getSingleRecordBySeqNum(const uint16_t seqNum) const;
  void parseXforms();
  const PMDXForm &getXForm(const uint32_t xFormId) const;

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

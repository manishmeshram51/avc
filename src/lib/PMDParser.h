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

#include <map>
#include <vector>

#include <librevenge/librevenge.h>

#include "PMDRecord.h"
#include "geometry.h"

namespace libpagemaker
{

class PMDCollector;
class PMDParser
{
  typedef std::vector<PMDRecordContainer> RecordContainerList_t;
  typedef std::map<uint16_t, std::vector<unsigned> > RecordTypeMap_t;

  librevenge::RVNGInputStream *m_input;
  unsigned long m_length;
  PMDCollector *m_collector;
  RecordTypeMap_t m_records;
  bool m_bigEndian;
  RecordContainerList_t m_recordsInOrder;
  std::map<uint32_t, PMDXForm> m_xFormMap;

  struct ToCState;
  class RecordIterator;

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
  void readNextRecordFromTableOfContents(ToCState &state, bool subRecord, uint16_t subRecordType = 0);
  void readTableOfContents(ToCState &state, uint32_t offset, unsigned records, bool subRecords, uint16_t subRecordType = 0);
  void parseTableOfContents(uint32_t offset, uint16_t length);
  void parseXforms();
  const PMDXForm &getXForm(const uint32_t xFormId) const;

  RecordIterator beginRecordsWithSeqNumber(uint16_t seqNum) const;
  RecordIterator beginRecordsOfType(uint16_t recType) const;
  RecordIterator endRecords() const;

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

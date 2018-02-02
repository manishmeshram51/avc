/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "PMDParser.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <memory>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>

#include <librevenge/librevenge.h>

#include "PMDCollector.h"
#include "PMDExceptions.h"
#include "PMDRecord.h"
#include "PMDTypes.h"
#include "Units.h"
#include "constants.h"
#include "geometry.h"
#include "libpagemaker_utils.h"
#include "offsets.h"

namespace libpagemaker
{

namespace
{

void readDims(librevenge::RVNGInputStream *input, bool bigEndian, int16_t &x, int16_t &y)
{
  int16_t dim1 = readS16(input, bigEndian);
  int16_t dim2 = readS16(input, bigEndian);
  x = bigEndian ? dim2 : dim1;
  y = bigEndian ? dim1 : dim2;
}

boost::optional<PMDStrokeProperties> readRule(librevenge::RVNGInputStream *input, bool bigEndian)
{
  const uint16_t flags = readU16(input, bigEndian);
  if (!(flags & 0x1))
  {
    skip(input, 18);
    return boost::none;
  }

  PMDStrokeProperties stroke;

  stroke.m_strokeType = readU8(input, bigEndian);
  skip(input, 1);
  // FIXME: needs fixing of reading of stroke width elsewhere
  stroke.m_strokeWidth = uint16_t(readU32(input, bigEndian) >> 8);
  stroke.m_strokeColor = readU16(input, bigEndian);
  stroke.m_strokeTint = readU16(input, bigEndian);
  skip(input, 6);

  return stroke;
}

}

struct PMDParser::ToCState
{
  ToCState();

  std::set<unsigned long> parsedBlocks;
  unsigned seqNum;
};

PMDParser::ToCState::ToCState()
  : parsedBlocks()
  , seqNum(0)
{
}

class PMDParser::RecordIterator :
  public boost::iterator_facade
  < PMDParser::RecordIterator
  , RecordContainerList_t::const_iterator::value_type
  , std::bidirectional_iterator_tag
  , RecordContainerList_t::const_iterator::reference
  , RecordContainerList_t::const_iterator::difference_type
  >
{
public:
  RecordIterator(const RecordContainerList_t &records);
  RecordIterator(const RecordContainerList_t &records, uint16_t seqNum);
  RecordIterator(const RecordContainerList_t &records, const RecordTypeMap_t &types, uint16_t recType);

private:
  friend class boost::iterator_core_access;

  reference dereference() const;
  bool equal(const RecordIterator &other) const;
  void increment();
  void decrement();

private:
  RecordContainerList_t::const_iterator m_it;
  RecordContainerList_t::const_iterator m_begin;
  RecordContainerList_t::const_iterator m_end;
  boost::optional<uint16_t> m_selector;
  boost::optional<RecordTypeMap_t::mapped_type::const_iterator> m_recIt;
  RecordTypeMap_t::mapped_type::const_iterator m_recBegin;
  RecordTypeMap_t::mapped_type::const_iterator m_recEnd;
};

PMDParser::RecordIterator::RecordIterator(const RecordContainerList_t &records)
  : m_it(records.end())
  , m_begin(records.begin())
  , m_end(records.end())
  , m_selector()
  , m_recIt()
  , m_recBegin()
  , m_recEnd()
{
}

PMDParser::RecordIterator::RecordIterator(const RecordContainerList_t &records, const uint16_t seqNum)
  : m_it(records.begin())
  , m_begin(records.begin())
  , m_end(records.end())
  , m_selector(seqNum)
  , m_recIt()
  , m_recBegin()
  , m_recEnd()
{
  increment();
}

PMDParser::RecordIterator::RecordIterator(const RecordContainerList_t &records, const RecordTypeMap_t &types, const uint16_t recType)
  : m_it(records.end())
  , m_begin(records.begin())
  , m_end(records.end())
  , m_selector()
  , m_recIt()
  , m_recBegin()
  , m_recEnd()
{
  const RecordTypeMap_t::const_iterator it = types.find(recType);
  if (it != types.end())
  {
    m_recBegin = it->second.begin();
    m_recIt = m_recBegin;
    m_recEnd = it->second.end();
    if (get(m_recIt) != m_recEnd)
      m_it = m_begin + *get(m_recIt);
  }
}

PMDParser::RecordIterator::reference PMDParser::RecordIterator::dereference() const
{
  return *m_it;
}

bool PMDParser::RecordIterator::equal(const RecordIterator &other) const
{
  return (m_it == m_end && other.m_it == other.m_end) || m_it == other.m_it;
}

void PMDParser::RecordIterator::increment()
{
  if (m_selector)
  {
    if (m_it != m_end)
    {
      ++m_it;
      while (m_it != m_end && m_it->m_seqNum != get(m_selector))
        ++m_it;
    }
  }
  else if (m_recIt && get(m_recIt) != m_recEnd)
  {
    ++get(m_recIt);
    if (get(m_recIt) == m_recEnd)
      m_it = m_end;
    else
      m_it = m_begin + *get(m_recIt);
  }
}

void PMDParser::RecordIterator::decrement()
{
  if (m_selector)
  {
    if (m_it != m_begin)
    {
      --m_it;
      while (m_it != m_begin && m_it->m_seqNum != get(m_selector))
        --m_it;
    }
  }
  else if (m_recIt && get(m_recIt) != m_recBegin)
  {
    --get(m_recIt);
    m_it = m_begin + *get(m_recIt);
  }
}

PMDParser::PMDParser(librevenge::RVNGInputStream *input, PMDCollector *collector)
  : m_input(input), m_length(getLength(input)), m_collector(collector),
    m_records(), m_bigEndian(false), m_recordsInOrder(), m_xFormMap()
{
}

const PMDXForm &PMDParser::getXForm(const uint32_t xFormId) const
{
  if (xFormId != (std::numeric_limits<uint32_t>::max)() && xFormId != 0)
  {
    auto it = m_xFormMap.find(xFormId);

    if (it != m_xFormMap.end())
      return it->second;
  }

  return m_xFormMap.find(0)->second;
}

void seekToRecord(librevenge::RVNGInputStream *const input, const PMDRecordContainer &container, const unsigned recordIndex)
{
  uint32_t recordOffset = container.m_offset;
  if (recordIndex > 0)
  {
    boost::optional<unsigned> sizePerRecord = getRecordSize(container.m_recordType);
    if (!sizePerRecord.is_initialized())
    {
      throw UnknownRecordSizeException(container.m_recordType);
    }
    recordOffset += sizePerRecord.get() * recordIndex;
  }
  seek(input, recordOffset);
}

PMDShapePoint readPoint(librevenge::RVNGInputStream *const input, const bool bigEndian)
{
  const PMDShapeUnit x(readS16(input, bigEndian));
  const PMDShapeUnit y(readS16(input, bigEndian));
  return bigEndian ? PMDShapePoint(y, x) : PMDShapePoint(x, y);
}

void PMDParser::parseGlobalInfo(const PMDRecordContainer &container)
{
  seekToRecord(m_input, container, 0);

  const unsigned opts = readU8(m_input, m_bigEndian);

  skip(m_input, 0x35);

  int16_t left = 0;
  int16_t right = 0;
  int16_t top = 0;
  int16_t bottom = 0;
  // FIXME: pass both pages' boundaries to collector instead of computed width/height
  readDims(m_input, m_bigEndian, left, top);
  readDims(m_input, m_bigEndian, right, bottom);

  m_collector->setDoubleSided(m_bigEndian ? opts & 0x40 : opts & 0x2);
  m_collector->setPageWidth(right - left);
  m_collector->setPageHeight(bottom - top);
}

void PMDParser::parseLine(const PMDRecordContainer &container, unsigned recordIndex,
                          unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  PMDStrokeProperties strokeProps;

  skip(m_input, 4);
  strokeProps.m_strokeColor = readU8(m_input);
  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);
  bool mirrored = false;
  skip(m_input, 0x18);
  uint16_t temp = readU16(m_input, m_bigEndian);

  if (temp != 257 && temp != 0)
    mirrored = true;

  skip(m_input, 6);
  strokeProps.m_strokeType = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeWidth =readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  strokeProps.m_strokeTint = readU8(m_input);
  skip(m_input, 6);
  strokeProps.m_strokeOverprint = readU8(m_input);

  std::shared_ptr<PMDLineSet> newShape(new PMDLine(bboxTopLeft, bboxBotRight, mirrored, strokeProps));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseTextBox(const PMDRecordContainer &container, unsigned recordIndex,
                             unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  skip(m_input, 6);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);

  uint16_t textBoxTextPropsOne = 0;
  uint16_t textBoxTextPropsTwo = 0;
  uint16_t textBoxTextStyle = 0;
  uint16_t textBoxText = 0;
  uint16_t textBoxChars = 0;
  uint16_t textBoxPara = 0;

  skip(m_input, 0xe);
  uint32_t textBoxXformId = readU32(m_input, m_bigEndian);
  uint32_t textBoxTextBlockId = readU32(m_input, m_bigEndian);

  const PMDXForm xFormContainer = getXForm(textBoxXformId);

  RecordIterator textBlockIt = beginRecordsOfType(TEXT_BLOCK);
  if (textBlockIt == endRecords())
  {
    PMD_ERR_MSG("No Text Block Record Found.\n");
  }

  for (; textBlockIt != endRecords(); ++textBlockIt)
  {
    const PMDRecordContainer &textBlockContainer = *textBlockIt;

    for (unsigned i = 0; i < textBlockContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, textBlockContainer, i);

      skip(m_input, 0x20);
      uint32_t textBlockId = readU32(m_input, m_bigEndian);

      if (textBlockId == textBoxTextBlockId)
      {
        seekToRecord(m_input, textBlockContainer, i); // return to the beginning of the record
        textBoxTextPropsOne = readU16(m_input, m_bigEndian);
        textBoxTextPropsTwo = readU16(m_input, m_bigEndian);
        textBoxText = readU16(m_input, m_bigEndian);
        textBoxChars = readU16(m_input, m_bigEndian);
        textBoxPara = readU16(m_input, m_bigEndian);
        textBoxTextStyle = readU16(m_input, m_bigEndian);

        (void) textBoxTextPropsOne;
        (void) textBoxTextPropsTwo;
        (void) textBoxTextStyle;
        PMD_DEBUG_MSG(("Text Box Props One is %x \n",textBoxTextPropsOne));
        PMD_DEBUG_MSG(("Text Box Props Two is %x \n",textBoxTextPropsTwo));
        PMD_DEBUG_MSG(("Text Box Style is %x \n",textBoxTextStyle));
        break;
      }

    }
  }
  std::string text = "";

  RecordIterator textIt = beginRecordsWithSeqNumber(textBoxText);
  if (textIt == endRecords())
  {
    PMD_ERR_MSG("No Text Found.\n");
  }

  for (; textIt != endRecords(); ++textIt)
  {
    const PMDRecordContainer &textContainer = *textIt;
    seekToRecord(m_input, textContainer, 0);
    for (unsigned i = 0; i < textContainer.m_numRecords; ++i)
    {
      text.push_back(readU8(m_input));
    }
  }

  std::vector<PMDCharProperties> charProps;
  for (RecordIterator it = beginRecordsWithSeqNumber(textBoxChars); it != endRecords(); ++it)
  {
    const PMDRecordContainer &charsContainer = *it;
    for (unsigned i = 0; i < charsContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, charsContainer, i);

      charProps.push_back(PMDCharProperties());
      auto &props = charProps.back();

      props.m_length = readU16(m_input, m_bigEndian);
      props.m_fontFace = readU16(m_input, m_bigEndian);
      props.m_fontSize = readU16(m_input, m_bigEndian);
      skip(m_input, 2);
      props.m_fontColor = readU16(m_input, m_bigEndian);
      const unsigned flags = readU16(m_input, m_bigEndian);
      props.m_bold = flags & 0x1;
      props.m_italic = flags & 0x2;
      props.m_underline = flags & 0x4;
      props.m_outline = flags & 0x8;
      props.m_shadow = flags & 0x10;
      props.m_strike = flags & 0x100;
      props.m_super = flags & 0x200;
      props.m_sub = flags & 0x400;
      props.m_allCaps = flags & 0x800;
      props.m_smallCaps = flags & 0x1000;
      skip(m_input, 4);
      props.m_kerning = readS16(m_input, m_bigEndian);
      skip(m_input, 2);
      props.m_superSubSize = readU16(m_input, m_bigEndian);
      props.m_subPos = readU16(m_input, m_bigEndian);
      props.m_superPos = readU16(m_input, m_bigEndian);
      skip(m_input, 2);
      props.m_tint = readU16(m_input, m_bigEndian);
    }
  }

  std::vector<PMDParaProperties> paraProps;
  for (RecordIterator it = beginRecordsWithSeqNumber(textBoxPara); it != endRecords(); ++it)
  {
    const PMDRecordContainer &paraContainer = *it;
    for (unsigned i = 0; i < paraContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, paraContainer, i);

      paraProps.push_back(PMDParaProperties());
      auto &props = paraProps.back();

      props.m_length = readU16(m_input, m_bigEndian);
      const unsigned flags = readU8(m_input, m_bigEndian);
      props.m_hyphenate = flags & 0x8;
      props.m_align = readU8(m_input);
      skip(m_input, 6);
      props.m_leftIndent = readU16(m_input, m_bigEndian);
      props.m_firstIndent = readU16(m_input, m_bigEndian);
      props.m_rightIndent = readU16(m_input, m_bigEndian);
      props.m_beforeIndent = readU16(m_input, m_bigEndian); // Above Para Spacing
      props.m_afterIndent = readU16(m_input, m_bigEndian); // Below Para Spacing
      skip(m_input, 18);
      props.m_hyphensCount = readU8(m_input, m_bigEndian);
      skip(m_input, 1);
      const unsigned keepOpts = readU16(m_input, m_bigEndian);
      props.m_keepTogether = keepOpts & 0x1;
      props.m_keepWithNext = (keepOpts >> 1) & 0x3;
      props.m_widows = (keepOpts >> 4) & 0x3;
      props.m_orphans = (keepOpts >> 7) & 0x3;
      skip(m_input, 2);
      props.m_ruleAbove = readRule(m_input, m_bigEndian);
      props.m_ruleBelow = readRule(m_input, m_bigEndian);
    }
  }

  std::shared_ptr<PMDLineSet> newShape(new PMDTextBox(bboxTopLeft, bboxBotRight, xFormContainer, text, charProps, paraProps));
  m_collector->addShapeToPage(pageID, newShape);

}

void PMDParser::parseRectangle(const PMDRecordContainer &container, unsigned recordIndex,
                               unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  PMDFillProperties fillProps;
  PMDStrokeProperties strokeProps;

  skip(m_input, 2);
  fillProps.m_fillOverprint = readU8(m_input);
  skip(m_input, 1);
  fillProps.m_fillColor = readU8(m_input);
  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);
  skip(m_input, 14);
  uint32_t rectXformId = readU32(m_input, m_bigEndian);

  strokeProps.m_strokeType = readU8(m_input);
  skip(m_input, 2);
  strokeProps.m_strokeWidth = readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  fillProps.m_fillType = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeColor = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeTint = readU8(m_input);

  skip(m_input, 0xb3);
  fillProps.m_fillTint = readU8(m_input);

  const PMDXForm &xFormContainer = getXForm(rectXformId);
  std::shared_ptr<PMDLineSet> newShape(new PMDRectangle(bboxTopLeft, bboxBotRight, xFormContainer, fillProps, strokeProps));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parsePolygon(const PMDRecordContainer &container, unsigned recordIndex,
                             unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  PMDFillProperties fillProps;
  PMDStrokeProperties strokeProps;

  skip(m_input, 2);
  fillProps.m_fillOverprint = readU8(m_input);
  skip(m_input, 1);
  fillProps.m_fillColor = readU8(m_input);

  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);

  skip(m_input, 14);
  uint32_t polyXformId = readU32(m_input, m_bigEndian);

  strokeProps.m_strokeType = readU8(m_input);
  skip(m_input, 2);
  strokeProps.m_strokeWidth = readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  fillProps.m_fillType = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeColor = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeTint = readU8(m_input);

  skip(m_input, 1);
  uint16_t lineSetSeqNum = readU16(m_input, m_bigEndian);
  skip(m_input, 8);
  uint8_t closedMarker = readU8(m_input);
  skip(m_input, 0xa7);
  fillProps.m_fillTint = readU8(m_input);

  bool closed;
  switch (closedMarker)
  {
  default:
    PMD_ERR_MSG("Unknown value for polygon closed/open marker. Defaulting to closed.\n");
  // Intentional fall-through.
  case POLYGON_CLOSED:
    closed = true;
    break;
  case POLYGON_OPEN:
    closed = false;
    break;
  case REGULAR_POLYGON:
    closed = true;
    break;
  }

  std::vector<PMDShapePoint> points;
  for (RecordIterator it = beginRecordsWithSeqNumber(lineSetSeqNum); it != endRecords(); ++it)
  {
    const PMDRecordContainer &lineSetContainer = *it;
    for (unsigned i = 0; i < lineSetContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, lineSetContainer, i);
      points.push_back(readPoint(m_input, m_bigEndian));
    }
  }

  const PMDXForm &xFormContainer = getXForm(polyXformId);
  std::shared_ptr<PMDLineSet> newShape(new PMDPolygon(points, closed, bboxTopLeft, bboxBotRight, xFormContainer, fillProps, strokeProps));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseEllipse(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  PMDFillProperties fillProps;
  PMDStrokeProperties strokeProps;

  skip(m_input, 2);
  fillProps.m_fillOverprint = readU8(m_input);
  skip(m_input, 1);
  fillProps.m_fillColor = readU8(m_input);

  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);

  skip(m_input, 14);
  uint32_t ellipseXformId = readU32(m_input, m_bigEndian);

  strokeProps.m_strokeType = readU8(m_input);
  skip(m_input, 2);
  strokeProps.m_strokeWidth = readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  fillProps.m_fillType = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeColor = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  strokeProps.m_strokeTint = readU8(m_input);

  skip(m_input, 0xb3);
  fillProps.m_fillTint = readU8(m_input);

  const PMDXForm &xFormContainer = getXForm(ellipseXformId);
  std::shared_ptr<PMDLineSet> newShape(new PMDEllipse(bboxTopLeft, bboxBotRight, xFormContainer, fillProps, strokeProps));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseBitmap(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID)
{
  librevenge::RVNGBinaryData bitmap;
  seekToRecord(m_input, container, recordIndex);

  skip(m_input, 6);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);
  skip(m_input, 14);
  uint32_t bboxXformId = readU32(m_input, m_bigEndian);

  skip(m_input, 16);
  uint16_t bitmapRecordSeqNum = readU16(m_input, m_bigEndian);

  const PMDXForm &xFormContainer = getXForm(bboxXformId);

  RecordIterator tiffIt = beginRecordsWithSeqNumber(bitmapRecordSeqNum);
  if (tiffIt == endRecords())
  {
    throw RecordNotFoundException(TIFF, bitmapRecordSeqNum);
  }

  for (; tiffIt != endRecords(); ++tiffIt)
  {
    const PMDRecordContainer &tiffContainer = *tiffIt;
    seekToRecord(m_input, tiffContainer, 0);
    const unsigned char *const tempBytes = readNBytes(m_input,tiffContainer.m_numRecords);
    bitmap.append(tempBytes,tiffContainer.m_numRecords);
  }

  tiffIt = beginRecordsWithSeqNumber(bitmapRecordSeqNum + 1);
  if (tiffIt == endRecords())
  {
    throw RecordNotFoundException(TIFF, bitmapRecordSeqNum);
  }
  for (; tiffIt != endRecords(); ++tiffIt)
  {
    const PMDRecordContainer &tiffSecondContainer = *tiffIt;
    seekToRecord(m_input, tiffSecondContainer, 0);
    const unsigned char *const tempBytes = readNBytes(m_input,tiffSecondContainer.m_numRecords);
    bitmap.append(tempBytes,tiffSecondContainer.m_numRecords);
  }


  std::shared_ptr<PMDLineSet> newShape(new PMDBitmap(bboxTopLeft, bboxBotRight, xFormContainer, bitmap));
  m_collector->addShapeToPage(pageID, newShape);

}

void PMDParser::parseShapes(uint16_t seqNum, unsigned pageID)
{
  for (RecordIterator it = beginRecordsWithSeqNumber(seqNum); it != endRecords(); ++it)
  {
    const PMDRecordContainer &container = *it;

    for (unsigned i = 0; i < container.m_numRecords; ++i)
    {
      seekToRecord(m_input, container, i);

      uint8_t shapeType = readU8(m_input);
      switch (shapeType)
      {
      case LINE_RECORD:
        parseLine(container, i, pageID);
        break;
      case RECTANGLE_RECORD:
        parseRectangle(container, i, pageID);
        break;
      case POLYGON_RECORD:
        parsePolygon(container, i, pageID);
        break;
      case ELLIPSE_RECORD:
        parseEllipse(container, i, pageID);
        break;
      case TEXT_RECORD:
        parseTextBox(container, i, pageID);
        break;
      case BITMAP_RECORD:
      case METAFILE_RECORD:
        parseBitmap(container, i, pageID);
        break;
      default:
        PMD_ERR_MSG("Encountered shape of unknown type.\n");
        continue;
      }
    }
  }
}

void PMDParser::parseFonts()
{
  RecordIterator it = beginRecordsOfType(FONTS);

  if (it != endRecords())
  {
    PMD_ERR_MSG("No Font Record Found.\n");
  }

  uint16_t fontIndex = 0;
  for (; it != endRecords(); ++it)
  {
    const PMDRecordContainer &container = *it;

    for (unsigned i = 0; i < container.m_numRecords; ++i)
    {
      seekToRecord(m_input, container, i);

      std::string fontName;

      uint8_t temp = readU8(m_input);

      while (temp)
      {
        fontName.push_back(temp);
        temp = readU8(m_input);
      }
      m_collector->addFont(PMDFont(fontIndex, fontName));
      fontIndex++;
    }
  }
}

void PMDParser::parseColors()
{
  RecordIterator it = beginRecordsOfType(COLORS);

  if (it != endRecords())
  {
    PMD_ERR_MSG("No Color Record Found.\n");
  }

  for (; it != endRecords(); ++it)
  {
    const PMDRecordContainer &container = *it;

    for (unsigned i = 0; i < container.m_numRecords; ++i)
    {
      seekToRecord(m_input, container, i);
      skip(m_input, 0x22);

      uint8_t colorModel = readU8(m_input);
      uint8_t red = 0;
      uint8_t blue = 0;
      uint8_t green = 0;

      skip(m_input, 3);
      if (colorModel == RGB)
      {
        red = readU8(m_input);
        green = readU8(m_input);
        blue = readU8(m_input);
      }
      else if (colorModel == CMYK || colorModel == HLS) // HLS is also stroed in CMYK format
      {
        uint16_t cyan = readU16(m_input, m_bigEndian);
        uint16_t magenta = readU16(m_input, m_bigEndian);
        uint16_t yellow = readU16(m_input, m_bigEndian);
        uint16_t black = readU16(m_input, m_bigEndian);

        uint16_t max = (std::numeric_limits<uint16_t>::max)();

        red = 255*(1 - std::min(1.0, (double)cyan/max + (double)black/max));
        green = 255*(1 - std::min(1.0, (double)magenta/max + (double)black/max));
        blue = 255*(1 - std::min(1.0, (double)yellow/max + (double)black/max));
      }

      m_collector->addColor(PMDColor(i, red, green, blue));
    }
  }
}

void PMDParser::parseXforms()
{
  RecordIterator it = beginRecordsOfType(XFORM);

  for (; it != endRecords(); ++it)
  {
    const PMDRecordContainer &xformContainer = *it;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {

      seekToRecord(m_input, xformContainer, i);

      uint32_t rotationDegree = readU32(m_input, m_bigEndian);
      uint32_t skewDegree = readU32(m_input, m_bigEndian);
      skip(m_input, 2);
      PMDShapePoint xformTopLeft = readPoint(m_input, m_bigEndian);
      PMDShapePoint xformBotRight = readPoint(m_input, m_bigEndian);
      PMDShapePoint rotatingPoint = readPoint(m_input, m_bigEndian);
      uint32_t xformId = readU32(m_input, m_bigEndian);

      m_xFormMap.insert(std::pair<uint32_t, PMDXForm>(xformId,PMDXForm(rotationDegree,skewDegree,xformTopLeft,xformBotRight,rotatingPoint,xformId)));
    }
  }
  m_xFormMap.insert(std::pair<uint32_t,PMDXForm>(0,PMDXForm(0,0,PMDShapePoint(0,0),PMDShapePoint(0,0),PMDShapePoint(0,0),0))); //Default XForm
}


void PMDParser::parsePages(const PMDRecordContainer &container)
{
  seekToRecord(m_input, container, 0);

  skip(m_input, 8);
  uint16_t pageWidth = readU16(m_input, m_bigEndian);
  (void) pageWidth;

  // if (pageWidth)
  // m_collector->setPageWidth(pageWidth);

  for (unsigned i = 0; i < container.m_numRecords; ++i)
  {
    seekToRecord(m_input, container, i);

    skip(m_input, 2);
    uint16_t shapesSeqNum = readU16(m_input, m_bigEndian);
    unsigned pageID = m_collector->addPage();
    parseShapes(shapesSeqNum, pageID);
  }
}

void PMDParser::parseHeader(uint32_t *tocOffset, uint16_t *tocLength)
{
  PMD_DEBUG_MSG(("[Header] Parsing header...\n"));
  seek(m_input, ENDIANNESS_MARKER_OFFSET);
  uint16_t endiannessMarker = readU16(m_input, false);
  if (endiannessMarker == ENDIANNESS_MARKER)
  {
    PMD_DEBUG_MSG(("[Header] File is little-endian.\n"));
    m_bigEndian = false;
  }
  else if (endiannessMarker == WARPED_ENDIANNESS_MARKER)
  {
    PMD_DEBUG_MSG(("[Header] File is big-endian.\n"));
    m_bigEndian = true;
  }
  else
  {
    throw PMDParseException("Endianness marker is corrupt in PMD header.");
  }
  try
  {
    seek(m_input, TABLE_OF_CONTENTS_LENGTH_OFFSET);
    *tocLength = readU16(m_input, m_bigEndian);
    PMD_DEBUG_MSG(("[Header] TOC length is %d\n", *tocLength));
  }
  catch (const PMDStreamException &)
  {
    throw PMDParseException("Can't find the table of contents length in the header.");
  }
  try
  {
    seek(m_input, TABLE_OF_CONTENTS_OFFSET_OFFSET);
    *tocOffset = readU32(m_input, m_bigEndian);
    PMD_DEBUG_MSG(("[Header] TOC offset is 0x%x\n", *tocOffset));
  }
  catch (const PMDStreamException &)
  {
    throw PMDParseException("Can't find the table of contents offset in the header.");
  }
}

void PMDParser::readNextRecordFromTableOfContents(ToCState &state, const bool subRecord, const uint16_t subRecordType)
{
  skip(m_input, 1);
  uint16_t recType = readU8(m_input);
  uint16_t numRecs = readU16(m_input, m_bigEndian);
  uint32_t offset = readU32(m_input, m_bigEndian);
  skip(m_input, 2);

  uint16_t subType = 0;

  if (!subRecord && (recType != 0 || numRecs == 0))
  {
    skip(m_input, 1);
    subType = readU8(m_input);
    if (subType == 0)
    {
      PMD_DEBUG_MSG(("[TOC] invalid subrecord type\n"));
    }
    skip(m_input, 4);
  }

  if (recType == 0 && numRecs == 0)
  {
    // empty record
    ++state.seqNum;
  }
  else if (!subRecord && recType == 1)
  {
    readTableOfContents(state, offset, numRecs, true, subType);
    ++state.seqNum;
  }
  else if (!subRecord && recType == 0)
  {
    readTableOfContents(state, offset, numRecs, false);
  }
  else
  {
    if (numRecs != 0 && offset != 0)
    {
      if (subRecord && recType != subRecordType)
      {
        PMD_DEBUG_MSG(("[TOC] subrecord type mismatch: expected %hu, got %hu.\n", subRecordType, recType));
        if (subRecordType != 0) // can only happen in a broken file -- better ignore
          recType = subRecordType;
      }
      m_recordsInOrder.push_back(PMDRecordContainer(recType, offset, state.seqNum, numRecs));
      m_records[recType].push_back((unsigned)(m_recordsInOrder.size() - 1));
    }
    if (!subRecord)
      ++state.seqNum;
  }
}

void PMDParser::readTableOfContents(ToCState &state, const uint32_t offset, unsigned records, const bool subRecords, const uint16_t subRecordType)
{
  if (state.parsedBlocks.end() != state.parsedBlocks.find(m_input->tell()))
  {
    PMD_DEBUG_MSG(("[TOC] ToC block at offset %ld has already been read. The file is probably broken. Skipping...\n", m_input->tell()));
    return;
  }

  state.parsedBlocks.insert(m_input->tell());

  if (records == 0 || offset == 0)
  {
    PMD_DEBUG_MSG(("[TOC] no records to read\n"));
    return;
  }

  const long orig = m_input->tell();

  PMD_DEBUG_MSG(("[TOC] reading %sblock at offset 0x%x\n", subRecords ? "subrecord " : "", offset));
  seek(m_input, offset);
  PMD_DEBUG_MSG(("[TOC] records to read: %d\n", records));
  const size_t minRecordSize = subRecords ? 10 : 16;
  const size_t maxPossibleRecords = (m_length-offset)/minRecordSize;
  for (unsigned i = 0; i < std::min<size_t>(records, maxPossibleRecords); ++i)
    readNextRecordFromTableOfContents(state, subRecords, subRecordType);

  seek(m_input, orig);
}

void PMDParser::parseTableOfContents(uint32_t offset, uint16_t length) try
{
  ToCState state;
  readTableOfContents(state, offset, length, false);
}
catch (...)
{
  PMD_ERR_MSG("Error reading the table of contents! Some or all records will be missing.\n");
}

void PMDParser::parse()
{
  uint32_t tocOffset;
  uint16_t tocLength;
  parseHeader(&tocOffset, &tocLength);
  parseTableOfContents(tocOffset, tocLength);
  parseFonts();
  parseColors();
  parseXforms();

  auto i = m_records.find(GLOBAL_INFO);
  if (i != m_records.end()
      && !(i->second.empty()))
  {
    parseGlobalInfo(m_recordsInOrder[i->second[0]]);
  }
  else
  {
    throw RecordNotFoundException(GLOBAL_INFO);
  }

  i = m_records.find(PAGE);
  if (i != m_records.end()
      && !(i->second.empty()))
  {
    parsePages(m_recordsInOrder[i->second[0]]);
  }
  else
  {
    throw RecordNotFoundException(PAGE);
  }
}

PMDParser::RecordIterator PMDParser::beginRecordsWithSeqNumber(const uint16_t seqNum) const
{
  return RecordIterator(m_recordsInOrder, seqNum);
}

PMDParser::RecordIterator PMDParser::beginRecordsOfType(const uint16_t recType) const
{
  return RecordIterator(m_recordsInOrder, m_records, recType);
}

PMDParser::RecordIterator PMDParser::endRecords() const
{
  return RecordIterator(m_recordsInOrder);
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

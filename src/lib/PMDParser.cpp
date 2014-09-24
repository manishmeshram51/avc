/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <string>
#include <cassert>
#include <vector>
#include <limits>
#include <librevenge/librevenge.h>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include "PMDRecord.h"
#include "PMDParser.h"
#include "PMDCollector.h"
#include "PMDExceptions.h"
#include "constants.h"
#include "offsets.h"
#include "libpagemaker_utils.h"
#include "geometry.h"
#include "PMDTypes.h"

namespace libpagemaker
{
PMDParser::PMDParser(librevenge::RVNGInputStream *input, PMDCollector *collector)
  : m_input(input), m_length(getLength(input)), m_collector(collector),
    m_records(), m_bigEndian(false), m_recordsInOrder(), m_xFormMap()
{
}

std::vector<PMDRecordContainer> PMDParser::getRecordsBySeqNum(const uint16_t seqNum)
{
  std::vector<PMDRecordContainer> tempContainer;


  for (unsigned j=0; j<m_recordsInOrder.size(); ++j)
  {
    const PMDRecordContainer *ptrToContainer = &(m_recordsInOrder[j]);
    const PMDRecordContainer &container = *ptrToContainer;

    if (container.m_seqNum == seqNum)
    {
      tempContainer.push_back(container);
    }
  }
  return tempContainer;
}

std::vector<PMDRecordContainer> PMDParser::getRecordsByRecType(const uint16_t recType)
{
  std::vector<PMDRecordContainer> tempContainer;

  for (unsigned j=0; j<m_recordsInOrder.size(); ++j)
  {
    const PMDRecordContainer *ptrToContainer = &(m_recordsInOrder[j]);
    const PMDRecordContainer &container = *ptrToContainer;

    if (container.m_recordType == recType)
    {
      tempContainer.push_back(container);
    }
  }
  return tempContainer;
}

const PMDRecordContainer &PMDParser::getSingleRecordBySeqNum(const uint16_t seqNum) const
{
  for (unsigned j=0; j<m_recordsInOrder.size(); ++j)
  {
    const PMDRecordContainer *ptrToContainer = &(m_recordsInOrder[j]);
    const PMDRecordContainer &container = *ptrToContainer;

    if (container.m_seqNum == seqNum)
    {
      return container;
    }
  }
  PMD_ERR_MSG("No record with the given sequence number.\n");
  throw RecordNotFoundException(seqNum);
}

const PMDXForm &PMDParser::getXForm(const uint32_t xFormId) const
{
  if (xFormId != (std::numeric_limits<uint32_t>::max)() && xFormId != 0)
  {
    std::map<uint32_t, PMDXForm>::const_iterator it = m_xFormMap.find(xFormId);

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
  return PMDShapePoint(x, y);
}

void PMDParser::parseGlobalInfo(const PMDRecordContainer &container)
{
  seekToRecord(m_input, container, 0);

  skip(m_input, 0x3a);
  uint16_t leftPageRightBound = readU16(m_input, m_bigEndian);
  uint16_t pageHeight = readU16(m_input, m_bigEndian);

  bool doubleSided = (leftPageRightBound == 0);
  m_collector->setDoubleSided(doubleSided);

  if (!doubleSided)
    m_collector->setPageWidth(leftPageRightBound);

  m_collector->setPageHeight(pageHeight);
}

void PMDParser::parseLine(const PMDRecordContainer &container, unsigned recordIndex,
                          unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);
  skip(m_input, 4);
  uint8_t strokeColor = readU8(m_input);
  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);
  bool mirrored = false;
  skip(m_input, 0x18);
  uint16_t temp = readU16(m_input, m_bigEndian);

  if (temp != 257 && temp != 0)
    mirrored = true;

  skip(m_input, 6);
  uint8_t strokeType = readU8(m_input);
  skip(m_input, 1);
  uint16_t strokeWidth =readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  uint8_t strokeTint = readU8(m_input);
  skip(m_input, 6);
  uint8_t strokeOverprint = readU8(m_input);

  PMDStrokeProperties strokeProps(strokeType,strokeWidth,strokeColor,strokeOverprint,strokeTint);

  boost::shared_ptr<PMDLineSet> newShape(new PMDLine(bboxTopLeft, bboxBotRight, mirrored, strokeProps));
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

  std::vector<PMDRecordContainer> tempContainer = getRecordsByRecType(TEXT_BLOCK);
  if (tempContainer.empty())
  {
    PMD_ERR_MSG("No Text Block Record Found.\n");
  }

  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer &textBlockContainer = tempContainer[j];

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

  tempContainer = getRecordsBySeqNum(textBoxText);
  if (tempContainer.empty())
  {
    PMD_ERR_MSG("No Text Found.\n");
  }

  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer &textContainer = tempContainer[j];
    seekToRecord(m_input, textContainer, 0);
    for (unsigned i = 0; i < textContainer.m_numRecords; ++i)
    {
      text.push_back(readU8(m_input));
    }
  }

  std::vector<PMDCharProperties> charProps;
  const PMDRecordContainer &charsContainer = getSingleRecordBySeqNum(textBoxChars);
  for (unsigned i = 0; i < charsContainer.m_numRecords; ++i)
  {
    seekToRecord(m_input, charsContainer, i);

    uint16_t length = readU16(m_input, m_bigEndian);
    uint16_t fontFace = readU16(m_input, m_bigEndian);
    uint16_t fontSize = readU16(m_input, m_bigEndian);
    skip(m_input, 2);
    uint8_t fontColor = readU8(m_input);
    skip(m_input, 1);
    uint8_t boldItalicUnderline = readU8(m_input);
    uint8_t superSubscript = readU8(m_input);
    skip(m_input, 4);
    int16_t kerning = readS16(m_input, m_bigEndian);
    skip(m_input, 2);
    uint16_t superSubSize = readU16(m_input, m_bigEndian);
    uint16_t subPos = readU16(m_input, m_bigEndian);
    uint16_t superPos = readU16(m_input, m_bigEndian);
    skip(m_input, 2);
    uint8_t tint = readU8(m_input);

    charProps.push_back(PMDCharProperties(length,fontFace,fontSize,fontColor,boldItalicUnderline,superSubscript,kerning,superSubSize,superPos,subPos,tint));
  }

  std::vector<PMDParaProperties> paraProps;
  const PMDRecordContainer &paraContainer = getSingleRecordBySeqNum(textBoxPara);
  for (unsigned i = 0; i < paraContainer.m_numRecords; ++i)
  {
    seekToRecord(m_input, paraContainer, i);

    uint16_t length = readU16(m_input, m_bigEndian);
    skip(m_input, 1);
    uint8_t align = readU8(m_input);
    skip(m_input, 6);
    uint16_t leftIndent = readU16(m_input, m_bigEndian);
    uint16_t firstIndent = readU16(m_input, m_bigEndian);
    uint16_t rightIndent = readU16(m_input, m_bigEndian);
    uint16_t beforeIndent = readU16(m_input, m_bigEndian); // Above Para Spacing
    uint16_t afterIndent = readU16(m_input, m_bigEndian); // Below Para Spacing

    paraProps.push_back(PMDParaProperties(length,align,leftIndent,firstIndent,rightIndent,beforeIndent,afterIndent));
  }

  boost::shared_ptr<PMDLineSet> newShape(new PMDTextBox(bboxTopLeft, bboxBotRight, xFormContainer, text, charProps, paraProps));
  m_collector->addShapeToPage(pageID, newShape);

}

void PMDParser::parseRectangle(const PMDRecordContainer &container, unsigned recordIndex,
                               unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  skip(m_input, 2);
  uint8_t fillOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t fillColor = readU8(m_input);
  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);
  skip(m_input, 14);
  uint32_t rectXformId = readU32(m_input, m_bigEndian);

  uint8_t strokeType = readU8(m_input);
  skip(m_input, 2);
  uint16_t strokeWidth = readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  uint8_t fillType = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeColor = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeTint = readU8(m_input);

  skip(m_input, 0xb3);
  uint8_t fillTint = readU8(m_input);

  PMDFillProperties fillProps(fillType,fillColor,fillOverprint,fillTint);
  PMDStrokeProperties strokeProps(strokeType,strokeWidth,strokeColor,strokeOverprint,strokeTint);

  const PMDXForm &xFormContainer = getXForm(rectXformId);
  boost::shared_ptr<PMDLineSet> newShape(new PMDRectangle(bboxTopLeft, bboxBotRight, xFormContainer, fillProps, strokeProps));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parsePolygon(const PMDRecordContainer &container, unsigned recordIndex,
                             unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  skip(m_input, 2);
  uint8_t fillOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t fillColor = readU8(m_input);

  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);

  skip(m_input, 14);
  uint32_t polyXformId = readU32(m_input, m_bigEndian);

  uint8_t strokeType = readU8(m_input);
  skip(m_input, 2);
  uint16_t strokeWidth = readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  uint8_t fillType = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeColor = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeTint = readU8(m_input);

  skip(m_input, 1);
  uint16_t lineSetSeqNum = readU16(m_input, m_bigEndian);
  skip(m_input, 8);
  uint8_t closedMarker = readU8(m_input);
  skip(m_input, 0xa7);
  uint8_t fillTint = readU8(m_input);

  PMDFillProperties fillProps(fillType,fillColor,fillOverprint,fillTint);
  PMDStrokeProperties strokeProps(strokeType,strokeWidth,strokeColor,strokeOverprint,strokeTint);

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

  const PMDRecordContainer &lineSetContainer = getSingleRecordBySeqNum(lineSetSeqNum);
  std::vector<PMDShapePoint> points;
  for (unsigned i = 0; i < lineSetContainer.m_numRecords; ++i)
  {
    seekToRecord(m_input, lineSetContainer, i);
    points.push_back(readPoint(m_input, m_bigEndian));
  }

  const PMDXForm &xFormContainer = getXForm(polyXformId);
  boost::shared_ptr<PMDLineSet> newShape(new PMDPolygon(points, closed, bboxTopLeft, bboxBotRight, xFormContainer, fillProps, strokeProps));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseEllipse(const PMDRecordContainer &container, unsigned recordIndex, unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  skip(m_input, 2);
  uint8_t fillOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t fillColor = readU8(m_input);

  skip(m_input, 1);
  PMDShapePoint bboxTopLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint bboxBotRight = readPoint(m_input, m_bigEndian);

  skip(m_input, 14);
  uint32_t ellipseXformId = readU32(m_input, m_bigEndian);

  uint8_t strokeType = readU8(m_input);
  skip(m_input, 2);
  uint16_t strokeWidth = readU16(m_input, m_bigEndian);
  skip(m_input, 1);
  uint8_t fillType = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeColor = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeTint = readU8(m_input);

  skip(m_input, 0xb3);
  uint8_t fillTint = readU8(m_input);

  PMDFillProperties fillProps(fillType,fillColor,fillOverprint,fillTint);
  PMDStrokeProperties strokeProps(strokeType,strokeWidth,strokeColor,strokeOverprint,strokeTint);

  const PMDXForm &xFormContainer = getXForm(ellipseXformId);
  boost::shared_ptr<PMDLineSet> newShape(new PMDEllipse(bboxTopLeft, bboxBotRight, xFormContainer, fillProps, strokeProps));
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
  uint16_t bitmapRecordSeqNum = readU16(m_input);

  std::vector<PMDRecordContainer> tempContainer;

  const PMDXForm &xFormContainer = getXForm(bboxXformId);

  tempContainer = getRecordsBySeqNum(bitmapRecordSeqNum);
  if (tempContainer.empty())
  {
    throw RecordNotFoundException(TIFF, bitmapRecordSeqNum);
  }

  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer *ptrToTiffContainer = &(tempContainer[j]);
    const PMDRecordContainer &tiffContainer = *ptrToTiffContainer;
    seekToRecord(m_input, tiffContainer, 0);
    const unsigned char *const tempBytes = readNBytes(m_input,tiffContainer.m_numRecords);
    bitmap.append(tempBytes,tiffContainer.m_numRecords);
  }

  tempContainer = getRecordsBySeqNum(bitmapRecordSeqNum + 1);
  if (tempContainer.empty())
  {
    throw RecordNotFoundException(TIFF, bitmapRecordSeqNum);
  }
  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer *ptrToTiffSecondContainer = &(tempContainer[j]);
    const PMDRecordContainer &tiffSecondContainer = *ptrToTiffSecondContainer;
    seekToRecord(m_input, tiffSecondContainer, 0);
    const unsigned char *const tempBytes = readNBytes(m_input,tiffSecondContainer.m_numRecords);
    bitmap.append(tempBytes,tiffSecondContainer.m_numRecords);
  }


  boost::shared_ptr<PMDLineSet> newShape(new PMDBitmap(bboxTopLeft, bboxBotRight, xFormContainer, bitmap));
  m_collector->addShapeToPage(pageID, newShape);

}

void PMDParser::parseShapes(uint16_t seqNum, unsigned pageID)
{
  std::vector<PMDRecordContainer> tempContainer = getRecordsBySeqNum(seqNum);

  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer *ptrToContainer = &(tempContainer[j]);
    const PMDRecordContainer &container = *ptrToContainer;

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
  std::vector<PMDRecordContainer> tempContainer = getRecordsByRecType(FONTS);

  if (tempContainer.empty())
  {
    PMD_ERR_MSG("No Font Record Found.\n");
  }

  uint16_t fontIndex = 0;
  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer &container = tempContainer[j];

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

  std::vector<PMDRecordContainer> tempContainer = getRecordsByRecType(COLORS);

  if (tempContainer.empty())
  {
    PMD_ERR_MSG("No Color Record Found.\n");
  }

  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer &container = tempContainer[j];

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

  std::vector<PMDRecordContainer> tempContainer = getRecordsByRecType(XFORM);

  for (unsigned j=0; j<tempContainer.size(); ++j)
  {
    const PMDRecordContainer &xformContainer = tempContainer[j];

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

  if (pageWidth)
    m_collector->setPageWidth(pageWidth);

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
    *tocLength = readU32(m_input, m_bigEndian);
    PMD_DEBUG_MSG(("[Header] TOC length is %d\n", *tocLength));
  }
  catch (PMDStreamException)
  {
    throw PMDParseException("Can't find the table of contents length in the header.");
  }
  try
  {
    seek(m_input, TABLE_OF_CONTENTS_OFFSET_OFFSET);
    *tocOffset = readU32(m_input, m_bigEndian);
    PMD_DEBUG_MSG(("[Header] TOC offset is 0x%x\n", *tocOffset));
  }
  catch (PMDStreamException)
  {
    throw PMDParseException("Can't find the table of contents offset in the header.");
  }
}

unsigned PMDParser::readNextRecordFromTableOfContents(std::set<unsigned long> &tocOffsets, unsigned &seqNum)
{
  const size_t minRecordSize = 10;

  if (tocOffsets.end() != tocOffsets.find(m_input->tell()))
  {
    PMD_DEBUG_MSG(("[TOC] ToC entry at offset %ld has already been read. The file is probably broken. Skipping...\n", m_input->tell()));
    return 0;
  }

  tocOffsets.insert(m_input->tell());

  uint16_t recType = readU16(m_input, m_bigEndian);
  uint16_t numRecs = readU16(m_input, m_bigEndian);
  uint32_t offset = readU32(m_input, m_bigEndian);

  skip(m_input, 2);

  if (recType == 0 && numRecs > 0)
  {
    uint32_t temp = m_input->tell();
    seek(m_input,offset);
    const size_t maxPossibleRecords = (m_length-offset)/minRecordSize;
    numRecs = std::min<size_t>(numRecs, maxPossibleRecords);
    for (unsigned i = 0; i < numRecs; ++i)
    {
      unsigned numRead = readNextRecordFromTableOfContents(tocOffsets, seqNum);
      (void)numRead;
      PMD_DEBUG_MSG(("[TOC] Learned about %d TMD records from ToC entry %d.\n",
                     numRead, i));
    }
    seek(m_input,temp);
  }
  else
  {
    if (readU8(m_input) != 0x01)
    {
      uint32_t temp = m_input->tell();
      seek(m_input,offset);
      const size_t maxPossibleRecords = (m_length-offset)/minRecordSize;
      numRecs = std::min<size_t>(numRecs, maxPossibleRecords);
      for (uint32_t i = 0; i<numRecs; ++i)
      {
        uint16_t subRecType = readU16(m_input, m_bigEndian);
        uint16_t subNumRecs = readU16(m_input, m_bigEndian);
        uint32_t subOffset = readU32(m_input, m_bigEndian);
        skip(m_input, 2);
        m_recordsInOrder.push_back(PMDRecordContainer(subRecType, subOffset, seqNum, subNumRecs));
        m_records[subRecType].push_back((unsigned)(m_recordsInOrder.size() - 1));
      }
      seek(m_input,temp);
    }
    else
    {
      m_recordsInOrder.push_back(PMDRecordContainer(recType, offset, seqNum, numRecs));
      m_records[recType].push_back((unsigned)(m_recordsInOrder.size() - 1));

    }
    ++seqNum;
    skip(m_input, 5);
  }

  return numRecs;
}

void PMDParser::parseTableOfContents(uint32_t offset, uint16_t length) try
{
  PMD_DEBUG_MSG(("[TOC] Seeking to offset 0x%x to read ToC\n", offset));
  seek(m_input, offset);
  PMD_DEBUG_MSG(("[TOC] entries to read: %d\n", length));
  unsigned j=0;
  std::set<unsigned long> tocOffsets;
  for (unsigned i = 0; i < length; ++i)
  {
    unsigned numRead = readNextRecordFromTableOfContents(tocOffsets, j);
    (void)numRead;
    PMD_DEBUG_MSG(("[TOC] Learned about %d TMD records from ToC entry %d.\n",
                   numRead, i));
  }
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

  typedef std::map<uint16_t, std::vector<unsigned> >::iterator RecIter;

  RecIter i = m_records.find(GLOBAL_INFO);
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
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

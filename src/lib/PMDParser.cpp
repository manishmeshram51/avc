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
#include <math.h>
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
  : m_input(input), m_collector(collector), m_records(), m_bigEndian(false),
    m_recordsInOrder()
{ }

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

  m_collector->setPageHeight(pageHeight);
}

void PMDParser::parseLine(const PMDRecordContainer &container, unsigned recordIndex,
                          unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);
  skip(m_input, 4);
  uint8_t strokeColor = readU8(m_input);
  skip(m_input, 1);
  PMDShapePoint topLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint botRight = readPoint(m_input, m_bigEndian);
  bool mirrored = false;
  skip(m_input, 0x18);
  uint16_t temp = readU16(m_input, m_bigEndian);

  if (temp != 257 && temp != 0)
    mirrored = true;

  skip(m_input, 6);
  uint8_t strokeType = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeWidth =readU8(m_input);
  skip(m_input, 2);
  uint8_t strokeTint = readU8(m_input);
  skip(m_input, 6);
  uint8_t strokeOverprint = readU8(m_input);

  boost::shared_ptr<PMDLineSet> newShape(new PMDLine(topLeft, botRight, mirrored, strokeType, strokeWidth, strokeColor, strokeOverprint, strokeTint));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseTextBox(const PMDRecordContainer &container, unsigned recordIndex,
                             unsigned pageID)
{
  seekToRecord(m_input, container, recordIndex);

  skip(m_input, 6);
  PMDShapePoint topLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint botRight = readPoint(m_input, m_bigEndian);
  uint32_t textBoxRotationDegree = 0;
  uint32_t textBoxSkewDegree = 0;
  PMDShapePoint xformTopLeft = PMDShapePoint(0,0);
  PMDShapePoint xformBotRight = PMDShapePoint(0,0);
  PMDShapePoint rotatingPoint = PMDShapePoint(0, 0);

  uint16_t textBoxTextPropsOne = 0;
  uint16_t textBoxTextPropsTwo = 0;
  uint16_t textBoxTextStyle = 0;
  uint16_t textBoxText = 0;
  uint16_t textBoxChars = 0;
  uint16_t textBoxPara = 0;

  skip(m_input, 0xe);
  uint32_t textBoxXformId = readU32(m_input, m_bigEndian);
  uint32_t textBoxTextBlockId = readU32(m_input, m_bigEndian);

  if (textBoxXformId != (std::numeric_limits<uint32_t>::max)())
  {
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, xformContainer, i);

      skip(m_input, 0x16);
      uint32_t xformId = readU32(m_input, m_bigEndian);
      if (xformId == textBoxXformId)
      {
        seekToRecord(m_input, xformContainer, i); // return to the beginning

        textBoxRotationDegree = readU32(m_input, m_bigEndian);
        textBoxSkewDegree = readU32(m_input, m_bigEndian);
        skip(m_input, 4);
        xformTopLeft = readPoint(m_input, m_bigEndian);
        xformBotRight = readPoint(m_input, m_bigEndian);
        rotatingPoint = readPoint(m_input, m_bigEndian);
        break;
      }
    }
  }

  int32_t temp = (int32_t)textBoxRotationDegree;
  double rotationRadian = -1 * (double)temp/1000 * (M_PI/180);
  temp = (int32_t)textBoxSkewDegree;
  double skewRadian = -1 * (double)temp/1000 * (M_PI/180);

  const PMDRecordContainer *ptrToTextBlockContainer = &(m_recordsInOrder[TEXT_BLOCK_OFFSET]);
  const PMDRecordContainer &textBlockContainer = *ptrToTextBlockContainer;

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
  const PMDRecordContainer *ptrToTextContainer =
    (textBoxText < m_recordsInOrder.size()) ? &(m_recordsInOrder[textBoxText]) : NULL;
  if (!ptrToTextContainer)
  {
    PMD_ERR_MSG("No text with the given sequence number.\n");
    return;
  }
  std::string text = "";
  const PMDRecordContainer &textContainer = *(ptrToTextContainer);

  seekToRecord(m_input, textContainer, 0);
  for (unsigned i = 0; i < textContainer.m_numRecords; ++i)
  {
    text.push_back(readU8(m_input));
  }

  const PMDRecordContainer *ptrToCharsContainer =
    (textBoxChars < m_recordsInOrder.size()) ? &(m_recordsInOrder[textBoxChars]) : NULL;
  if (!ptrToCharsContainer)
  {
    PMD_ERR_MSG("No chars with the given sequence number.\n");
    return;
  }

  std::vector<PMDCharProperties> charProps;
  const PMDRecordContainer &charsContainer = *(ptrToCharsContainer);
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

    charProps.push_back(PMDCharProperties(length,fontFace,fontSize,fontColor,boldItalicUnderline,superSubscript,kerning,superSubSize,superPos,subPos));
  }

  const PMDRecordContainer *ptrToParaContainer =
    (textBoxPara < m_recordsInOrder.size()) ? &(m_recordsInOrder[textBoxPara]) : NULL;
  if (!ptrToParaContainer)
  {
    PMD_ERR_MSG("No para with the given sequence number.\n");
    return;
  }

  std::vector<PMDParaProperties> paraProps;
  const PMDRecordContainer &paraContainer = *(ptrToParaContainer);
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


  boost::shared_ptr<PMDLineSet> newShape(new PMDTextBox(topLeft, botRight, rotationRadian, skewRadian, rotatingPoint, xformTopLeft, xformBotRight, text, charProps, paraProps));
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
  PMDShapePoint topLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint botRight = readPoint(m_input, m_bigEndian);
  uint32_t rectRotationDegree = 0;
  uint32_t rectSkewDegree = 0;
  PMDShapePoint xformTopLeft = PMDShapePoint(0,0);
  PMDShapePoint xformBotRight = PMDShapePoint(0,0);
  PMDShapePoint rotatingPoint = PMDShapePoint(0, 0);
  skip(m_input, 14);
  uint32_t rectXformId = readU32(m_input, m_bigEndian);

  uint8_t strokeType = readU8(m_input);
  skip(m_input, 2);
  uint8_t strokeWidth = readU8(m_input);
  skip(m_input, 2);
  uint8_t fillType = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeColor = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeTint = readU8(m_input);

  skip(m_input, 0xb3);
  uint8_t fillTint = readU8(m_input);

  if (rectXformId != (std::numeric_limits<uint32_t>::max)())
  {
    PMD_DEBUG_MSG(("Rectangle contains rotation\n"));
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, xformContainer, i);

      skip(m_input, 0x16);
      uint32_t xformId = readU32(m_input, m_bigEndian);
      if (xformId == rectXformId)
      {
        seekToRecord(m_input, xformContainer, i); // return to the beginning

        rectRotationDegree = readU32(m_input, m_bigEndian);
        rectSkewDegree = readU32(m_input, m_bigEndian);
        skip(m_input, 4);
        xformTopLeft = readPoint(m_input, m_bigEndian);
        xformBotRight = readPoint(m_input, m_bigEndian);
        rotatingPoint = readPoint(m_input, m_bigEndian);
        break;
      }
    }
  }
  int32_t temp = (int32_t)rectRotationDegree;
  double rotationRadian = -1 * (double)temp/1000 * (M_PI/180);
  temp = (int32_t)rectSkewDegree;
  double skewRadian = -1 * (double)temp/1000 * (M_PI/180);
  boost::shared_ptr<PMDLineSet> newShape(new PMDRectangle(topLeft, botRight, rotationRadian, skewRadian, rotatingPoint, xformTopLeft, xformBotRight, fillType, fillColor, fillOverprint, fillTint,  strokeType, strokeWidth, strokeColor, strokeOverprint, strokeTint));
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

  skip(m_input, 5);
  PMDShapePoint topLeft = readPoint(m_input, m_bigEndian);
  PMDShapePoint botRight = readPoint(m_input, m_bigEndian);

  skip(m_input, 14);
  uint32_t polyXformId = readU32(m_input, m_bigEndian);

  uint8_t strokeType = readU8(m_input);
  skip(m_input, 2);
  uint8_t strokeWidth = readU8(m_input);
  skip(m_input, 2);
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

  uint32_t polySkewDegree = 0;
  PMDShapePoint xformTopLeft = PMDShapePoint(0,0);
  PMDShapePoint xformBotRight = PMDShapePoint(0,0);

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

  const PMDRecordContainer *ptrToLineSetContainer =
    (lineSetSeqNum < m_recordsInOrder.size()) ? &(m_recordsInOrder[lineSetSeqNum]) : NULL;
  if (!ptrToLineSetContainer)
  {
    PMD_ERR_MSG("No line set with the given sequence number.\n");
    return;
  }
  const PMDRecordContainer &lineSetContainer = *(ptrToLineSetContainer);
  std::vector<PMDShapePoint> points;
  for (unsigned i = 0; i < lineSetContainer.m_numRecords; ++i)
  {
    seekToRecord(m_input, lineSetContainer, i);
    points.push_back(readPoint(m_input, m_bigEndian));
  }

  uint32_t polyRotationDegree = 0;

  if (polyXformId != (std::numeric_limits<uint32_t>::max)())
  {
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, xformContainer, i);

      skip(m_input, 0x16);
      uint32_t xformId = readU32(m_input, m_bigEndian);
      if (xformId == polyXformId)
      {
        seekToRecord(m_input, xformContainer, i); // return to the beginning

        polyRotationDegree = readU32(m_input, m_bigEndian);
        polySkewDegree = readU32(m_input, m_bigEndian);
        skip(m_input, 4);
        xformTopLeft = readPoint(m_input, m_bigEndian);
        xformBotRight = readPoint(m_input, m_bigEndian);
        break;
      }
    }
  }
  int32_t temp = (int32_t)polyRotationDegree;
  double rotationRadian = -1 * (double)temp/1000 * (M_PI/180);
  temp = (int32_t)polySkewDegree;
  double skewRadian = -1 * (double)temp/1000 * (M_PI/180);

  boost::shared_ptr<PMDLineSet> newShape(new PMDPolygon(points, closed, rotationRadian, skewRadian, topLeft, botRight, xformTopLeft, xformBotRight, fillType, fillColor, fillOverprint, fillTint, strokeType, strokeWidth, strokeColor, strokeOverprint, strokeTint));
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

  uint32_t ellipseRotationDegree = 0;
  uint32_t ellipseSkewDegree = 0;
  PMDShapePoint xformTopLeft = PMDShapePoint(0,0);
  PMDShapePoint xformBotRight = PMDShapePoint(0,0);

  skip(m_input, 14);
  uint32_t ellipseXformId = readU32(m_input, m_bigEndian);

  uint8_t strokeType = readU8(m_input);
  skip(m_input, 2);
  uint8_t strokeWidth = readU8(m_input);
  skip(m_input, 2);
  uint8_t fillType = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeColor = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeOverprint = readU8(m_input);
  skip(m_input, 1);
  uint8_t strokeTint = readU8(m_input);

  skip(m_input, 0xb3);
  uint8_t fillTint = readU8(m_input);

  if (ellipseXformId != (std::numeric_limits<uint32_t>::max)())
  {
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      seekToRecord(m_input, xformContainer, i);

      skip(m_input, 0x16);
      uint32_t xformId = readU32(m_input, m_bigEndian);
      if (xformId == ellipseXformId)
      {
        seekToRecord(m_input, xformContainer, i); // return to the beginning

        ellipseRotationDegree = readU32(m_input, m_bigEndian);
        ellipseSkewDegree = readU32(m_input, m_bigEndian);
        skip(m_input, 4);
        xformTopLeft = readPoint(m_input, m_bigEndian);
        xformBotRight = readPoint(m_input, m_bigEndian);
        break;
      }
    }
  }
  int32_t temp = (int32_t)ellipseRotationDegree;
  double rotationRadian = -1 * (double)temp/1000 *(M_PI/180);
  temp = (int32_t)ellipseSkewDegree;
  double skewRadian = -1 * (double)temp/1000 * (M_PI/180);

  boost::shared_ptr<PMDLineSet> newShape(new PMDEllipse(bboxTopLeft, bboxBotRight, rotationRadian, skewRadian, xformTopLeft, xformBotRight, fillType, fillColor, fillOverprint, fillTint, strokeType, strokeWidth, strokeColor, strokeOverprint, strokeTint));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseShapes(uint16_t seqNum, unsigned pageID)
{
  const PMDRecordContainer *ptrToContainer =
    (seqNum < m_recordsInOrder.size()) ? &(m_recordsInOrder[seqNum]) : NULL;
  if (!ptrToContainer)
  {
    throw RecordNotFoundException(SHAPE, seqNum);
  }
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
    default:
      PMD_ERR_MSG("Encountered shape of unknown type.\n");
      continue;
    }
  }
}

void PMDParser::parseFonts(const PMDRecordContainer &container)
{
  uint16_t fontIndex = 0;

  for (unsigned i = 0; i < container.m_numRecords; ++i)
  {
    seekToRecord(m_input, container, i);

    uint16_t recType = readU16(m_input, m_bigEndian);
    uint16_t numRecs = readU16(m_input, m_bigEndian);
    uint16_t offset = readU16(m_input, m_bigEndian);

    PMDRecordContainer subContainer(recType, offset, 0, numRecs);

    for (unsigned k = 0; k < subContainer.m_numRecords; ++k)
    {
      std::string fontName;

      seekToRecord(m_input, subContainer, k);
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

void PMDParser::parseColors(const PMDRecordContainer &container)
{
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
    else if (colorModel == CMYK)
    {
      uint16_t cyan = readU16(m_input, m_bigEndian);
      uint16_t magenta = readU16(m_input, m_bigEndian);
      uint16_t yellow = readU16(m_input, m_bigEndian);
      uint16_t black = readU16(m_input, m_bigEndian);

      uint16_t max = (std::numeric_limits<uint16_t>::max)();

      red = 255* round((1 - std::min(1.0, (double)cyan/max + (double)black/max)));
      green = 255*round((1 - std::min(1.0, (double)magenta/max + (double)black/max)));
      blue = 255*round((1 - std::min(1.0, (double)yellow/max + (double)black/max)));
    }
    else if (colorModel == HLS)
    { }

    m_collector->addColor(PMDColor(i, red, green, blue));
  }
}

void PMDParser::parsePages(const PMDRecordContainer &container)
{
  seekToRecord(m_input, container, 0);

  skip(m_input, 8);
  uint16_t pageWidth = readU16(m_input, m_bigEndian);
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
    *tocOffset = readU16(m_input, m_bigEndian);
    PMD_DEBUG_MSG(("[Header] TOC offset is 0x%x\n", *tocOffset));
  }
  catch (PMDStreamException)
  {
    throw PMDParseException("Can't find the table of contents offset in the header.");
  }
}

unsigned PMDParser::readNextRecordFromTableOfContents(unsigned seqNum)
{
  uint16_t recType = readU16(m_input, m_bigEndian);
  uint16_t numRecs = readU16(m_input, m_bigEndian);
  uint32_t offset = readU32(m_input, m_bigEndian);

  m_input->seek(8, librevenge::RVNG_SEEK_CUR);

  m_recordsInOrder.push_back(PMDRecordContainer(recType, offset, seqNum, numRecs));
  m_records[recType].push_back((unsigned)(m_recordsInOrder.size() - 1));
  return numRecs;
}

void PMDParser::parseTableOfContents(uint32_t offset, uint16_t length) try
{
  PMD_DEBUG_MSG(("[TOC] Seeking to offset 0x%x to read ToC\n", offset));
  seek(m_input, offset);
  PMD_DEBUG_MSG(("[TOC] entries to read: %d\n", length));
  for (unsigned i = 0; i < length; ++i)
  {
    unsigned numRead = readNextRecordFromTableOfContents(i);
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

  i = m_records.find(COLORS);
  if (i != m_records.end()
      && !(i->second.empty()))
  {
    parseColors(m_recordsInOrder[i->second[0]]);
  }
  else
  {
    throw RecordNotFoundException(COLORS);
  }

  i = m_records.find(FONTS_PARENT);
  if (i != m_records.end()
      && !(i->second.empty()))
  {
    parseFonts(m_recordsInOrder[i->second[0]]);
  }
  else
  {
    throw RecordNotFoundException(FONTS_PARENT);
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

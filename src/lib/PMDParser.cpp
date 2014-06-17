#include <stdint.h>
#include <string>
#include <cassert>
#include <vector>
#include <limits>
#include <math.h>
#include <librevenge/librevenge.h>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include "PMDRecord.h"
#include "PMDParser.h"
#include "PMDCollector.h"
#include "PMDExceptions.h"
#include "constants.h"
#include "offsets.h"
#include "libpagemaker_utils.h"
#include "geometry.h"

namespace libpagemaker
{
PMDParser::PMDParser(librevenge::RVNGInputStream *input, PMDCollector *collector)
  : m_input(input), m_collector(collector), m_records(), m_bigEndian(false),
    m_recordsInOrder()
{ }


template <typename T> T tryReadRecordAt(librevenge::RVNGInputStream *input,
                                        bool bigEndian, PMDRecordContainer container,
                                        unsigned recordIndex,
                                        uint32_t offsetWithinRecord, const std::string &errorMsg)
{
  try
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
    seek(input, recordOffset + offsetWithinRecord);
    switch (sizeof(T))
    {
    case 1:
      return T(readU8(input, bigEndian));
    case 2:
      return T(readU16(input, bigEndian));
    case 4:
      return T(readU32(input, bigEndian));
    case 8:
      return T(readU64(input, bigEndian));
    default:
      assert(0);
    }
  }
  catch (PMDStreamException)
  {
  }
  throw CorruptRecordException(container.m_recordType, errorMsg);
}

PMDShapePoint tryReadPointFromRecord(librevenge::RVNGInputStream *input,
                                     bool bigEndian, PMDRecordContainer container,
                                     unsigned recordIndex,
                                     uint32_t offsetWithinRecord, const std::string &errorMsg)
{
  PMDShapeUnit x(tryReadRecordAt<int16_t>(input, bigEndian, container, recordIndex, offsetWithinRecord,
                                          errorMsg));
  PMDShapeUnit y(tryReadRecordAt<int16_t>(input, bigEndian, container, recordIndex, offsetWithinRecord + 2,
                                          errorMsg));
  return PMDShapePoint(x, y);
}

void PMDParser::parseGlobalInfo(PMDRecordContainer container)
{
  uint16_t pageHeight = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, 0,
                                                  PAGE_HEIGHT_OFFSET, "Can't find page height in global attributes record.");

  uint16_t leftPageRightBound = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, 0,
                                                          LEFT_PAGE_RIGHT_BOUND_OFFSET, "Can't find the right boundary of the left page.");

  bool doubleSided = (leftPageRightBound == 0);
  m_collector->setDoubleSided(doubleSided);

  m_collector->setPageHeight(pageHeight);
}

void PMDParser::parseLine(PMDRecordContainer container, unsigned recordIndex,
                          unsigned pageID)
{
  PMDShapePoint topLeft = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                 recordIndex, SHAPE_TOP_LEFT_OFFSET, "Can't read line first point.");
  PMDShapePoint botRight = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                  recordIndex, SHAPE_BOT_RIGHT_OFFSET, "Can't read line second point.");
  bool mirrored = false;
  uint16_t temp = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, recordIndex , LINE_MIRRORED_OFFSET, "Can't read line mirror.");

  if (temp != 257 && temp != 0)
    mirrored = true;

  uint8_t strokeColor = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_COLOR_OFFSET, "Can't read rectangle stroke color.");
  uint8_t strokeType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TYPE_OFFSET, "Can't read rectangle stroke type.");
  uint8_t strokeWidth = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_WIDTH_OFFSET, "Can't read rectangle stroke width.");
  uint8_t strokeTint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TINT_OFFSET, "Can't read rectangle stroke tint.");
  uint8_t strokeOverprint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_OVERPRINT_OFFSET, "Can't read rectangle stroke overprint.");

  boost::shared_ptr<PMDLineSet> newShape(new PMDLine(topLeft, botRight, mirrored, strokeType, strokeWidth, strokeColor, strokeOverprint, strokeTint));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseTextBox(PMDRecordContainer container, unsigned recordIndex,
                             unsigned pageID)
{
  PMDShapePoint topLeft = tryReadPointFromRecord(m_input, m_bigEndian, container,
                          recordIndex, SHAPE_TOP_LEFT_OFFSET, "Can't read text box top-left point.");
  PMDShapePoint botRight = tryReadPointFromRecord(m_input, m_bigEndian, container,
                           recordIndex, SHAPE_BOT_RIGHT_OFFSET, "Can't read text box bottom-right point.");
  uint32_t textBoxRotationDegree = 0;
  uint32_t textBoxSkewDegree = 0;
  PMDShapePoint xformTopLeft = PMDShapePoint(0,0);
  PMDShapePoint xformBotRight = PMDShapePoint(0,0);
  PMDShapePoint rotatingPoint = PMDShapePoint(0, 0);

  uint16_t textBoxTextPropsOne = 0;
  uint16_t textBoxTextPropsTwo = 0;
  uint16_t textBoxText = 0;
  uint16_t textBoxChars = 0;
  uint16_t textBoxPara = 0;
  uint16_t textBoxTextStyle = 0;

  uint32_t textBoxXformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_XFORM_ID_OFFSET, "Can't read text box xform id.");

  if (textBoxXformId != (std::numeric_limits<uint32_t>::max)())
  {
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      uint32_t xformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i,
                         XFORM_ID_OFFSET, "Can't find xform id.");
      if (xformId == textBoxXformId)
      {
        textBoxRotationDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_RECT_ROTATION_OFFSET, "Can't read text box rotation.");
        textBoxSkewDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_SKEW_OFFSET, "Can't read text box skew.");
        rotatingPoint = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_ROTATING_POINT_OFFSET, "Can't read rotating point.");
        xformTopLeft = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_TOP_LEFT_OFFSET, "Can't read xform top-left point.");
        xformBotRight = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_BOT_RIGHT_OFFSET, "Can't read xform bot-right point.");
        break;
      }
    }
  }
  int32_t temp = (int32_t)textBoxRotationDegree;
  double rotationRadian = -1 * (double)temp/1000 * (M_PI/180);
  temp = (int32_t)textBoxSkewDegree;
  double skewRadian = -1 * (double)temp/1000 * (M_PI/180);

  uint32_t textBoxTextBlockId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_TEXT_BLOCK_ID_OFFSET, "Can't read text box text block id.");

  const PMDRecordContainer *ptrToTextBlockContainer = &(m_recordsInOrder[TEXT_BLOCK_OFFSET]);
  const PMDRecordContainer &textBlockContainer = *ptrToTextBlockContainer;

  for (unsigned i = 0; i < textBlockContainer.m_numRecords; ++i)
  {
    uint32_t textBlockId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, textBlockContainer, i,
                           TEXT_BLOCK_ID_OFFSET, "Can't find textBlock id.");

    if (textBlockId == textBoxTextBlockId)
    {
      textBoxTextPropsOne = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, textBlockContainer, i , TEXT_BLOCK_TEXT_PROPS_ONE_OFFSET, "Can't read text box props one.");
      textBoxTextPropsTwo = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, textBlockContainer, i , TEXT_BLOCK_TEXT_PROPS_TWO_OFFSET, "Can't read text box props two.");
      textBoxText = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, textBlockContainer, i , TEXT_BLOCK_TEXT_OFFSET, "Can't read text box text.");
      textBoxChars = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, textBlockContainer, i , TEXT_BLOCK_CHARS_OFFSET, "Can't read text box chars.");
      textBoxPara = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, textBlockContainer, i , TEXT_BLOCK_PARA_OFFSET, "Can't read text box para.");
      textBoxTextStyle = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, textBlockContainer, i , TEXT_BLOCK_TEXT_STYLE_OFFSET, "Can't read text box style.");
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
  for (unsigned i = 0; i < textContainer.m_numRecords; ++i)
  {
    text.push_back(tryReadRecordAt<uint8_t>(m_input, m_bigEndian, textContainer, 0, i,"Can't read text box text."));
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
    uint16_t length = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, charsContainer, i, 0x00,"Can't read text box text length.");
    uint16_t fontFace = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, charsContainer, i, 0x02,"Can't read text box text font face.");
    uint16_t fontSize = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, charsContainer, i, 0x04,"Can't read text box text font size.");
    uint8_t boldItalicUnderline = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, charsContainer, i, 0x0a,"Can't read text box text bold-italic-undeline.");
    uint8_t superSubscript = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, charsContainer, i, 0x0b,"Can't read text box text subsuperscript.");
    int16_t kerning = tryReadRecordAt<int16_t>(m_input, m_bigEndian, charsContainer, i, 0x10,"Can't read text box text kerning.");

    charProps.push_back(PMDCharProperties(length,fontFace,fontSize,boldItalicUnderline,superSubscript,kerning));
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
    uint16_t length = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, paraContainer, i, 0x00,"Can't read text box para length.");
    uint8_t align = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, paraContainer, i, 0x03,"Can't read text box para align.");
    uint16_t leftIndent = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, paraContainer, i, 0x0a,"Can't read text box para align.");
    uint16_t firstIndent = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, paraContainer, i, 0x0c,"Can't read text box para align.");
    uint16_t rightIndent = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, paraContainer, i, 0x0e,"Can't read text box para align.");
    uint16_t beforeIndent = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, paraContainer, i, 0x10,"Can't read text box para align."); // Above Para Spacing
    uint16_t afterIndent = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, paraContainer, i, 0x12,"Can't read text box para align.");  // Below Para Spacing

    paraProps.push_back(PMDParaProperties(length,align,leftIndent,firstIndent,rightIndent,beforeIndent,afterIndent));
  }


  boost::shared_ptr<PMDLineSet> newShape(new PMDTextBox(topLeft, botRight, rotationRadian, skewRadian, rotatingPoint, xformTopLeft, xformBotRight, text, charProps, paraProps));
  m_collector->addShapeToPage(pageID, newShape);

}

void PMDParser::parseRectangle(PMDRecordContainer container, unsigned recordIndex,
                               unsigned pageID)
{
  PMDShapePoint topLeft = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                 recordIndex, SHAPE_TOP_LEFT_OFFSET, "Can't read rectangle top-left point.");
  PMDShapePoint botRight = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                  recordIndex, SHAPE_BOT_RIGHT_OFFSET, "Can't read rectangle bottom-right point.");
  uint32_t rectRotationDegree = 0;
  uint32_t rectSkewDegree = 0;
  PMDShapePoint xformTopLeft = PMDShapePoint(0,0);
  PMDShapePoint xformBotRight = PMDShapePoint(0,0);
  PMDShapePoint rotatingPoint = PMDShapePoint(0, 0);
  uint32_t rectXformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_XFORM_ID_OFFSET, "Can't read rectangle xform id.");

  uint8_t fillColor = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_COLOR_OFFSET, "Can't read rectangle fill color.");
  uint8_t fillType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_TYPE_OFFSET, "Can't read rectangle fill type.");
  uint8_t fillOverprint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_OVERPRINT_OFFSET, "Can't read rectangle fill overprint.");
  uint8_t fillTint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_TINT_OFFSET, "Can't read rectangle fill tint.");

  uint8_t strokeColor = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_COLOR_OFFSET, "Can't read rectangle stroke color.");
  uint8_t strokeType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TYPE_OFFSET, "Can't read rectangle stroke type.");
  uint8_t strokeWidth = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_WIDTH_OFFSET, "Can't read rectangle stroke width.");
  uint8_t strokeTint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TINT_OFFSET, "Can't read rectangle stroke tint.");
  uint8_t strokeOverprint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_OVERPRINT_OFFSET, "Can't read rectangle stroke overprint.");


  if (rectXformId != (std::numeric_limits<uint32_t>::max)())
  {
    PMD_DEBUG_MSG(("Rectangle contains rotation\n"));
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      uint32_t xformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i,
                                                   XFORM_ID_OFFSET, "Can't find xform id.");
      if (xformId == rectXformId)
      {
        rectRotationDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_RECT_ROTATION_OFFSET, "Can't read rectangle rotation.");
        rectSkewDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_SKEW_OFFSET, "Can't read rectangle skew.");
        rotatingPoint = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_ROTATING_POINT_OFFSET, "Can't read rotating point.");
        xformTopLeft = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_TOP_LEFT_OFFSET, "Can't read xform top-left point.");
        xformBotRight = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_BOT_RIGHT_OFFSET, "Can't read xform bot-right point.");
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

void PMDParser::parsePolygon(PMDRecordContainer container, unsigned recordIndex,
                             unsigned pageID)
{
  PMDShapePoint topLeft = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                 recordIndex, SHAPE_TOP_LEFT_OFFSET, "Can't read bbox top-left point.");
  PMDShapePoint botRight = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                  recordIndex, SHAPE_BOT_RIGHT_OFFSET, "Can't read bbox bottom-right point.");
  uint16_t lineSetSeqNum = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, recordIndex,
                                                     POLYGON_LINE_SEQNUM_OFFSET, "Can't find seqNum of line set record in polygon record.");
  uint8_t closedMarker = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex,
                                                  POLYGON_CLOSED_MARKER_OFFSET, "Can't find the byte telling whether the polygon is open or closed.");

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
    points.push_back(tryReadPointFromRecord(m_input, m_bigEndian, lineSetContainer, i, 0,
                                            (boost::format("Couldn't read point %d from line set container at seqnum %d.\n") % i % lineSetSeqNum).str()));
  }

  uint32_t polyRotationDegree = 0;
  uint32_t polyXformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_XFORM_ID_OFFSET, "Can't read polygon xform id.");

  uint8_t fillColor = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_COLOR_OFFSET, "Can't read polygon fill color.");
  uint8_t fillType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_TYPE_OFFSET, "Can't read polygon fill type.");
  uint8_t fillOverprint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_OVERPRINT_OFFSET, "Can't read polygon fill overprint.");
  uint8_t fillTint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_TINT_OFFSET, "Can't read polygon fill tint.");

  uint8_t strokeColor = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_COLOR_OFFSET, "Can't read polygon stroke color.");
  uint8_t strokeType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TYPE_OFFSET, "Can't read polygon stroke type.");
  uint8_t strokeWidth = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_WIDTH_OFFSET, "Can't read polygon stroke width.");
  uint8_t strokeTint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TINT_OFFSET, "Can't read polygon stroke tint.");
  uint8_t strokeOverprint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_OVERPRINT_OFFSET, "Can't read polygon stroke overprint.");

  if (polyXformId != (std::numeric_limits<uint32_t>::max)())
  {
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      uint32_t xformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i,
                                                   XFORM_ID_OFFSET, "Can't find xform id.");
      if (xformId == polyXformId)
      {
        polyRotationDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_RECT_ROTATION_OFFSET, "Can't read polygon rotation.");
        polySkewDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_SKEW_OFFSET, "Can't read polygon skew.");
        xformTopLeft = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_TOP_LEFT_OFFSET, "Can't read xform top-left point.");
        xformBotRight = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_BOT_RIGHT_OFFSET, "Can't read xform bot-right point.");
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

void PMDParser::parseEllipse(PMDRecordContainer container, unsigned recordIndex, unsigned pageID)
{

  PMDShapePoint bboxTopLeft = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                     recordIndex, SHAPE_TOP_LEFT_OFFSET, "Can't read bbox top-left point.");
  PMDShapePoint bboxBotRight = tryReadPointFromRecord(m_input, m_bigEndian, container,
                                                      recordIndex, SHAPE_BOT_RIGHT_OFFSET, "Can't read bbox bottom-right point.");

  uint32_t ellipseRotationDegree = 0;
  uint32_t ellipseSkewDegree = 0;
  PMDShapePoint xformTopLeft = PMDShapePoint(0,0);
  PMDShapePoint xformBotRight = PMDShapePoint(0,0);
  uint32_t ellipseXformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_XFORM_ID_OFFSET, "Can't read ellipse xform id.");

  uint8_t fillColor = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_COLOR_OFFSET, "Can't read rectangle fill color.");
  uint8_t fillType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_TYPE_OFFSET, "Can't read rectangle fill type.");
  uint8_t fillOverprint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_OVERPRINT_OFFSET, "Can't read rectangle fill overprint.");
  uint8_t fillTint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_FILL_TINT_OFFSET, "Can't read rectangle fill tint.");

  uint8_t strokeColor = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_COLOR_OFFSET, "Can't read rectangle stroke color.");
  uint8_t strokeType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TYPE_OFFSET, "Can't read rectangle stroke type.");
  uint8_t strokeWidth = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_WIDTH_OFFSET, "Can't read rectangle stroke width.");
  uint8_t strokeTint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_TINT_OFFSET, "Can't read rectangle stroke tint.");
  uint8_t strokeOverprint = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex, SHAPE_STROKE_OVERPRINT_OFFSET, "Can't read rectangle stroke overprint.");

  if (ellipseXformId != (std::numeric_limits<uint32_t>::max)())
  {
    const PMDRecordContainer *ptrToXformContainer = &(m_recordsInOrder[0x0c]);
    const PMDRecordContainer &xformContainer = *ptrToXformContainer;

    for (unsigned i = 0; i < xformContainer.m_numRecords; ++i)
    {
      uint32_t xformId = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i,
                                                   XFORM_ID_OFFSET, "Can't find xform id.");
      if (xformId == ellipseXformId)
      {
        ellipseRotationDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_RECT_ROTATION_OFFSET, "Can't read ellipse rotation.");
        ellipseSkewDegree = tryReadRecordAt<uint32_t>(m_input, m_bigEndian, xformContainer, i , XFORM_SKEW_OFFSET, "Can't read ellipse skew.");
        xformTopLeft = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_TOP_LEFT_OFFSET, "Can't read xform top-left point.");
        xformBotRight = tryReadPointFromRecord(m_input, m_bigEndian, xformContainer, i, XFORM_BOT_RIGHT_OFFSET, "Can't read xform bot-right point.");
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
    uint8_t shapeType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, i,
                                                 SHAPE_TYPE_OFFSET, "Can't find shape type in shape.");
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
      //parseRectangle(container, i, pageID);
      break;
    default:
      PMD_ERR_MSG("Encountered shape of unknown type.\n");
      continue;
    }
  }
}

void PMDParser::parsePages(PMDRecordContainer container)
{
  uint16_t pageWidth = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, 0, PAGE_WIDTH_OFFSET,
                                                 "Can't find page width in first page record.");
  m_collector->setPageWidth(pageWidth);
  for (unsigned i = 0; i < container.m_numRecords; ++i)
  {
    uint16_t shapesSeqNum = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, i,
                                                      PAGE_SHAPE_SEQNUM_OFFSET,
                                                      (boost::format("Can't find shape record sequence number in page record at index %d") % i).str());
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
  uint16_t recType = readU16(m_input, true);
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

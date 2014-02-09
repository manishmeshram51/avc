#include <string>
#include <cassert>
#include <vector>
#include <librevenge/librevenge.h>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include "PMDRecord.h"
#include "PMDParser.h"
#include "PMDCollector.h"
#include "PMDExceptions.h"
#include "constants.h"
#include "offsets.h"
#include "libpagemaker_utils.h"

namespace libpagemaker
{
PMDParser::PMDParser(librevenge::RVNGInputStream *input, PMDCollector *collector)
  : m_input(input), m_collector(collector), m_records(), m_bigEndian(false),
    m_recordsInOrder()
{ }

PMDShapePoint tryReadPointFromRecord(librevenge::RVNGInputStream *input,
                                     bool bigEndian, PMDRecordContainer container,
                                     unsigned recordIndex,
                                     uint32_t offsetWithinRecord, const std::string &errorMsg)
{
  PMDShapeUnit x(tryReadRecordAt(input, bigEndian, container, recordIndex, offsetWithinRecord,
    errorMsg));
  PMDShapeUnit y(tryReadRecordAt(input, bigEndian, container, recordIndex, offsetWithinRecord + 2,
    errorMsg));
  return PMDShapePoint(x, y);
}


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
      boost::optional<unsigned> sizePerRecord = getRecordSize(container.m_recordTYpe);
      if (!sizePerRecord.is_initialized())
      {
        throw UnknownRecordSizeException(container.m_recordType);
      }
      recordOffset += sizePerRecord.get() * recordIndex;
    }
    seek(input, record.m_offset + offsetWithinRecord);
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
  throw CorruptRecordException(record.m_recordType, errorMsg);
}

void PMDParser::parseGlobalInfo(PMDRecordContainer container)
{
  uint16_t pageHeight = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, 0
                        PAGE_HEIGHT_OFFSET, "Can't find page height in global attributes record.");

  m_collector->setPageHeight(pageHeight);
}

void PMDParser::parseRectangle(PMDRecordContainer container, unsigned recordIndex,
                               unsigned pageID)
{
  PMDShapePoint topLeft = tryReadPointFromRecord(m_input, m_bigEndian, container,
    recordIndex, RECT_TOP_LEFT_OFFSET, "Can't read rectangle top-left point.");
  PMDShapePoint botRight = tryReadPointFromRecord(m_input, m_bigEndian, container,
    recordIndex, RECT_BOT_RIGHT_OFFSET, "Can't read rectangle bottom-right point.");
  
  std::shared_ptr<PMDLineSet> newShape(new PMDRectangle(topLeft, botRight));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parsePolygon(PMDRecordContainer container, unsigned recordIndex,
                             unsigned pageID)
{
  uint16_t lineSetSeqNum = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, container, recordIndex,
    POLYGON_LINE_SEQNUM_OFFSET, "Can't find seqNum of line set record in polygon record.");
  uint8_t closedMarker = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, recordIndex,
    POLYGON_CLOSED_MARKER_OFFSET, "Can't find the byte telling whether the polygon is open or closed.");
  bool closed;
  switch (closedMarker)
  {
  default:
    PMD_ERR_MSG(
      boost::format("Unknown value %d for polygon closed/open marker. Defaulting to closed.\n",
        closedMarker));
    // Intentional fall-through.
  case POLYGON_CLOSED:
    closed = true;
    break;
  case POLYGON_OPEN:
    closed = false;
    break;
  }
  
  const PMDRecordContainer *ptrToLineSetContainer =
    (lineSetSeqNum < m_recordsInOrder.size()) ? m_recordsInOrder[lineSetSeqNum] : NULL;
  if (!ptrToLineSetContainer)
  {
    PMD_ERR_MSG(boost::format("No line set with sequence number %d\n", lineSetSeqNum));
    return;
  }
  const PMDRecordContainer &lineSetContainer = *(ptrToLineSetContainer);
  std::vector<PMDShapePoint> points(lineSetContainer.m_numRecords);
  for (unsigned i = 0; i < lineSetContainer.m_numRecords; ++i)
  {
    points.push_back(m_input, m_bigEndian, container, i, 0,
      boost::format("Couldn't read point %d from line set container at seqnum %d.\n",
        i, lineSetSeqNum));
  }
  std::shared_ptr<PMDLineSet> newShape(new PMDPolygon(points, closed));
  m_collector->addShapeToPage(pageID, newShape);
}

void PMDParser::parseShapes(uint16_t seqNum, unsigned pageID)
{
  const PMDRecordContainer *ptrToContainer =
    (seqNum < m_recordsInOrder.size()) ? m_recordsInOrder[seqNum] : NULL;
  if (!container)
  {
    throw RecordNotFoundException(SHAPE, seqNum);
  }
  const PMDRecordContainer &container = *(m_recordsInOrder[seqNum]);
  for (unsigned i = 0; i < container.m_numRecords; ++i)
  {
    uint8_t shapeType = tryReadRecordAt<uint8_t>(m_input, m_bigEndian, container, i,
      SHAPE_TYPE_OFFSET, "Can't find shape type in shape.");
    switch (shapeType)
    {
    case RECTANGLE_RECORD:
      parseRectangle(container, i, pageID);
      break;
    case POLYGON_RECORD:
      parsePolygon(container, i, pageID);
      break;
    default:
      PMD_ERR_MSG(boost::format("Encountered shape of unknown type %d.\n", shapeType));
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
      boost::format("Can't find shape record sequence number in page record at index %d", i));
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

void PMDParser::readNextRecordFromTableOfContents(unsigned seqNum, bool seekToNext)
{
  long currentPosition = m_input->tell();

  uint16_t recType = readU16(m_input, m_bigEndian);
  uint16_t numRecs = readU16(m_input, m_bigEndian);
  uint32_t offset = readU32(m_input, m_bigEndian);

  if (seekToNext)
  {
    seek(m_input, currentPosition + 16);
  }

  m_records[recType].push_back(PMDRecordContainer(recType, offset, numRecs, seqNum));
  m_recordsInOrder.push_back(&(m_records[recType].back()));
  return numRecs;
}

void PMDParser::parseTableOfContents(uint32_t offset, uint16_t length) try
{
  PMD_DEBUG_MSG(("[TOC] Seeking to offset 0x%x to read ToC\n", offset));
  seek(m_input, offset);
  PMD_DEBUG_MSG(("[TOC] entries to read: %d\n", length));
  for (unsigned i = 0; i < length; ++i)
  {
    unsigned numRead = readNextRecordFromTableOfContents(currentSeqNum, i != length);
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

  typedef std::map<uint16_t, std::vector<PMDRecordContainer> >::iterator RecIter;

  RecIter i = m_records.find(GLOBAL_INFO);
  if (i != m_records.end()
      && !(i->second.empty()))
  {
    parseGlobalInfo(i->second[0]);
  }
  else
  {
    throw RecordNotFoundException(GLOBAL_INFO);
  }
  i = m_records.find(PAGE);
  if (i != m_records.end()
      && !(i->second.empty()))
  {
    parsePages(i->second[0]);
  }
  else
  {
    throw RecordNotFoundException(PAGE);
  }
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

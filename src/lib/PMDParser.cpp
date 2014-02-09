#include <string>
#include <cassert>
#include <vector>
#include <librevenge/librevenge.h>
#include <boost/optional.hpp>
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

template <typename T> T tryReadRecordAt(librevenge::RVNGInputStream *input,
                                        bool bigEndian, PMDRecordContainer container,
                                        uint16_t recordIndex,
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

void PMDParser::parsePages(PMDRecordContainer container)
{
  uint16_t pageWidth = tryReadRecordAt<uint16_t>(m_input, m_bigEndian, record, 0, PAGE_WIDTH_OFFSET,
                       "Can't find page width in first page record.");
  m_collector->setPageWidth(pageWidth);
  for (unsigned i = 0; i < container.m_numRecords; ++i)
  {
    m_collector->addPage();
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

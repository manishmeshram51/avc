#pragma once
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
  std::map<uint16_t, std::vector<PMDRecord> > m_records;
  bool m_bigEndian;

  /* Private functions. */
  void parseGlobalInfo(const std::vector<PMDRecord> &);
  void parsePages(const std::vector<PMDRecord> &);
  void parseHeader(uint32_t *tocOffset, uint16_t *tocLength);
  unsigned readNextRecordFromTableOfContents(unsigned seqNum, bool seekToNext);
  void parseTableOfContents(uint32_t offset, uint16_t length);

  /* Prevent copy and assignment */
  PMDParser &operator=(const PMDParser &);
  PMDParser(const PMDParser &);
public:
  PMDParser(librevenge::RVNGInputStream *, PMDCollector *);
  void parse();
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

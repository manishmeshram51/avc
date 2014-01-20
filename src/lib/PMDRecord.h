#pragma once
#include <stdint.h>
#include <librevenge/librevenge.h>
#include <boost/optional.hpp>

#include "constants.h"

namespace libpagemaker
{

struct PMDRecord
{
  uint16_t m_recordType;
  uint32_t m_offset;
  unsigned m_seqNum;
  
  PMDRecord(uint16_t recordType, uint32_t offset, unsigned seqNum)
    : m_recordType(recordType), m_offset(offset), m_seqNum(seqNum)
  { }
};

inline boost::optional<unsigned> getRecordSize(uint16_t recType)
{
  switch (recType)
  {
  case GLOBAL_INFO:
    return 2496;
  case PAGE:
    return 472;
  default:
    return boost::none;
  }
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

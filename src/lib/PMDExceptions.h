#pragma once
#include <librevenge/librevenge.h>
#include <boost/format.hpp>
namespace libpagemaker
{

struct PMDParseException
{
  std::string m_message;
  PMDParseException(std::string message)
   : m_message(message)
  { }
  virtual ~PMDParseException() { }
};

struct RecordNotFoundException : public PMDParseException
{
  uint16_t m_recordType;

  RecordNotFoundException(uint16_t recordType)
    : PMDParseException((boost::format("Record not found: %d") % recordType).str()),
      m_recordType(recordType)
  { }
};

struct CorruptRecordException : public PMDParseException
{
  uint16_t m_recordType;

  CorruptRecordException(uint16_t recordType, std::string message)
    : PMDParseException((boost::format("Corrupt record: %d\nError message: %s\n") % recordType % message).str()),
      m_recordType(recordType)
  { }
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
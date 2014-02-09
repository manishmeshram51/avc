#pragma once
#include <stdint.h>
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
  RecordNotFoundException(uint16_t recordType, uint16_t seqNum)
    : PMDParseException((boost::format("Record of type %d not found at seqNum %d") % recordType % seqNum).str()),
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

struct EmptyLineSetException
{
};

struct UnknownRecordSizeException : public PMDParseException
{
  uint16_t m_recordType;

  UnknownRecordSizeException(uint16_t recordType)
    : PMDParseException(boost::format("Tried to parse record %d of unknown size.\n")),
      m_recordType(recordTYpe)
  { }
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

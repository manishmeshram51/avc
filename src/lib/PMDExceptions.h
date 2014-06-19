/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDEXCEPTIONS_H__
#define __PMDEXCEPTIONS_H__

#include <stdint.h>
#include <librevenge/librevenge.h>
#include <boost/format.hpp>
namespace libpagemaker
{

struct PMDParseException
{
  std::string m_message;
  PMDParseException(const std::string &message)
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

  CorruptRecordException(uint16_t recordType, const std::string &message)
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
    : PMDParseException((boost::format("Tried to parse record %d of unknown size.\n") % recordType).str()),
      m_recordType(recordType)
  { }
};

}

#endif /* __PMDEXCEPTIONS_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

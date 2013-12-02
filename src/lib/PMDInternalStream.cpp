/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>

#include "PMDInternalStream.h"

namespace libpagemaker
{

PMDInternalStream::PMDInternalStream(const unsigned char *data, unsigned long size)
  : librevenge::RVNGInputStream()
  , m_offset(0)
  , m_size(size)
  , m_data(data)
  , m_tmpBuf(0)
  , m_dataOwner(false)
{
}

PMDInternalStream::PMDInternalStream(const std::vector<unsigned char> &data)
  : librevenge::RVNGInputStream()
  , m_offset(0)
  , m_size(data.size())
  , m_data(&data[0])
  , m_tmpBuf(0)
  , m_dataOwner(false)
{
}

PMDInternalStream::PMDInternalStream(const RVNGInputStreamPtr &input, unsigned long size)
  : librevenge::RVNGInputStream()
  , m_offset(0)
  , m_size(0)
  , m_data(0)
  , m_tmpBuf(0)
  , m_dataOwner(true)
{
  const unsigned char *const bytes = readNBytes(input, size);
  unsigned char *const data = new unsigned char[size];
  m_data = data;
  m_dataOwner = true;
  m_size = size;
  std::uninitialized_copy(bytes, bytes + size, data);
}

PMDInternalStream::~PMDInternalStream()
{
  if (m_dataOwner)
    delete [] m_data;
  if (m_tmpBuf)
    delete [] m_tmpBuf;
}

bool PMDInternalStream::isStructured()
{
  return false;
}

unsigned PMDInternalStream::subStreamCount()
{
  return 0;
}

const char *PMDInternalStream::subStreamName(unsigned)
{
  return 0;
}

librevenge::RVNGInputStream *PMDInternalStream::getSubStreamByName(const char *)
{
  return 0;
}

librevenge::RVNGInputStream *PMDInternalStream::getSubStreamById(unsigned)
{
  return 0;
}

const unsigned char *PMDInternalStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
  numBytesRead = 0;

  if (0 == numBytes)
    return 0;

  if (m_tmpBuf)
    delete [] m_tmpBuf;
  m_tmpBuf = 0;

  int numBytesToRead;

  if ((m_offset+numBytes) < m_size)
    numBytesToRead = numBytes;
  else
    numBytesToRead = m_size - m_offset;

  numBytesRead = numBytesToRead; // about as paranoid as we can be..

  if (numBytesToRead == 0)
    return 0;

  m_tmpBuf = new unsigned char[numBytesToRead];
  for (unsigned long i=0; (long)i<(long)numBytesToRead; i++)
  {
    m_tmpBuf[i] = m_data[m_offset];
    m_offset++;
  }

  return m_tmpBuf;
}

int PMDInternalStream::seek(long offset, librevenge::RVNG_SEEK_TYPE seekType)
{
  if (seekType == librevenge::RVNG_SEEK_CUR)
    m_offset += offset;
  else if (seekType == librevenge::RVNG_SEEK_SET)
    m_offset = offset;

  if (m_offset < 0)
  {
    m_offset = 0;
    return 1;
  }
  if ((long)m_offset > (long)m_size)
  {
    m_offset = m_size;
    return 1;
  }

  return 0;
}

long PMDInternalStream::tell()
{
  return m_offset;
}

bool PMDInternalStream::isEnd()
{
  if ((long)m_offset == (long)m_size)
    return true;

  return false;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

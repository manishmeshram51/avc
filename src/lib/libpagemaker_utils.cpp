/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "libpagemaker_utils.h"

namespace libpagemaker
{

namespace
{

void checkStream(const RVNGInputStreamPtr &input)
{
  if (!input || input->isEnd())
    throw EndOfStreamException();
}

struct SeekFailedException : public PMDStreamException {};

}

uint8_t readU8(const RVNGInputStreamPtr &input, bool /* bigEndian */)
{
  checkStream(input);

  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint8_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint8_t))
    return *(uint8_t const *)(p);
  throw EndOfStreamException();
}

int8_t readS8(const RVNGInputStreamPtr &input, bool /* bigEndian */)
{
  return static_cast<int8_t>(readU8(input));
}

uint16_t readU16(const RVNGInputStreamPtr &input, bool bigEndian)
{
  checkStream(input);

  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint16_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint16_t))
  {
    if (bigEndian)
      return static_cast<uint16_t>((uint16_t)p[1]|((uint16_t)p[0]<<8));
    return static_cast<uint16_t>((uint16_t)p[0]|((uint16_t)p[1]<<8));
  }
  throw EndOfStreamException();
}

int16_t readS16(const RVNGInputStreamPtr &input, const bool bigEndian)
{
  return static_cast<int16_t>(readU16(input, bigEndian));
}

uint32_t readU32(const RVNGInputStreamPtr &input, bool bigEndian)
{
  checkStream(input);

  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint32_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint32_t))
  {
    if (bigEndian)
      return (uint32_t)p[3]|((uint32_t)p[2]<<8)|((uint32_t)p[1]<<16)|((uint32_t)p[0]<<24);
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
  }
  throw EndOfStreamException();
}

int32_t readS32(const RVNGInputStreamPtr &input, const bool bigEndian)
{
  return static_cast<int32_t>(readU32(input, bigEndian));
}

uint64_t readU64(const RVNGInputStreamPtr &input, bool bigEndian)
{
  checkStream(input);

  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint64_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint64_t))
  {
    if (bigEndian)
      return (uint64_t)p[7]|((uint64_t)p[6]<<8)|((uint64_t)p[5]<<16)|((uint64_t)p[4]<<24)|((uint64_t)p[3]<<32)|((uint64_t)p[2]<<40)|((uint64_t)p[1]<<48)|((uint64_t)p[0]<<56);
    return (uint64_t)p[0]|((uint64_t)p[1]<<8)|((uint64_t)p[2]<<16)|((uint64_t)p[3]<<24)|((uint64_t)p[4]<<32)|((uint64_t)p[5]<<40)|((uint64_t)p[6]<<48)|((uint64_t)p[7]<<56);
  }
  throw EndOfStreamException();
}

int64_t readS64(const RVNGInputStreamPtr &input, const bool bigEndian)
{
  return static_cast<int64_t>(readU64(input, bigEndian));
}

const unsigned char *readNBytes(const RVNGInputStreamPtr &input, const unsigned long numBytes)
{
  checkStream(input);

  unsigned long readBytes = 0;
  const unsigned char *const s = input->read(numBytes, readBytes);

  if (numBytes != readBytes)
    throw EndOfStreamException();

  return s;
}

void skip(const RVNGInputStreamPtr &input, unsigned long numBytes)
{
  checkStream(input);

  seekRelative(input, static_cast<long>(numBytes));
}

void seek(const RVNGInputStreamPtr &input, const unsigned long pos)
{
  if (!input)
    throw EndOfStreamException();

  if (0 != input->seek(static_cast<long>(pos), librevenge::RVNG_SEEK_SET))
    throw SeekFailedException();
}

void seekRelative(const RVNGInputStreamPtr &input, const long pos)
{
  if (!input)
    throw EndOfStreamException();

  if (0 != input->seek(pos, librevenge::RVNG_SEEK_CUR))
    throw SeekFailedException();
}

unsigned long getLength(const RVNGInputStreamPtr &input)
{
  if (!input)
    throw EndOfStreamException();

  const long orig = input->tell();
  unsigned long end = 0;

  if (0 == input->seek(0, librevenge::RVNG_SEEK_END))
    end = static_cast<unsigned long>(input->tell());
  else
  {
    // RVNG_SEEK_END does not work. Use the harder way.
    seek(input, 0);
    while (!input->isEnd())
    {
      readU8(input);
      ++end;
    }
  }

  seek(input, orig);

  return end;
}

EndOfStreamException::EndOfStreamException()
{
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

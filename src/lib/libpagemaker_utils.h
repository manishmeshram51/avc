/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __LIBPAGEMAKER_UTILS_H__
#define __LIBPAGEMAKER_UTILS_H__

#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

#include <librevenge-stream/librevenge-stream.h>
#include <librevenge/librevenge.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _MSC_VER

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned uint32_t;
typedef int int32_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;

#else

#ifdef HAVE_CONFIG_H

#include <config.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#else

// assume that the headers are there inside LibreOffice build when no HAVE_CONFIG_H is defined
#include <stdint.h>
#include <inttypes.h>

#endif

#endif

#if defined(__clang__) || defined(__GNUC__)
#  define PMD_ATTRIBUTE_PRINTF(fmt, arg) __attribute__((__format__(__printf__, fmt, arg)))
#else
#  define PMD_ATTRIBUTE_PRINTF(fmt, arg)
#endif

// debug message includes source file and line number
//#define VERBOSE_DEBUG 1

// do nothing with debug messages in a release compile
#ifdef DEBUG
namespace libpagemaker
{
void debugPrint(const char *format, ...) PMD_ATTRIBUTE_PRINTF(1, 2);
}
#ifdef VERBOSE_DEBUG
#define PMD_DEBUG_MSG(M) libpagemaker::debugPrint("%15s:%5d: ", __FILE__, __LINE__); libpagemaker::debugPrint M
#define PMD_DEBUG(M) M
#else
#define PMD_DEBUG_MSG(M) libpagemaker::debugPrint M
#define PMD_DEBUG(M) M
#endif

#define PMD_WARN_MSG(M) std::fprintf(stderr, "PageMaker [WARN] %15s:%d: ", __FILE__, __LINE__); std::fprintf(stderr, M)
#define PMD_ERR_MSG(M) std::fprintf(stderr, "PageMaker [ERROR] %15s:%d: ", __FILE__, __LINE__); std::fprintf(stderr, M)
#else
#define PMD_DEBUG_MSG(M)
#define PMD_DEBUG(M)
#define PMD_WARN_MSG(M)
#define PMD_ERR_MSG(M)
#endif

#define PMD_NUM_ELEMENTS(array) sizeof(array)/sizeof(array[0])

namespace libpagemaker
{

typedef librevenge::RVNGInputStream *RVNGInputStreamPtr;

struct PMDDummyDeleter
{
  void operator()(void *) {}
};

uint8_t readU8(const RVNGInputStreamPtr &input, bool = false);
int8_t readS8(const RVNGInputStreamPtr &input, bool = false);
uint16_t readU16(const RVNGInputStreamPtr &input, bool bigEndian=false);
int16_t readS16(const RVNGInputStreamPtr &input, bool bigEndian=false);
uint32_t readU32(const RVNGInputStreamPtr &input, bool bigEndian=false);
int32_t readS32(const RVNGInputStreamPtr &input, bool bigEndian=false);
uint64_t readU64(const RVNGInputStreamPtr &input, bool bigEndian=false);
int64_t readS64(const RVNGInputStreamPtr &input, bool bigEndian=false);

const unsigned char *readNBytes(const RVNGInputStreamPtr &input, unsigned long numBytes);

void skip(const RVNGInputStreamPtr &input, unsigned long numBytes);

void seek(const RVNGInputStreamPtr &input, unsigned long pos);
void seekRelative(const RVNGInputStreamPtr &input, long pos);

unsigned long getLength(const RVNGInputStreamPtr &input);

struct PMDStreamException
{
  virtual ~PMDStreamException() { }
};

struct EndOfStreamException : public PMDStreamException
{
  EndOfStreamException();
};

struct GenericException
{
};

}

#endif // __LIBPAGEMAKER_UTILS_H__

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

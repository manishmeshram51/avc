/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This file is a part of the libpagemaker project.
 *
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2013 David Tardon (dtardon@redhat.com)
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 */

#ifndef LIBPMD_UTILS_H_INCLUDED
#define LIBPMD_UTILS_H_INCLUDED

#ifdef DEBUG
#include <cstdio>
#endif

#include <string>

#include <boost/shared_ptr.hpp>

#include <librevenge-stream/librevenge-stream.h>
#include <librevenge/librevenge.h>

#ifdef _MSC_VER

typedef unsigned char uint8_t;
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

// debug message includes source file and line number
//#define VERBOSE_DEBUG 1

// do nothing with debug messages in a release compile
#ifdef DEBUG
#ifdef VERBOSE_DEBUG
#define PMD_DEBUG_MSG(M) std::printf("%15s:%5d: ", __FILE__, __LINE__); std::printf M
#define PMD_DEBUG(M) M
#else
#define PMD_DEBUG_MSG(M) std::printf M
#define PMD_DEBUG(M) M
#endif
#else
#define PMD_DEBUG_MSG(M)
#define PMD_DEBUG(M)
#endif

#define PMD_NUM_ELEMENTS(array) sizeof(array)/sizeof(array[0])

namespace libpagemaker
{

typedef boost::shared_ptr<librevenge::RVNGInputStream> RVNGInputStreamPtr;

struct PMDDummyDeleter
{
  void operator()(void *) {}
};

uint8_t readU8(const RVNGInputStreamPtr &input, bool = false);
uint16_t readU16(const RVNGInputStreamPtr &input, bool bigEndian=false);
uint32_t readU32(const RVNGInputStreamPtr &input, bool bigEndian=false);
uint64_t readU64(const RVNGInputStreamPtr &input, bool bigEndian=false);

const unsigned char *readNBytes(const RVNGInputStreamPtr &input, unsigned long numBytes);

void skip(const RVNGInputStreamPtr &input, unsigned long numBytes);

void seek(const RVNGInputStreamPtr &input, unsigned long pos);
void seekRelative(const RVNGInputStreamPtr &input, long pos);

unsigned long getLength(const RVNGInputStreamPtr &input);

struct EndOfStreamException
{
  EndOfStreamException();
};

struct GenericException
{
};

}

#endif // LIBPMD_UTILS_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

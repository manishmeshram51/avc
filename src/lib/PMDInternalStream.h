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
 * Copyright (C) 2004-2005 William Lachance (wrlach@gmail.com)
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 */

#ifndef PMDINTERNALSTREAM_H_INCLUDED
#define PMDINTERNALSTREAM_H_INCLUDED

#include <vector>

#include <librevenge-stream/librevenge-stream.h>

#include "libpagemaker_utils.h"

namespace libpagemaker
{

class PMDInternalStream : public librevenge::RVNGInputStream
{
public:
  PMDInternalStream(const unsigned char *data, unsigned long size);
  PMDInternalStream(const std::vector<unsigned char> &data);
  PMDInternalStream(const RVNGInputStreamPtr &input, unsigned long size);
  virtual ~PMDInternalStream();

  virtual bool isStructured();
  virtual unsigned subStreamCount();
  virtual const char *subStreamName(unsigned id);
  virtual librevenge::RVNGInputStream *getSubStreamByName(const char *name);
  virtual librevenge::RVNGInputStream *getSubStreamById(unsigned id);

  const virtual unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
  virtual int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType);
  virtual long tell();
  virtual bool isEnd();

private:
  long m_offset;
  unsigned long m_size;
  const unsigned char *m_data;
  unsigned char *m_tmpBuf;
  bool m_dataOwner;
  PMDInternalStream(const PMDInternalStream &);
  PMDInternalStream &operator=(const PMDInternalStream &);
};

}

#endif // PMDINTERNALSTREAM_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

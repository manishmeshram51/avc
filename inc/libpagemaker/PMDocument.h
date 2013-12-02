/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This file is part of the libpagemaker project.
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

#ifndef LIBPMD_PMDDOCUMENT_H_INCLUDED
#define LIBPMD_PMDDOCUMENT_H_INCLUDED

#include <librevenge/librevenge.h>

namespace libpagemaker
{

class PMDocument
{
public:

  /**
    Analyzes the content of an input stream to see if it can be
    parsed.

    \param input The input stream
    \return A value that
    indicates whether the content from the input stream is a
    PageMaker document that libpagemaker is able to parse
  */
  static bool isSupported(librevenge::RVNGInputStream *input);

  /**
    Parses the input stream content.

    It will make callbacks to the functions provided by a
    librevenge::RVNGDrawingInterface class implementation when needed.

    \param input The input stream
    \param painter A librevenge::RVNGDrawingInterface implementation
    \return A value that indicates whether the parsing was successful
  */
  static bool parse(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
};

} // namespace libpagemaker

#endif // LIBPMD_PMDDOCUMENT_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDOCUMENT_H__
#define __PMDOCUMENT_H__

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

  static bool parseToYaml(librevenge::RVNGInputStream *input);
};

} // namespace libpagemaker

#endif // __PMDOCUMENT_H__

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

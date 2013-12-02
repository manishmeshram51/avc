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
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
 * Copyright (C) 2004 Marc Oude Kotte (marc@solcon.nl)
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 */

#include <stdio.h>
#include <string.h>

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>

#include <libpagemaker/libpagemaker.h>

namespace
{

int printUsage()
{
  printf("Usage: pmd2raw [OPTION] <PageMaker File>\n");
  printf("\n");
  printf("Options:\n");
  printf("--callgraph           Display the call graph nesting level\n");
  printf("--help                Shows this help message\n");
  return -1;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
  bool printIndentLevel = false;
  char *file = 0;

  if (argc < 2)
    return printUsage();

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--callgraph"))
      printIndentLevel = true;
    else if (!file && strncmp(argv[i], "--", 2))
      file = argv[i];
    else
      return printUsage();
  }

  if (!file)
    return printUsage();

  librevenge::RVNGFileStream input(file);

  if (!libpagemaker::PMDocument::isSupported(&input))
  {
    fprintf(stderr, "ERROR: Unsupported file format (unsupported version) or file is encrypted!\n");
    return 1;
  }

  librevenge::RVNGRawDrawingGenerator painter(printIndentLevel);
  if (!libpagemaker::PMDocument::parse(&input, &painter))
    return 1;

  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
  printf("Usage: pmd2yaml [OPTION] <PageMaker File>\n");
  printf("\n");
  printf("Options:\n");
  printf("--help                Shows this help message\n");
  return -1;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
  char *file = 0;

  if (argc < 2)
    return printUsage();

  for (int i = 1; i < argc; i++)
  {
    if (!file && strncmp(argv[i], "--", 2))
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

  if (!libpagemaker::PMDocument::parseToYaml(&input))
    return 1;

  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

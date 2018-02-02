/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include <libpagemaker/libpagemaker.h>

#include "PMDCollector.h"
#include "PMDParser.h"
#include "libpagemaker_utils.h"

namespace libpagemaker
{

bool PMDocument::isSupported(librevenge::RVNGInputStream *input) try
{
  return input && input->isStructured() && input->existsSubStream("PageMaker");
}
catch (...)
{
  return false;
}

bool PMDocument::parse(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter) try
{
  if (!input || !painter)
    return false;

  if (!isSupported(input))
    return false;

  PMDCollector collector;
  PMD_DEBUG_MSG(("About to start parsing...\n"));
  std::unique_ptr<librevenge::RVNGInputStream> pmdStream(input->getSubStreamByName("PageMaker"));
  PMDParser(pmdStream.get(), &collector).parse();
  PMD_DEBUG_MSG(("About to start drawing...\n"));
  collector.draw(painter);
  return true;
}
catch (...)
{
  return false;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

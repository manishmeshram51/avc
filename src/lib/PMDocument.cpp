/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libpagemaker/libpagemaker.h>

#include <boost/scoped_ptr.hpp>

#include "PMDCollector.h"
#include "PMDParser.h"
#include "libpagemaker_utils.h"

namespace libpagemaker
{

bool PMDocument::isSupported(librevenge::RVNGInputStream * /*input*/) try
{
  // TODO: Fix this.
  return true;
}
catch (...)
{
  return false;
}

bool PMDocument::parse(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter)
{
  PMDCollector collector;
  PMD_DEBUG_MSG(("About to start parsing...\n"));
  boost::scoped_ptr<librevenge::RVNGInputStream>
  pmdStream(input->getSubStreamByName("PageMaker"));
  PMDParser(pmdStream.get(), &collector).parse();
  PMD_DEBUG_MSG(("About to start drawing...\n"));
  collector.draw(painter);
  return true;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

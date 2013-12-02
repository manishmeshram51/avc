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
 * Copyright (C) 2007 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 */

#include <libpagemaker/libpagemaker.h>

namespace libpagemaker
{

bool PMDocument::isSupported(librevenge::RVNGInputStream *input) try
{
  // TODO: implement me
  (void) input;
  return false;
}
catch (...)
{
  return false;
}

bool PMDocument::parse(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter) try
{
  // TODO: implement me
  (void) input;
  (void) painter;
  return false;
}
catch (...)
{
  return false;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

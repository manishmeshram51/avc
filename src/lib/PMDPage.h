/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDPAGE_H__
#define __PMDPAGE_H__

#include "geometry.h"

#include <vector>
#include <boost/shared_ptr.hpp>
#include <librevenge/librevenge.h>

namespace libpagemaker
{

class PMDPage
{
  std::vector<boost::shared_ptr<PMDLineSet> > m_shapes;
public:
  PMDPage() : m_shapes()
  { }

  void addShape(const boost::shared_ptr<PMDLineSet> &shape)
  {
    m_shapes.push_back(shape);
  }

  unsigned numShapes() const
  {
    return m_shapes.size();
  }

  boost::shared_ptr<const PMDLineSet> getShape(unsigned i) const
  {
    return m_shapes.at(i);
  }
};

}

#endif /* __PMDPAGE_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __PMDCOLLECTOR_H__
#define __PMDCOLLECTOR_H__

#include <stdint.h>

#include <vector>
#include <string>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>


#include "geometry.h"
#include "PMDPage.h"
#include "PMDExceptions.h"
#include "Units.h"
#include "OutputShape.h"
#include "PMDTypes.h"

namespace libpagemaker
{


/**
 * Builder class for PMD Documents.
 *
 * Methods are called by the PMD parser to build the document. This done,
 * the draw() method may be called to output the document to the
 * supplied drawing interface.
 */
class PMDCollector
{
  typedef std::vector<boost::shared_ptr<const OutputShape> > PageShapes_t;
  typedef std::vector<PageShapes_t> PageShapesList_t;

  /*
   * Height and width in PMD page units.
   * One PMD page unit is 1/20 of a point (1/720 inch)
   */
  boost::optional<PMDPageUnit> m_pageWidth;
  boost::optional<PMDPageUnit> m_pageHeight;

  std::vector<PMDPage> m_pages;
  std::vector<PMDColor> m_color;
  std::vector<PMDFont> m_font;
  bool m_doubleSided;

  void writePage(const PMDPage &,
                 librevenge::RVNGDrawingInterface *,
                 const std::vector<boost::shared_ptr<const OutputShape> > &) const;

  void paintShape(const OutputShape &shape,
                  librevenge::RVNGDrawingInterface *) const;

  const PageShapesList_t getOutputShapesByPage_OneSided() const;
  const PageShapesList_t getOutputShapesByPage_TwoSided() const;
  const PageShapesList_t getOutputShapesByPage() const;
public:
  PMDCollector();

  /* State-mutating functions */
  void setPageWidth(PMDPageUnit);
  void setPageHeight(PMDPageUnit);
  void setDoubleSided(bool);
  void addShapeToPage(unsigned pageID, const boost::shared_ptr<PMDLineSet> &shape);
  void addColor(const PMDColor &color);
  void addFont(const PMDFont &font);

  unsigned addPage();

  /* Output functions */
  void draw(librevenge::RVNGDrawingInterface *) const;
};

}

#endif /* __PMDCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

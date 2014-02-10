#pragma once
#include <stdint.h>

#include <map>
#include <vector>
#include <string>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include "geometry.h"
#include "PMDPage.h"
#include "PMDExceptions.h"
#include "Units.h"
#include "OutputShape.h"

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
  /*
   * Height and width in PMD page units.
   * One PMD page unit is 1/20 of a point (1/720 inch)
   */
  boost::optional<PMDPageUnit> m_pageWidth;
  boost::optional<PMDPageUnit> m_pageHeight;

  std::vector<PMDPage> m_pages;
  bool m_doubleSided;

  void writePage(const PMDPage &,
                 librevenge::RVNGDrawingInterface *,
                 const std::vector<boost::shared_ptr<const OutputShape> > &) const;

  void paintShape(const OutputShape &shape,
                  librevenge::RVNGDrawingInterface *) const;

  std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > getOutputShapesByPage_OneSided() const;
  std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > getOutputShapesByPage_TwoSided() const;
  std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > getOutputShapesByPage() const;
public:
  PMDCollector();

  /* State-mutating functions */
  void setPageWidth(PMDPageUnit);
  void setPageHeight(PMDPageUnit);
  void addShapeToPage(unsigned pageID, boost::shared_ptr<PMDLineSet> shape);
  unsigned addPage();

  /* Output functions */
  void draw(librevenge::RVNGDrawingInterface *) const;
  void emitYaml(yaml_emitter_t *) const;
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

#pragma once
#include <stdint.h>

#include <vector>
#include <string>

#include <boost/optional.hpp>

#include "PMDPage.h"
#include "PMDExceptions.h"
#include "Units.h"

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

  void writePage(const PMDPage &page, librevenge::RVNGDrawingInterface *painter) const;

public:
  PMDCollector();

  /* State-mutating functions */
  void setPageWidth(PMDPageUnit);
  void setPageHeight(PMDPageUnit);
  unsigned addPage();

  /* Output functions */
  void draw(librevenge::RVNGDrawingInterface *) const;
  std::string getJsonRepresentation() const;
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

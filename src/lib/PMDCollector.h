#pragma once
#include <stdint.h>

#include <vector>
#include <string>

#include <boost/optional.hpp>

#include "PMDPage.h"
#include "PMDExceptions.h"

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
   * All units are in PMD internal format.
   * From what I've seen so far, one PMD unit is 1/20 of a point (1/720 inch)
   */
  boost::optional<uint16_t> m_pageWidth;
  boost::optional<uint16_t> m_pageHeight;

  std::vector<PMDPage> m_pages;

public:
  PMDCollector();

  /* State-mutating functions */
  void setPageWidth(int);
  void setPageHeight(int);
  void addPage();

  /* Output functions */
  void draw(librevenge::RVNGDrawingInterface *) const;
  std::string getJsonRepresentation() const;
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

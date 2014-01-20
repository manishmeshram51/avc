#include "PMDCollector.h"

namespace libpagemaker
{
PMDCollector::PMDCollector() :
  m_pageWidth(), m_pageHeight(), m_pages()
{ }

/* State-mutating functions */
void PMDCollector::setPageWidth(int pageWidth)
{
  m_pageWidth = pageWidth;
}

void PMDCollector::setPageHeight(int pageHeight)
{
  m_pageHeight = pageHeight;
}

void PMDCollector::addPage()
{
  m_pages.push_back( (PMDPage()) );
}

/* Output functions */
void PMDCollector::draw(librevenge::RVNGDrawingInterface* /* painter */) const
{
}

std::string PMDCollector::getJsonRepresentation() const
{
  return "{ }";
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

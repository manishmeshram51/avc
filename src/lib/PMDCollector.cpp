#include "PMDCollector.h"
#include <sstream>
#include <string>
#include <iostream>

#include "libpagemaker_utils.h"

namespace libpagemaker
{
PMDCollector::PMDCollector() :
  m_pageWidth(), m_pageHeight(), m_pages()
{ }

/* State-mutating functions */
void PMDCollector::setPageWidth(PMDPageUnit pageWidth)
{
  m_pageWidth = pageWidth;
}

void PMDCollector::setPageHeight(PMDPageUnit pageHeight)
{
  m_pageHeight = pageHeight;
}

unsigned PMDCollector::addPage()
{
  m_pages.push_back((PMDPage()));
}

void PMDCollector::writePage(const PMDPage & /*page*/, librevenge::RVNGDrawingInterface *painter) const
{
  librevenge::RVNGPropertyList pageProps;
  if (m_pageWidth.is_initialized())
  {
    double widthInInches = m_pageWidth.get().toInches();
    pageProps.insert("svg:width", widthInInches);
  }
  if (m_pageHeight.is_initialized())
  {
    double heightInInches = m_pageHeight.get().toInches();
    pageProps.insert("svg:height", heightInInches);
  }
  painter->startPage(pageProps);
  painter->endPage();
}

/* Output functions */
void PMDCollector::draw(librevenge::RVNGDrawingInterface *painter) const
{
  std::cout << "hi" << std::endl;
  painter->startDocument(librevenge::RVNGPropertyList());
  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    writePage(m_pages[i], painter);
  }
  painter->endDocument();
}

std::string PMDCollector::getJsonRepresentation() const
{
  std::ostringstream output;
  output << "{\n";
  output << "\tpages:\n\t[";
  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    output << "\n\t\t{\n";
    output << "\t\t\t\theight: " << m_pageHeight.get().m_value << ",\n";
    output << "\t\t\t\twidth: " << m_pageWidth.get().m_value << "\n\t\t}";
    if (i + 1 < m_pages.size())
    {
      output << ",";
    }
  }
  output << "\n\t]";
  output << "\n}";
  return output.str();
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

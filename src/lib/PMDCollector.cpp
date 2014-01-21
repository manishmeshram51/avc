#include "PMDCollector.h"
#include <sstream>
#include <string>
#include <iostream>

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
  std::cout << getJsonRepresentation() << std::endl;
}

std::string PMDCollector::getJsonRepresentation() const
{
  std::ostringstream output;
  output << "{\n";
  output << "\tpages:\n\t[";
  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    output << "\n\t\t{\n";
    output << "\t\t\t\theight: " << m_pageHeight.get() << ",\n";
    output << "\t\t\t\twidth: " << m_pageWidth.get() << "\n\t\t}";
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

#include "PMDCollector.h"
#include "OutputShape.h"
#include <sstream>
#include <string>
#include <iostream>
#include <math.h>

#include "libpagemaker_utils.h"

namespace libpagemaker
{
PMDCollector::PMDCollector() :
  m_pageWidth(), m_pageHeight(), m_pages(),
  m_doubleSided(false)
{ }

void PMDCollector::setDoubleSided(bool doubleSided)
{
  m_doubleSided = doubleSided;
}

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
  return m_pages.size() - 1;
}

void PMDCollector::addShapeToPage(unsigned pageID, boost::shared_ptr<PMDLineSet> shape)
{
  m_pages.at(pageID).addShape(shape);
}

void PMDCollector::paintShape(const OutputShape &shape,
                              librevenge::RVNGDrawingInterface *painter) const
{
  if (shape.shapeTypePolygon())
  {
    librevenge::RVNGPropertyListVector vertices;
    for (unsigned i = 0; i < shape.numPoints(); ++i)
    {
      librevenge::RVNGPropertyList vertex;
      vertex.insert("svg:x", shape.getPoint(i).m_x);
      vertex.insert("svg:y", shape.getPoint(i).m_y);
      vertices.append(vertex);
    }
    librevenge::RVNGPropertyList points;
    points.insert("svg:points", vertices);
    if (shape.getIsClosed())
    {
      painter->drawPolygon(points);
    }
    else
    {
      painter->drawPolyline(points);
    }
  }
  else
  {
    double cx = shape.getPoint(0).m_x;
    double cy = shape.getPoint(0).m_y;
    double rx = shape.getPoint(1).m_x;
    double ry = shape.getPoint(1).m_y;

    double rotation = shape.getRotation();

    PMD_DEBUG_MSG(("\n\nCx and Cy are %f , %f \n",cx,cy));
    PMD_DEBUG_MSG(("Rx and Ry are %f , %f \n",rx,ry));
    PMD_DEBUG_MSG(("Rotation is %f \n",rotation));
    librevenge::RVNGPropertyList propList;

    if (false)
    {
      propList.insert("svg:rx",rx);
      propList.insert("svg:ry",ry);
      propList.insert("svg:cx",cx);
      propList.insert("svg:cy",cy);
      painter->drawEllipse(propList);
    }
    else
    {
      double sx = cx - rx;
      double sy = cy;
      double ex = cx + rx;
      double ey = cy;
      if (rotation)
      {
       sx = cx - rx*cos(rotation)/2;
       sy = cy - ry*sin(rotation)/2;

       ex = cx + rx*cos(rotation)/2;
       ey = cy + ry*sin(rotation)/2;
      }
      PMD_DEBUG_MSG(("Sx and Sy are %f , %f \n",sx,sy));
      PMD_DEBUG_MSG(("Ex and Ey are %f , %f \n",ex,ey));

      librevenge::RVNGPropertyListVector vec;
      librevenge::RVNGPropertyList node;

      node.insert("librevenge:path-action", "M");
      node.insert("svg:x", sx);
      node.insert("svg:y", sy);
      vec.append(node);

      node.clear();
      node.insert("librevenge:path-action", "A");
      node.insert("svg:rx", rx);
      node.insert("svg:ry", ry);
      node.insert("librevenge:rotate", rotation * 180 / M_PI, librevenge::RVNG_GENERIC);
      node.insert("librevenge:large-arc", false);
      node.insert("librevenge:sweep", false);
      node.insert("svg:x", ex);
      node.insert("svg:y", ey);
      vec.append(node);

      node.clear();
      node.insert("librevenge:path-action", "A");
      node.insert("svg:rx", rx);
      node.insert("svg:ry", ry);
      node.insert("librevenge:rotate", rotation * 180 / M_PI, librevenge::RVNG_GENERIC);
      node.insert("librevenge:large-arc", true);
      node.insert("librevenge:sweep", false);
      node.insert("svg:x", sx);
      node.insert("svg:y", sy);
      vec.append(node);

      node.clear();
      node.insert("librevenge:path-action", "Z");
      vec.append(node);

      propList.insert("svg:d",vec);
      painter->drawPath(propList);
    }
  }
}



void PMDCollector::writePage(const PMDPage & /*page*/,
                             librevenge::RVNGDrawingInterface *painter,
                             const std::vector<boost::shared_ptr<const OutputShape> > &outputShapes) const
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
  for (unsigned i = 0; i < outputShapes.size(); ++i)
  {
    paintShape(*(outputShapes[i]), painter);
  }
  painter->endPage();
}

std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > PMDCollector::getOutputShapesByPage_TwoSided()
const
{
  std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > toReturn;
  double centerToEdge_x = m_pageWidth.get().toInches() / 2;
  double centerToEdge_y = m_pageHeight.get().toInches() / 2;
  InchPoint translateForLeftPage(centerToEdge_x * 2, centerToEdge_y);
  InchPoint translateForRightPage(0, centerToEdge_y);
  /* Iterate over the right-sided pages. */
  for (unsigned i = 0; i < m_pages.size(); i += 2)
  {
    const PMDPage &page = m_pages[i];
    for (unsigned j = 0; j < page.numShapes(); ++j)
    {
      bool leftPageExists = (i > 0);

      boost::shared_ptr<const OutputShape> right = newOutputShape(page.getShape(j), translateForRightPage);
      if (right->getBoundingBox().second.m_x >= 0)
      {
        toReturn[i].push_back(right);
      }
      if (leftPageExists)
      {
        boost::shared_ptr<const OutputShape> left = newOutputShape(page.getShape(j), translateForLeftPage);
        if (left->getBoundingBox().first.m_x <= centerToEdge_x * 2)
        {
          toReturn[i - 1].push_back(left);
        }
      }
    }
  }
  return toReturn;
}

std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > PMDCollector::getOutputShapesByPage_OneSided()
const
{
  std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > toReturn;
  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    const PMDPage &page = m_pages[i];
    for (unsigned j = 0; j < page.numShapes(); ++j)
    {
      toReturn[i].push_back(newOutputShape(page.getShape(j), InchPoint(0, 0)));
    }
  }
  return toReturn;
}

std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > PMDCollector::getOutputShapesByPage()
const
{
  return m_doubleSided ?
         getOutputShapesByPage_TwoSided()
         : getOutputShapesByPage_OneSided();
}

/* Output functions */
void PMDCollector::draw(librevenge::RVNGDrawingInterface *painter) const
{
  painter->startDocument(librevenge::RVNGPropertyList());

  std::map<unsigned, std::vector<boost::shared_ptr<const OutputShape> > > shapesByPage
    = getOutputShapesByPage();
  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    std::vector<boost::shared_ptr<const OutputShape> > shapes = shapesByPage[i];
    writePage(m_pages[i], painter, shapes);
  }
  painter->endDocument();
}

void PMDCollector::emitYaml(yaml_emitter_t *emitter) const
{
  yamlBeginMap(emitter);
  if (m_pageWidth.is_initialized())
  {
    yamlMapObject(emitter, "pageWidth", m_pageWidth.get());
  }
  if (m_pageHeight.is_initialized())
  {
    yamlMapObject(emitter, "pageHeight", m_pageHeight.get());
  }
  yamlMapEntry(emitter, "doubleSided", m_doubleSided);
  yamlForeach(emitter, "pages", m_pages);
  yamlEndMap(emitter);
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

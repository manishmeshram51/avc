/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "PMDCollector.h"
#include "OutputShape.h"
#include <sstream>
#include <string>
#include <iostream>
#include <math.h>
#include "constants.h"
#include "libpagemaker_utils.h"

namespace libpagemaker
{

static const double EM2PT = 11.95516799999881;

PMDCollector::PMDCollector() :
  m_pageWidth(), m_pageHeight(), m_pages(), m_color(),m_font(),
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

void PMDCollector::addColor(const PMDColor &color)
{
  m_color.push_back(color);
}

void PMDCollector::addFont(const PMDFont &font)
{
  m_font.push_back(font);
}

void PMDCollector::addShapeToPage(unsigned pageID, const boost::shared_ptr<PMDLineSet> &shape)
{
  m_pages.at(pageID).addShape(shape);
}

void PMDCollector::paintShape(const OutputShape &shape,
                              librevenge::RVNGDrawingInterface *painter) const
{
  if (shape.shapeType() == SHAPE_TYPE_LINE || shape.shapeType() == SHAPE_TYPE_POLY || shape.shapeType() == SHAPE_TYPE_RECT)
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
    uint8_t fillType = shape.getFillType();
    uint8_t fillColor = shape.getFillColor();
    //uint8_t fillOverprint = shape.getFillOverprint();
    uint8_t fillTint = shape.getFillTint();

    uint8_t strokeType = shape.getStrokeType();
    double strokeWidth = shape.getStrokeWidth();
    strokeWidth = strokeWidth/360;
    uint8_t strokeColor = shape.getStrokeColor();
    //uint8_t strokeOverprint = shape.getStrokeOverprint();
    uint8_t strokeTint = shape.getStrokeTint();

    switch (fillType)
    {
    case FILL_SOLID:
      points.insert("draw:fill", "solid");
      break;
    case FILL_NONE:
      points.insert("draw:fill", "none");
      break;
    default:
      points.insert("draw:fill", "none");
    }

    if (fillColor < m_color.size())
    {
      PMDColor tempFillColor = m_color[fillColor];
      librevenge::RVNGString tempFillColorString;
      tempFillColorString.sprintf("#%.2x%.2x%.2x", tempFillColor.m_red,tempFillColor.m_green,tempFillColor.m_blue);
      points.insert("draw:fill-color", tempFillColorString);
    }
    else
    {
      PMD_DEBUG_MSG(("Fill Color Not Available"));
    }

    if (fillColor == 0)
      points.insert("draw:opacity", 0);
    else
      points.insert("draw:opacity", fillTint);

    switch (strokeType)
    {
    case STROKE_NORMAL:
      points.insert("draw:stroke", "solid");
      break;
    case STROKE_DASHED:
      points.insert("draw:stroke","dash");
      break;
    default:
      points.insert("draw:stroke", "solid");
    }

    points.insert("svg:stroke-width", strokeWidth);

    if (strokeColor < m_color.size())
    {
      PMDColor tempStrokeColor = m_color[strokeColor];
      librevenge::RVNGString tempStrokeColorString;
      tempStrokeColorString.sprintf("#%.2x%.2x%.2x", tempStrokeColor.m_red,tempStrokeColor.m_green,tempStrokeColor.m_blue);
      points.insert("svg:stroke-color", tempStrokeColorString);
    }
    else
    {
      PMD_DEBUG_MSG(("Stroke Color Not Available"));
    }

    points.insert("svg:stroke-opacity", strokeTint);

    if (shape.getIsClosed())
    {
      painter->drawPolygon(points);
    }
    else
    {
      painter->drawPolyline(points);
    }
  }
  else if (shape.shapeType() == SHAPE_TYPE_TEXTBOX)
  {
    librevenge::RVNGPropertyList textbox;

    textbox.insert("svg:x",shape.getPoint(0).m_x, librevenge::RVNG_INCH);
    textbox.insert("svg:y",shape.getPoint(0).m_y, librevenge::RVNG_INCH);
    textbox.insert("svg:width",shape.getPoint(1).m_x, librevenge::RVNG_INCH);
    textbox.insert("svg:height",shape.getPoint(1).m_y, librevenge::RVNG_INCH);
    //textbox.insert("text:anchor-type", "page");
    //textbox.insert("text:anchor-page-number", 1);
    //textbox.insert("style:vertical-rel", "page");
    //textbox.insert("style:horizontal-rel", "page");
    //textbox.insert("style:horizontal-pos", "from-left");
    //textbox.insert("style:vertical-pos", "from-top");
    textbox.insert("draw:stroke", "none");
    textbox.insert("draw:fill", "none");
    textbox.insert("librevenge:rotate", shape.getRotation() * 180 / M_PI);

    painter->startTextObject(textbox);

    uint16_t paraStart = 0;
    uint16_t paraEnd = 0;
    uint16_t paraLength = 0;

    std::vector<PMDParaProperties> paraProperties = shape.getParaProperties();

    for (unsigned p = 0; p < paraProperties.size(); ++p)
    {

      paraLength = paraProperties[p].m_length;
      paraEnd = paraStart + paraLength - 1;

      librevenge::RVNGPropertyList paraProps;

      switch (paraProperties[p].m_align)
      {
      case 1:
        paraProps.insert("fo:text-align", "right");
        break;
      case 2:
        paraProps.insert("fo:text-align", "center");
        break;
      case 3:
        paraProps.insert("fo:text-align", "justify");
        break;
      case 4: // force-justify
        // Strictly speaking, this is not equivalent to the real force-justify
        // layout. But it is the best approximation ODF can do.
        paraProps.insert("fo:text-align", "justify");
        paraProps.insert("fo:text-align-last", "justify");
        break;
      case 0:
      default:
        paraProps.insert("fo:text-align", "left");
        break;
      }

      if (paraProperties[p].m_afterIndent != 0)
      {
        paraProps.insert("fo:margin-bottom", (double)paraProperties[p].m_afterIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperties[p].m_beforeIndent != 0)
      {
        paraProps.insert("fo:margin-top", (double)paraProperties[p].m_beforeIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperties[p].m_firstIndent != 0)
      {
        paraProps.insert("fo:text-indent", (double)paraProperties[p].m_firstIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperties[p].m_leftIndent != 0)
      {
        paraProps.insert("fo:margin-left", (double)paraProperties[p].m_leftIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperties[p].m_rightIndent != 0)
      {
        paraProps.insert("fo:margin-right", (double)paraProperties[p].m_rightIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }

      painter->openParagraph(paraProps);
      PMD_DEBUG_MSG(("\n\nPara Start is %d \n",paraStart));
      PMD_DEBUG_MSG(("Para End is %d \n\n",paraEnd));

      //charProps.insert("style:font-name", "Ubuntu");

      std::string tempText = shape.getText();
      std::vector<PMDCharProperties> charProperties = shape.getCharProperties();

      uint16_t charStart = 0;
      uint16_t charEnd = 0;
      uint16_t charLength = 0;

      uint16_t j = 0;
      bool capsFlag = false;


      for (unsigned i = 0; i < charProperties.size(); ++i)
      {
        charLength = charProperties[i].m_length;
        uint16_t charEndTemp = charStart + charLength -1;

        if (paraStart > charStart)
          charStart = paraStart;

        if (charEndTemp > paraEnd)
          charEnd = paraEnd;
        else
          charEnd = charEndTemp;

        if (charStart <= charEnd && paraStart <= charEndTemp)
        {
          PMD_DEBUG_MSG(("Start is %d \n",charStart));
          PMD_DEBUG_MSG(("End is %d \n",charEnd));

          librevenge::RVNGPropertyList charProps;
          charProps.insert("fo:font-size",(double)charProperties[i].m_fontSize/10,librevenge::RVNG_POINT);

          if (charProperties[i].m_fontFace < m_font.size())
          {
            PMDFont tempFont = m_font[charProperties[i].m_fontFace];
            std::string tempFontString = tempFont.m_fontName;
            charProps.insert("style:font-name", tempFontString.c_str());
          }
          else
          {
            PMD_DEBUG_MSG(("Font Not Available"));
          }

          if (charProperties[i].m_fontColor < m_color.size())
          {
            PMDColor tempColor = m_color[charProperties[i].m_fontColor];
            librevenge::RVNGString tempColorString;
            tempColorString.sprintf("#%.2x%.2x%.2x", tempColor.m_red,tempColor.m_green,tempColor.m_blue);
            charProps.insert("fo:color", tempColorString);
          }
          else
          {
            PMD_DEBUG_MSG(("Color Not Available"));
          }

          switch (charProperties[i].m_boldItalicUnderline)
          {
          case 1:
            charProps.insert("fo:font-weight", "bold");
            break;
          case 2:
            charProps.insert("fo:font-style", "italic");
            break;
          case 3:
            charProps.insert("fo:font-weight", "bold");
            charProps.insert("fo:font-style", "italic");
            break;
          case 4:
            charProps.insert("style:text-underline-type", "single");
            break;
          case 5:
            charProps.insert("fo:font-weight", "bold");
            charProps.insert("style:text-underline-type", "single");
            break;
          case 6:
            charProps.insert("fo:font-style", "italic");
            charProps.insert("style:text-underline-type", "single");
            break;
          case 7:
            charProps.insert("fo:font-weight", "bold");
            charProps.insert("fo:font-style", "italic");
            charProps.insert("style:text-underline-type", "single");
            break;
          default:
            break;
          }

          librevenge::RVNGString superscriptPosSizeString = "";
          librevenge::RVNGString subscriptPosSizeString = "";
          double superPos = (double)charProperties[i].m_superPos/10;
          double subPos = (double)charProperties[i].m_subPos/10;
          double superSubSize = (double)charProperties[i].m_superSubSize/10;

          std::ostringstream sstream;
          sstream << superSubSize;
          std::string varAsString = sstream.str();

          std::ostringstream sstream1;
          sstream1 << superPos;
          std::string varAsString1 = sstream1.str();

          std::ostringstream sstream2;
          sstream2 << subPos;
          std::string varAsString2 = sstream2.str();

          superscriptPosSizeString.append(varAsString1.c_str());
          superscriptPosSizeString.append("% ");
          superscriptPosSizeString.append(varAsString.c_str());
          superscriptPosSizeString.append("%");

          subscriptPosSizeString.append("-");
          subscriptPosSizeString.append(varAsString2.c_str());
          subscriptPosSizeString.append("% ");
          subscriptPosSizeString.append(varAsString.c_str());
          subscriptPosSizeString.append("%");

          switch (charProperties[i].m_superSubscript)
          {
          case 1:
            charProps.insert("style:text-line-through-style","solid");
            break;
          case 2:
            charProps.insert("style:text-position", superscriptPosSizeString);
            break;
          case 3:
            charProps.insert("style:text-line-through-style","solid");
            charProps.insert("style:text-position", superscriptPosSizeString);
            break;
          case 4:
            charProps.insert("style:text-position", subscriptPosSizeString);
            break;
          case 5:
            charProps.insert("style:text-line-through-style","solid");
            charProps.insert("style:text-position", subscriptPosSizeString);
            break;
          case 8: // Small Caps
            capsFlag = true;
            charProps.insert("fo:font-variant","small-caps"); // Present in Open Document Schema Central but not working
            break;
          case 9:
            capsFlag = true;
            charProps.insert("fo:font-variant","small-caps");
            charProps.insert("style:text-line-through-style","solid");
            break;
          case 0x0a:
            capsFlag = true;
            charProps.insert("fo:font-variant","small-caps");
            charProps.insert("style:text-position", superscriptPosSizeString);
            break;
          case 0x0b:
            capsFlag = true;
            charProps.insert("fo:font-variant","small-caps");
            charProps.insert("style:text-line-through-style","solid");
            charProps.insert("style:text-position", superscriptPosSizeString);
            break;
          case 0x0c:
            capsFlag = true;
            charProps.insert("fo:font-variant","small-caps");
            charProps.insert("style:text-position", subscriptPosSizeString);
            break;
          case 0x0d:
            capsFlag = true;
            charProps.insert("fo:font-variant","small-caps");
            charProps.insert("style:text-line-through-style","solid");
            charProps.insert("style:text-position", subscriptPosSizeString);
            break;
          case 0x10: // Large Caps
            capsFlag = true;
            break;
          case 0x11:
            capsFlag = true;
            charProps.insert("style:text-line-through-style","solid");
            break;
          case 0x12:
            capsFlag = true;
            charProps.insert("style:text-position", superscriptPosSizeString);
            break;
          case 0x13:
            capsFlag = true;
            charProps.insert("style:text-line-through-style","solid");
            charProps.insert("style:text-position", superscriptPosSizeString);
            break;
          case 0x14:
            capsFlag = true;
            charProps.insert("style:text-position", subscriptPosSizeString);
            break;
          case 0x15:
            capsFlag = true;
            charProps.insert("style:text-line-through-style","solid");
            charProps.insert("style:text-position", subscriptPosSizeString);
            break;
          default:
            capsFlag = false;
            break;
          }

          if (charProperties[i].m_kerning != 0)
          {
            charProps.insert("style:letter-kerning","true");
            charProps.insert("fo:letter-spacing",((double)charProperties[i].m_kerning/1000)*EM2PT,librevenge::RVNG_POINT);
          }


          painter->openSpan(charProps);

          for (j=charStart; j<=charEnd; j++)
          {
            std::string tempStr;
            if (capsFlag && tempText[j] >=97 && tempText[j] <=122)
            {
              tempStr.push_back(tempText[j] - 32);
              painter->insertText(tempStr.c_str());
            }
            else if (tempText[j] == ' ')
            {
              painter->insertSpace();
            }
            else if (tempText[j] == '\t')
            {
              painter->insertTab();
            }
            else if (tempText[j] == '\r')
            {
              painter->insertLineBreak();
            }
            else
            {
              tempStr.push_back(tempText[j]);
              painter->insertText(tempStr.c_str());
            }
          }

          charStart = j;
          painter->closeSpan();
        }
        else
          charStart = charEnd + 1;
      }

      painter->closeParagraph();

      paraStart = paraEnd + 1;

    }
    painter->endTextObject();
  }
  else
  {
    double cx = shape.getPoint(0).m_x;
    double cy = shape.getPoint(0).m_y;
    double rx = shape.getPoint(1).m_x;
    double ry = shape.getPoint(1).m_y;

    double rotation = shape.getRotation();
#ifdef DEBUG
    double skew = shape.getSkew();
#endif

    PMD_DEBUG_MSG(("\n\nCx and Cy are %f , %f \n",cx,cy));
    PMD_DEBUG_MSG(("Rx and Ry are %f , %f \n",rx,ry));
    PMD_DEBUG_MSG(("Rotation is %f \n",rotation));
    PMD_DEBUG_MSG(("Skew is %f \n",skew));
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
      double sx = cx - rx*cos(rotation);
      double sy = cy - rx*sin(rotation);

      double ex = cx + rx*cos(rotation);
      double ey = cy + rx*sin(rotation);

      //if ((rotation == 0 || rotation < skew) && skew != 0)
      //rotation += (ry*skew/rx)/2;

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

      uint8_t fillType = shape.getFillType();
      uint8_t fillColor = shape.getFillColor();
      //uint8_t fillOverprint = shape.getFillOverprint();
      uint8_t fillTint = shape.getFillTint();

      uint8_t strokeType = shape.getStrokeType();
      double strokeWidth = shape.getStrokeWidth();
      strokeWidth = strokeWidth/360;
      uint8_t strokeColor = shape.getStrokeColor();
      //uint8_t strokeOverprint = shape.getStrokeOverprint();
      uint8_t strokeTint = shape.getStrokeTint();

      switch (fillType)
      {
      case FILL_SOLID:
        propList.insert("draw:fill", "solid");
        break;
      case FILL_NONE:
        propList.insert("draw:fill", "none");
        break;
      default:
        propList.insert("draw:fill", "none");
      }

      if (fillColor < m_color.size())
      {
        PMDColor tempFillColor = m_color[fillColor];
        librevenge::RVNGString tempFillColorString;
        tempFillColorString.sprintf("#%.2x%.2x%.2x", tempFillColor.m_red,tempFillColor.m_green,tempFillColor.m_blue);
        propList.insert("draw:fill-color", tempFillColorString);
      }
      else
      {
        PMD_DEBUG_MSG(("Fill Color Not Available"));
      }

      if (fillColor == 0)
        propList.insert("draw:opacity", 0);
      else
        propList.insert("draw:opacity", fillTint);

      switch (strokeType)
      {
      case STROKE_NORMAL:
        propList.insert("draw:stroke", "solid");
        break;
      case STROKE_DASHED:
        propList.insert("draw:stroke","dash");
        break;
      default:
        propList.insert("draw:stroke", "solid");
      }

      propList.insert("svg:stroke-width", strokeWidth);

      if (strokeColor < m_color.size())
      {
        PMDColor tempStrokeColor = m_color[strokeColor];
        librevenge::RVNGString tempStrokeColorString;
        tempStrokeColorString.sprintf("#%.2x%.2x%.2x", tempStrokeColor.m_red,tempStrokeColor.m_green,tempStrokeColor.m_blue);
        propList.insert("svg:stroke-color", tempStrokeColorString);
      }
      else
      {
        PMD_DEBUG_MSG(("Stroke Color Not Available"));
      }

      propList.insert("svg:stroke-opacity", strokeTint);

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

const PMDCollector::PageShapesList_t PMDCollector::getOutputShapesByPage_TwoSided() const
{
  PageShapesList_t toReturn(m_pages.size() * 2 - 1); // the first "page" only has right side
  double centerToEdge_x = m_pageWidth.get().toInches() / 2;
  double centerToEdge_y = m_pageHeight.get().toInches() / 2;
  InchPoint translateForLeftPage(centerToEdge_x * 2, centerToEdge_y);
  InchPoint translateForRightPage(0, centerToEdge_y);

  for (unsigned i = 0, pageNum = 0; i < m_pages.size(); ++i, pageNum += 2)
  {
    const bool leftPageExists = (pageNum > 0);

    const PMDPage &page = m_pages[i];
    for (unsigned j = 0; j < page.numShapes(); ++j)
    {
      boost::shared_ptr<const OutputShape> right = newOutputShape(page.getShape(j), translateForRightPage);
      if (right->getBoundingBox().second.m_x >= 0)
      {
        toReturn[pageNum].push_back(right);
      }
      if (leftPageExists)
      {
        boost::shared_ptr<const OutputShape> left = newOutputShape(page.getShape(j), translateForLeftPage);
        if (left->getBoundingBox().first.m_x <= centerToEdge_x * 2)
        {
          toReturn[pageNum - 1].push_back(left);
        }
      }
    }
  }

  if (toReturn.back().empty()) // the last "page" only has left side
    toReturn.pop_back();

  return toReturn;
}

const PMDCollector::PageShapesList_t PMDCollector::getOutputShapesByPage_OneSided() const
{
  PageShapesList_t toReturn;
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

const PMDCollector::PageShapesList_t PMDCollector::getOutputShapesByPage() const
{
  return m_doubleSided ? getOutputShapesByPage_TwoSided() : getOutputShapesByPage_OneSided();
}

/* Output functions */
void PMDCollector::draw(librevenge::RVNGDrawingInterface *painter) const
{
  painter->startDocument(librevenge::RVNGPropertyList());

  PageShapesList_t shapesByPage = getOutputShapesByPage();
  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    const PageShapes_t &shapes = shapesByPage[i];
    writePage(m_pages[i], painter, shapes);
  }
  painter->endDocument();
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

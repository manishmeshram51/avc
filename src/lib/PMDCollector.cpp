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
#include <string>
#include <iostream>
#include <math.h>
#include "constants.h"
#include "libpagemaker_utils.h"

namespace libpagemaker
{

static const double EM2PT = 11.95516799999881;

namespace
{

void flushText(std::string &text, librevenge::RVNGDrawingInterface *const painter)
{
  if (!text.empty())
  {
    painter->insertText(text.c_str());
    text.clear();
  }
}

void writeTextSpan(const std::string &text, const std::size_t charStart, std::size_t charEnd, librevenge::RVNGDrawingInterface *const painter)
{
  ++charEnd;
  if (charEnd > text.size())
    charEnd = text.size();

  std::string currentText;
  bool wasSpace = false;
  for (std::size_t i = charStart; i < charEnd; ++i)
  {
    const char c = text[i];

    switch (c)
    {
    case '\t' :
      flushText(currentText, painter);
      painter->insertTab();
      break;
    case '\r' :
      flushText(currentText, painter);
      painter->insertLineBreak();
      break;
    case ' ' :
      if (wasSpace)
      {
        flushText(currentText, painter);
        painter->insertSpace();
      }
      else
      {
        currentText.push_back(c);
      }
      break;
    default:
      // Ignore control characters that do not have known use in PageMaker.
      // Specific control characters are handled in the switch already.
      if (c < 0x20)
      {
        PMD_DEBUG_MSG(("skipping control character %#x\n", c));
        break;
      }
      else
      {
        currentText.push_back(c);
      }
    }

    wasSpace = ' ' == c;
  }

  flushText(currentText, painter);
}

}

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

void PMDCollector::addShapeToPage(unsigned pageID, const std::shared_ptr<PMDLineSet> &shape)
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

    PMDFillProperties fillProps = shape.getFillProperties();
    PMDStrokeProperties strokeProps = shape.getStrokeProperties();

    switch (fillProps.m_fillType)
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

    if (fillProps.m_fillColor < m_color.size())
    {
      PMDColor tempFillColor = m_color[fillProps.m_fillColor];
      librevenge::RVNGString tempFillColorString;
      tempFillColorString.sprintf("#%.2x%.2x%.2x", tempFillColor.m_red,tempFillColor.m_green,tempFillColor.m_blue);
      points.insert("draw:fill-color", tempFillColorString);
    }
    else
    {
      PMD_DEBUG_MSG(("Fill Color Not Available"));
    }

    if (fillProps.m_fillColor == 0)
      points.insert("draw:opacity", 0);
    else
      points.insert("draw:opacity", fillProps.m_fillTint);

    switch (strokeProps.m_strokeType)
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

    points.insert("svg:stroke-width", (double)strokeProps.m_strokeWidth/5.0,librevenge::RVNG_POINT);

    if (strokeProps.m_strokeColor < m_color.size())
    {
      PMDColor tempStrokeColor = m_color[strokeProps.m_strokeColor];
      librevenge::RVNGString tempStrokeColorString;
      tempStrokeColorString.sprintf("#%.2x%.2x%.2x", tempStrokeColor.m_red,tempStrokeColor.m_green,tempStrokeColor.m_blue);
      points.insert("svg:stroke-color", tempStrokeColorString);
    }
    else
    {
      PMD_DEBUG_MSG(("Stroke Color Not Available"));
    }

    points.insert("svg:stroke-opacity", (double)strokeProps.m_strokeTint/100.0,librevenge::RVNG_PERCENT);

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
    textbox.insert("svg:width",shape.getWidth(), librevenge::RVNG_INCH);
    textbox.insert("svg:height",shape.getHeight(), librevenge::RVNG_INCH);
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

    for (auto &paraProperty : paraProperties)
    {

      paraLength = paraProperty.m_length;
      paraEnd = paraStart + paraLength - 1;

      librevenge::RVNGPropertyList paraProps;

      switch (paraProperty.m_align)
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

      if (paraProperty.m_afterIndent != 0)
      {
        paraProps.insert("fo:margin-bottom", (double)paraProperty.m_afterIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperty.m_beforeIndent != 0)
      {
        paraProps.insert("fo:margin-top", (double)paraProperty.m_beforeIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperty.m_firstIndent != 0)
      {
        paraProps.insert("fo:text-indent", (double)paraProperty.m_firstIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperty.m_leftIndent != 0)
      {
        paraProps.insert("fo:margin-left", (double)paraProperty.m_leftIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
      }
      if (paraProperty.m_rightIndent != 0)
      {
        paraProps.insert("fo:margin-right", (double)paraProperty.m_rightIndent/SHAPE_UNITS_PER_INCH,librevenge::RVNG_INCH);
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

      for (auto &charProperty : charProperties)
      {
        charLength = charProperty.m_length;
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
          charProps.insert("fo:font-size",(double)charProperty.m_fontSize/10,librevenge::RVNG_POINT);

          if (charProperty.m_fontFace < m_font.size())
          {
            PMDFont tempFont = m_font[charProperty.m_fontFace];
            std::string tempFontString = tempFont.m_fontName;
            charProps.insert("style:font-name", tempFontString.c_str());
          }
          else
          {
            PMD_DEBUG_MSG(("Font Not Available"));
          }

          if (charProperty.m_fontColor < m_color.size())
          {
            PMDColor tempColor = m_color[charProperty.m_fontColor];
            double charTint = (double)charProperty.m_tint/100;
            double temp_bgcolor = (1 - charTint) * 255;
            librevenge::RVNGString tempColorString;
            tempColorString.sprintf("#%.2x%.2x%.2x",(uint16_t)(tempColor.m_red * charTint + temp_bgcolor),(uint16_t)(tempColor.m_green * charTint + temp_bgcolor),(uint16_t)(tempColor.m_blue * charTint + temp_bgcolor));
            charProps.insert("fo:color", tempColorString);
          }
          else
          {
            PMD_DEBUG_MSG(("Color Not Available"));
          }

          if (charProperty.m_bold)
            charProps.insert("fo:font-weight", "bold");
          if (charProperty.m_italic)
            charProps.insert("fo:font-style", "italic");
          if (charProperty.m_underline)
            charProps.insert("style:text-underline-type", "single");
          if (charProperty.m_outline)
            charProps.insert("style:text-outline", true);
          if (charProperty.m_shadow)
            charProps.insert("fo:text-shadow", "1pt 1pt");

          if (charProperty.m_strike)
            charProps.insert("style:text-line-through-style","solid");
          if (charProperty.m_super || charProperty.m_sub)
          {
            const int32_t intPos = charProperty.m_sub ? -int32_t(charProperty.m_subPos) : int32_t(charProperty.m_superPos);
            librevenge::RVNGString pos;
            pos.sprintf("%.1f%% %.1f%%", intPos / 10.0, charProperty.m_superSubSize / 10.0);
            charProps.insert("style:text-position", pos);
          }

          if (charProperty.m_smallCaps)
            charProps.insert("fo:font-variant","small-caps");
          if (charProperty.m_allCaps)
            charProps.insert("fo:text-transform", "capitalize");

          if (charProperty.m_kerning != 0)
          {
            charProps.insert("style:letter-kerning","true");
            charProps.insert("fo:letter-spacing",((double)charProperty.m_kerning/1000)*EM2PT,librevenge::RVNG_POINT);
          }


          painter->openSpan(charProps);
          writeTextSpan(tempText, charStart, charEnd, painter);
          painter->closeSpan();
        }

        charStart = charEnd + 1;
      }

      painter->closeParagraph();

      paraStart = paraEnd + 1;

    }
    painter->endTextObject();
  }
  else if (shape.shapeType() == SHAPE_TYPE_BITMAP)
  {
    librevenge::RVNGPropertyList props;
    props.insert("svg:x", shape.getPoint(0).m_x,librevenge::RVNG_INCH);
    props.insert("svg:y", shape.getPoint(0).m_y,librevenge::RVNG_INCH);
    props.insert("svg:width", shape.getWidth(),librevenge::RVNG_INCH);
    props.insert("svg:height", shape.getHeight(),librevenge::RVNG_INCH);

    if (shape.getRotation() != 0.0)
      props.insert("librevenge:rotate", shape.getRotation() * 180 / M_PI, librevenge::RVNG_GENERIC);

    props.insert("librevenge:mime-type", "image/tiff");
    props.insert("office:binary-data", shape.getBitmap());
    painter->drawGraphicObject(props);
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

      PMDFillProperties fillProps = shape.getFillProperties();
      PMDStrokeProperties strokeProps = shape.getStrokeProperties();

      switch (fillProps.m_fillType)
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

      if (fillProps.m_fillColor < m_color.size())
      {
        PMDColor tempFillColor = m_color[fillProps.m_fillColor];
        librevenge::RVNGString tempFillColorString;
        tempFillColorString.sprintf("#%.2x%.2x%.2x", tempFillColor.m_red,tempFillColor.m_green,tempFillColor.m_blue);
        propList.insert("draw:fill-color", tempFillColorString);
      }
      else
      {
        PMD_DEBUG_MSG(("Fill Color Not Available"));
      }

      if (fillProps.m_fillColor == 0)
        propList.insert("draw:opacity", 0);
      else
        propList.insert("draw:opacity", fillProps.m_fillTint);

      switch (strokeProps.m_strokeType)
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

      propList.insert("svg:stroke-width", (double)strokeProps.m_strokeWidth/5.0,librevenge::RVNG_POINT);

      if (strokeProps.m_strokeColor < m_color.size())
      {
        PMDColor tempStrokeColor = m_color[strokeProps.m_strokeColor];
        librevenge::RVNGString tempStrokeColorString;
        tempStrokeColorString.sprintf("#%.2x%.2x%.2x", tempStrokeColor.m_red,tempStrokeColor.m_green,tempStrokeColor.m_blue);
        propList.insert("svg:stroke-color", tempStrokeColorString);
      }
      else
      {
        PMD_DEBUG_MSG(("Stroke Color Not Available"));
      }

      propList.insert("svg:stroke-opacity", (double)strokeProps.m_strokeTint/100.0,librevenge::RVNG_PERCENT);

      painter->drawPath(propList);
    }
  }
}



void PMDCollector::writePage(const PMDPage & /*page*/,
                             librevenge::RVNGDrawingInterface *painter,
                             const std::vector<std::shared_ptr<const OutputShape> > &outputShapes) const
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
  for (const auto &outputShape : outputShapes)
  {
    paintShape(*outputShape, painter);
  }
  painter->endPage();
}

void PMDCollector::fillOutputShapesByPage_TwoSided(PageShapesList_t &pageShapes) const
{
  pageShapes.assign(m_pages.size() * 2 - 1, PageShapes_t()); // the first "page" only has right side

  double centerToEdge_x = m_pageWidth.get_value_or(0).toInches() / 2;
  double centerToEdge_y = m_pageHeight.get_value_or(0).toInches() / 2;
  InchPoint translateForLeftPage(centerToEdge_x * 2, centerToEdge_y);
  InchPoint translateForRightPage(0, centerToEdge_y);

  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    const bool leftPageExists = (i > 0);

    const PMDPage &page = m_pages[i];
    for (unsigned j = 0; j < page.numShapes(); ++j)
    {
      std::shared_ptr<const OutputShape> right = newOutputShape(page.getShape(j), translateForRightPage);
      if (right->getBoundingBox().second.m_x >= 0)
      {
        pageShapes[i].push_back(right);
        continue;
      }
      if (leftPageExists)
      {
        std::shared_ptr<const OutputShape> left = newOutputShape(page.getShape(j), translateForLeftPage);
        if (left->getBoundingBox().first.m_x <= centerToEdge_x * 2)
        {
          pageShapes[i - 1].push_back(left);
        }
      }
    }
  }

  if ((pageShapes.size() > 1) && pageShapes.back().empty()) // the last "page" only has left side
    pageShapes.pop_back();
}

void PMDCollector::fillOutputShapesByPage_OneSided(PageShapesList_t &pageShapes) const
{
  pageShapes.reserve(m_pages.size());
  pageShapes.assign(m_pages.size(), PageShapes_t());

  double centerToEdge_x = m_pageWidth.get().toInches() / 2;
  double centerToEdge_y = m_pageHeight.get().toInches() / 2;
  InchPoint translateShapes(centerToEdge_x, centerToEdge_y);

  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    const PMDPage &page = m_pages[i];
    for (unsigned j = 0; j < page.numShapes(); ++j)
    {
      pageShapes[i].push_back(newOutputShape(page.getShape(j), translateShapes));
    }
  }
}

void PMDCollector::fillOutputShapesByPage(PageShapesList_t &pageShapes) const
{
  if (m_doubleSided)
    fillOutputShapesByPage_TwoSided(pageShapes);
  else
    fillOutputShapesByPage_OneSided(pageShapes);
}

/* Output functions */
void PMDCollector::draw(librevenge::RVNGDrawingInterface *painter) const
{
  painter->startDocument(librevenge::RVNGPropertyList());

  PageShapesList_t shapesByPage;
  fillOutputShapesByPage(shapesByPage);
  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    PageShapes_t shapes = shapesByPage[i];
    writePage(m_pages[i], painter, shapes);
  }
  painter->endDocument();
}

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

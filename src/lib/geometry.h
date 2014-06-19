/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __LIBPAGEMAKER_GEOMETRY_H__
#define __LIBPAGEMAKER_GEOMETRY_H__

#include <vector>
#include "Units.h"
#include "constants.h"

namespace libpagemaker
{
template <typename Unit> struct Point
{
  Unit m_x;
  Unit m_y;

  Point(Unit x, Unit y) : m_x(x), m_y(y)
  { }
};

typedef Point<PMDShapeUnit> PMDShapePoint;
typedef Point<double> InchPoint;

struct PMDParaProperties
{
  uint16_t m_length;
  uint8_t m_align;
  uint16_t m_leftIndent;
  uint16_t m_firstIndent;
  uint16_t m_rightIndent;
  uint16_t m_beforeIndent;
  uint16_t m_afterIndent;

  PMDParaProperties(const uint16_t length, const uint8_t align,const uint16_t leftIndent,const uint16_t firstIndent,const uint16_t rightIndent,const uint16_t beforeIndent,const uint16_t afterIndent)
    : m_length(length), m_align(align), m_leftIndent(leftIndent), m_firstIndent(firstIndent), m_rightIndent(rightIndent), m_beforeIndent(beforeIndent), m_afterIndent(afterIndent)
  { }
};

struct PMDCharProperties
{
  uint16_t m_length;
  uint16_t m_fontFace;
  uint16_t m_fontSize;
  uint8_t m_fontColor;
  uint8_t m_boldItalicUnderline;
  uint8_t m_superSubscript;
  int16_t m_kerning;
  uint16_t m_superSubSize;
  uint16_t m_superPos;
  uint16_t m_subPos;

  PMDCharProperties(const uint16_t length, const uint16_t fontFace, const uint16_t fontSize, const uint8_t fontColor, const uint8_t boldItalicUnderline, const uint8_t superSubscript, const int16_t kerning, const uint16_t superSubSize, const uint16_t superPos, const uint16_t subPos)
    : m_length(length), m_fontFace(fontFace), m_fontSize(fontSize), m_fontColor(fontColor), m_boldItalicUnderline(boldItalicUnderline), m_superSubscript(superSubscript), m_kerning(kerning), m_superSubSize(superSubSize), m_superPos(superPos), m_subPos(subPos)
  { }
};

class PMDLineSet
{
public:
  virtual std::vector<PMDShapePoint> getPoints() const = 0;
  virtual bool getIsClosed() const = 0;
  virtual double getRotation() const = 0;
  virtual double getSkew() const = 0;
  virtual PMDShapePoint getRotatingPoint() const = 0;
  virtual PMDShapePoint getXformTopLeft() const = 0;
  virtual PMDShapePoint getXformBotRight() const = 0;
  virtual uint8_t shapeType() const = 0;
  virtual PMDShapePoint getTopLeft() const = 0;
  virtual PMDShapePoint getBotRight() const = 0;
  virtual uint8_t getFillType() const = 0;
  virtual uint8_t getFillColor() const = 0;
  virtual uint8_t getFillOverprint() const = 0;
  virtual uint8_t getFillTint() const = 0;
  virtual uint8_t getStrokeType() const = 0;
  virtual uint8_t getStrokeWidth() const = 0;
  virtual uint8_t getStrokeColor() const = 0;
  virtual uint8_t getStrokeOverprint() const = 0;
  virtual uint8_t getStrokeTint() const = 0;
  virtual std::string getText() const = 0;
  virtual std::vector<PMDCharProperties> getCharProperties() const = 0;
  virtual std::vector<PMDParaProperties> getParaProperties() const = 0;

  virtual ~PMDLineSet()
  {
  }
};

class PMDLine : public PMDLineSet
{
  PMDShapePoint m_topLeft;
  PMDShapePoint m_botRight;
  bool m_mirrored;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDLine(const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const bool mirrored, const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_topLeft(topLeft), m_botRight(botRight), m_mirrored(mirrored), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  virtual double getRotation() const
  {
    return 0;
  }

  virtual double getSkew() const
  {
    return 0;
  }

  virtual PMDShapePoint getXformTopLeft() const
  {
    return PMDShapePoint(0,0);
  }

  virtual PMDShapePoint getXformBotRight() const
  {
    return PMDShapePoint(0,0);
  }

  virtual PMDShapePoint getRotatingPoint() const
  {
    return PMDShapePoint(0,0);
  }

  virtual bool getIsClosed() const
  {
    return false;
  }

  virtual PMDShapePoint getTopLeft() const
  {
    return m_topLeft;
  }

  virtual PMDShapePoint getBotRight() const
  {
    return m_botRight;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    if (m_mirrored)
    {
      points.push_back(PMDShapePoint(m_botRight.m_x,m_topLeft.m_y));
      points.push_back(PMDShapePoint(m_topLeft.m_x,m_botRight.m_y));
    }
    else
    {
      points.push_back(m_topLeft);
      points.push_back(m_botRight);
    }
    return points;
  }

  virtual uint8_t shapeType() const
  {
    return SHAPE_TYPE_LINE;
  }

  virtual uint8_t getFillType() const
  {
    return FILL_SOLID;
  }

  virtual uint8_t getFillColor() const
  {
    return 0;
  }

  virtual uint8_t getFillOverprint() const
  {
    return 0;
  }

  virtual uint8_t getFillTint() const
  {
    return 0;
  }

  virtual uint8_t getStrokeType() const
  {
    return m_strokeType;
  }

  virtual uint8_t getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  virtual uint8_t getStrokeColor() const
  {
    return m_strokeColor;
  }

  virtual uint8_t getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  virtual uint8_t getStrokeTint() const
  {
    return m_strokeTint;
  }

  virtual std::string getText() const
  {
    return "";
  }

  virtual std::vector<PMDCharProperties> getCharProperties() const
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  virtual std::vector<PMDParaProperties> getParaProperties() const
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  virtual ~PMDLine()
  {
  }
};


class PMDPolygon : public PMDLineSet
{
  std::vector<PMDShapePoint> m_points;
  bool m_isClosed;
  double m_rotation;
  double m_skew;
  PMDShapePoint m_topLeft;
  PMDShapePoint m_botRight;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  uint8_t m_fillType;
  uint8_t m_fillColor;
  uint8_t m_fillOverprint;
  uint8_t m_fillTint;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDPolygon(std::vector<PMDShapePoint> points, bool isClosed, const double rotation, const double skew, const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const uint8_t fillType, const uint8_t fillColor, const uint8_t fillOverprint, const uint8_t fillTint, const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_points(points), m_isClosed(isClosed), m_rotation(rotation), m_skew(skew), m_topLeft(topLeft), m_botRight(botRight), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  virtual double getRotation() const
  {
    return m_rotation;
  }

  virtual double getSkew() const
  {
    return m_skew;
  }

  virtual PMDShapePoint getXformTopLeft() const
  {
    return m_xformTopLeft;
  }

  virtual PMDShapePoint getXformBotRight() const
  {
    return m_xformBotRight;
  }

  virtual PMDShapePoint getRotatingPoint() const
  {
    return PMDShapePoint(0,0);
  }

  virtual PMDShapePoint getTopLeft() const
  {
    return m_topLeft;
  }

  virtual PMDShapePoint getBotRight() const
  {
    return m_botRight;
  }

  virtual bool getIsClosed() const
  {
    return m_isClosed;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    return m_points;
  }

  virtual uint8_t shapeType() const
  {
    return SHAPE_TYPE_POLY;
  }

  virtual uint8_t getFillType() const
  {
    return m_fillType;
  }

  virtual uint8_t getFillColor() const
  {
    return m_fillColor;
  }

  virtual uint8_t getFillOverprint() const
  {
    return m_fillOverprint;
  }

  virtual uint8_t getFillTint() const
  {
    return m_fillTint;
  }

  virtual uint8_t getStrokeType() const
  {
    return m_strokeType;
  }

  virtual uint8_t getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  virtual uint8_t getStrokeColor() const
  {
    return m_strokeColor;
  }

  virtual uint8_t getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  virtual uint8_t getStrokeTint() const
  {
    return m_strokeTint;
  }

  virtual std::string getText() const
  {
    return "";
  }

  virtual std::vector<PMDCharProperties> getCharProperties() const
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  virtual std::vector<PMDParaProperties> getParaProperties() const
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  virtual ~PMDPolygon()
  {
  }
};

class PMDTextBox : public PMDLineSet
{
  PMDShapePoint m_topLeft;
  PMDShapePoint m_botRight;
  double m_rotation;
  double m_skew;
  PMDShapePoint m_rotatingPoint;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  std::string m_text;
  std::vector<PMDCharProperties> m_charProps;
  std::vector<PMDParaProperties> m_paraProps;

public:
  PMDTextBox(const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const double rotation, const double skew, const PMDShapePoint rotatingPoint, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const std::string text, const std::vector<PMDCharProperties> charProps, const std::vector<PMDParaProperties> paraProps)
    : m_topLeft(topLeft), m_botRight(botRight), m_rotation(rotation), m_skew(skew), m_rotatingPoint(rotatingPoint), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_text(text), m_charProps(charProps), m_paraProps(paraProps)
  { }

  virtual double getRotation() const
  {
    return m_rotation;
  }

  virtual double getSkew() const
  {
    return m_skew;
  }

  virtual PMDShapePoint getXformTopLeft() const
  {
    return m_xformTopLeft;
  }

  virtual PMDShapePoint getXformBotRight() const
  {
    return m_xformBotRight;
  }

  virtual PMDShapePoint getRotatingPoint() const
  {
    return m_rotatingPoint;
  }

  virtual PMDShapePoint getTopLeft() const
  {
    return m_topLeft;
  }

  virtual PMDShapePoint getBotRight() const
  {
    return m_botRight;
  }

  virtual bool getIsClosed() const
  {
    return true;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_topLeft);

    return points;
  }

  virtual uint8_t shapeType() const
  {
    return SHAPE_TYPE_TEXTBOX;
  }

  virtual uint8_t getFillType() const
  {
    return 0;
  }

  virtual uint8_t getFillColor() const
  {
    return 0;
  }

  virtual uint8_t getFillOverprint() const
  {
    return 0;
  }

  virtual uint8_t getFillTint() const
  {
    return 0;
  }

  virtual uint8_t getStrokeType() const
  {
    return 0;
  }

  virtual uint8_t getStrokeWidth() const
  {
    return 0;
  }

  virtual uint8_t getStrokeColor() const
  {
    return 0;
  }

  virtual uint8_t getStrokeOverprint() const
  {
    return 0;
  }

  virtual uint8_t getStrokeTint() const
  {
    return 0;
  }

  virtual std::string getText() const
  {
    return m_text;
  }

  virtual std::vector<PMDCharProperties> getCharProperties() const
  {
    return m_charProps;
  }

  virtual std::vector<PMDParaProperties> getParaProperties() const
  {
    return m_paraProps;
  }

  virtual ~PMDTextBox()
  {
  }
};

class PMDRectangle : public PMDLineSet
{
  PMDShapePoint m_topLeft;
  PMDShapePoint m_botRight;
  double m_rotation;
  double m_skew;
  PMDShapePoint m_rotatingPoint;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  uint8_t m_fillType;
  uint8_t m_fillColor;
  uint8_t m_fillOverprint;
  uint8_t m_fillTint;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDRectangle(const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const double rotation, const double skew, const PMDShapePoint rotatingPoint, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const uint8_t fillType, const uint8_t fillColor, const uint8_t fillOverprint, const uint8_t fillTint,  const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_topLeft(topLeft), m_botRight(botRight), m_rotation(rotation), m_skew(skew), m_rotatingPoint(rotatingPoint), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  virtual double getRotation() const
  {
    return m_rotation;
  }

  virtual double getSkew() const
  {
    return m_skew;
  }

  virtual PMDShapePoint getXformTopLeft() const
  {
    return m_xformTopLeft;
  }

  virtual PMDShapePoint getXformBotRight() const
  {
    return m_xformBotRight;
  }

  virtual PMDShapePoint getRotatingPoint() const
  {
    return m_rotatingPoint;
  }

  virtual PMDShapePoint getTopLeft() const
  {
    return m_topLeft;
  }

  virtual PMDShapePoint getBotRight() const
  {
    return m_botRight;
  }

  virtual bool getIsClosed() const
  {
    return true;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_topLeft);
    points.push_back(PMDShapePoint(m_botRight.m_x, m_topLeft.m_y));
    points.push_back(m_botRight);
    points.push_back(PMDShapePoint(m_topLeft.m_x, m_botRight.m_y));

    return points;
  }

  virtual uint8_t shapeType() const
  {
    return SHAPE_TYPE_RECT;
  }

  virtual uint8_t getFillType() const
  {
    return m_fillType;
  }

  virtual uint8_t getFillColor() const
  {
    return m_fillColor;
  }

  virtual uint8_t getFillOverprint() const
  {
    return m_fillOverprint;
  }

  virtual uint8_t getFillTint() const
  {
    return m_fillTint;
  }

  virtual uint8_t getStrokeType() const
  {
    return m_strokeType;
  }

  virtual uint8_t getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  virtual uint8_t getStrokeColor() const
  {
    return m_strokeColor;
  }

  virtual uint8_t getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  virtual uint8_t getStrokeTint() const
  {
    return m_strokeTint;
  }

  virtual std::string getText() const
  {
    return "";
  }

  virtual std::vector<PMDCharProperties> getCharProperties() const
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  virtual std::vector<PMDParaProperties> getParaProperties() const
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  virtual ~PMDRectangle()
  {
  }
};

class PMDEllipse : public PMDLineSet
{
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  double m_rotation;
  double m_skew;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  uint8_t m_fillType;
  uint8_t m_fillColor;
  uint8_t m_fillOverprint;
  uint8_t m_fillTint;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDEllipse(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const double rotation, const double skew, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const uint8_t fillType, const uint8_t fillColor, const uint8_t fillOverprint, const uint8_t fillTint,  const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight), m_rotation(rotation), m_skew(skew), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  virtual double getRotation() const
  {
    return m_rotation;
  }

  virtual double getSkew() const
  {
    return m_skew;
  }

  virtual PMDShapePoint getXformTopLeft() const
  {
    return m_xformTopLeft;
  }

  virtual PMDShapePoint getXformBotRight() const
  {
    return m_xformBotRight;
  }

  virtual PMDShapePoint getRotatingPoint() const
  {
    return PMDShapePoint(0,0);
  }
  virtual bool getIsClosed() const
  {
    return true;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_bboxTopLeft);
    points.push_back(m_bboxBotRight);

    return points;
  }

  virtual PMDShapePoint getTopLeft() const
  {
    return m_bboxTopLeft;
  }

  virtual PMDShapePoint getBotRight() const
  {
    return m_bboxBotRight;
  }

  virtual uint8_t shapeType() const
  {
    return SHAPE_TYPE_ELLIPSE;
  }

  virtual uint8_t getFillType() const
  {
    return m_fillType;
  }

  virtual uint8_t getFillColor() const
  {
    return m_fillColor;
  }

  virtual uint8_t getFillOverprint() const
  {
    return m_fillOverprint;
  }

  virtual uint8_t getFillTint() const
  {
    return m_fillTint;
  }

  virtual uint8_t getStrokeType() const
  {
    return m_strokeType;
  }

  virtual uint8_t getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  virtual uint8_t getStrokeColor() const
  {
    return m_strokeColor;
  }

  virtual uint8_t getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  virtual uint8_t getStrokeTint() const
  {
    return m_strokeTint;
  }

  virtual std::string getText() const
  {
    return "";
  }

  virtual std::vector<PMDCharProperties> getCharProperties() const
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  virtual std::vector<PMDParaProperties> getParaProperties() const
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  virtual ~PMDEllipse()
  {
  }
};

class TransformationMatrix
{
  double m_tl, m_tr, m_bl, m_br;

public:
  TransformationMatrix(double topLeft, double topRight, double bottomLeft, double bottomRight)
    : m_tl(topLeft), m_tr(topRight), m_bl(bottomLeft), m_br(bottomRight)
  { }

  template <typename Unit> InchPoint transform(const Point<Unit> &point) const
  {
    double xInches = point.m_x.toInches(),
           yInches = point.m_y.toInches();
    double newX = m_tl * xInches + m_tr * yInches,
           newY = m_bl * xInches + m_br * yInches;
    return InchPoint(newX, newY);
  }
};
std::pair<InchPoint, InchPoint>
getBoundingBox(const PMDLineSet &lineSet, const TransformationMatrix &matrix);
}

#endif /* __LIBPAGEMAKER_GEOMETRY_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

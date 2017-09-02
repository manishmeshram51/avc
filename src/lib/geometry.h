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
#include <librevenge/librevenge.h>
#include <PMDTypes.h>
#include "libpagemaker_utils.h"

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

struct PMDXForm
{
  uint32_t m_rotationDegree;
  uint32_t m_skewDegree;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  PMDShapePoint m_rotatingPoint;
  uint32_t m_xformId;

  PMDXForm(const uint32_t rotationDegree, const uint32_t skewDegree, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const PMDShapePoint rotatingPoint, const uint32_t xformId)
    : m_rotationDegree(rotationDegree), m_skewDegree(skewDegree), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_rotatingPoint(rotatingPoint), m_xformId(xformId)
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
  virtual PMDShapePoint getBboxTopLeft() const = 0;
  virtual PMDShapePoint getBboxBotRight() const = 0;
  virtual PMDFillProperties getFillProperties() const = 0;
  virtual PMDStrokeProperties getStrokeProperties() const = 0;
  virtual std::string getText() const = 0;
  virtual std::vector<PMDCharProperties> getCharProperties() const = 0;
  virtual std::vector<PMDParaProperties> getParaProperties() const = 0;
  virtual librevenge::RVNGBinaryData getBitmap() const = 0;


  virtual ~PMDLineSet()
  {
  }
};

class PMDLine : public PMDLineSet
{
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  bool m_mirrored;
  PMDStrokeProperties m_strokeProps;

public:
  PMDLine(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const bool mirrored, const PMDStrokeProperties strokeProps)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight), m_mirrored(mirrored), m_strokeProps(strokeProps)
  { }

  double getRotation() const override
  {
    return 0;
  }

  double getSkew() const override
  {
    return 0;
  }

  PMDShapePoint getXformTopLeft() const override
  {
    return PMDShapePoint(0,0);
  }

  PMDShapePoint getXformBotRight() const override
  {
    return PMDShapePoint(0,0);
  }

  PMDShapePoint getRotatingPoint() const override
  {
    return PMDShapePoint(0,0);
  }

  bool getIsClosed() const override
  {
    return false;
  }

  PMDShapePoint getBboxTopLeft() const override
  {
    return m_bboxTopLeft;
  }

  PMDShapePoint getBboxBotRight() const override
  {
    return m_bboxBotRight;
  }

  std::vector<PMDShapePoint> getPoints() const override
  {
    std::vector<PMDShapePoint> points;

    if (m_mirrored)
    {
      points.push_back(PMDShapePoint(m_bboxBotRight.m_x,m_bboxTopLeft.m_y));
      points.push_back(PMDShapePoint(m_bboxTopLeft.m_x,m_bboxBotRight.m_y));
    }
    else
    {
      points.push_back(m_bboxTopLeft);
      points.push_back(m_bboxBotRight);
    }
    return points;
  }

  uint8_t shapeType() const override
  {
    return SHAPE_TYPE_LINE;
  }

  PMDFillProperties getFillProperties() const override
  {
    return PMDFillProperties(FILL_SOLID,0,0,0);
  }

  PMDStrokeProperties getStrokeProperties() const override
  {
    return m_strokeProps;
  }

  std::string getText() const override
  {
    return "";
  }

  std::vector<PMDCharProperties> getCharProperties() const override
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  std::vector<PMDParaProperties> getParaProperties() const override
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  librevenge::RVNGBinaryData getBitmap() const override
  {
    librevenge::RVNGBinaryData temp;
    return temp;
  }

  ~PMDLine() override
  {
  }
};


class PMDPolygon : public PMDLineSet
{
  std::vector<PMDShapePoint> m_points;
  bool m_isClosed;
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  PMDXForm m_xFormContainer;
  PMDFillProperties m_fillProps;
  PMDStrokeProperties m_strokeProps;

public:
  PMDPolygon(std::vector<PMDShapePoint> points, bool isClosed, const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const PMDXForm &xFormContainer, const PMDFillProperties fillProps, const PMDStrokeProperties strokeProps)
    : m_points(points), m_isClosed(isClosed), m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight), m_xFormContainer(xFormContainer), m_fillProps(fillProps), m_strokeProps(strokeProps)
  { }

  double getRotation() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_rotationDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  double getSkew() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_skewDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  PMDShapePoint getXformTopLeft() const override
  {
    return m_xFormContainer.m_xformTopLeft;
  }

  PMDShapePoint getXformBotRight() const override
  {
    return m_xFormContainer.m_xformBotRight;
  }

  PMDShapePoint getRotatingPoint() const override
  {
    return m_xFormContainer.m_rotatingPoint;
  }

  PMDShapePoint getBboxTopLeft() const override
  {
    return m_bboxTopLeft;
  }

  PMDShapePoint getBboxBotRight() const override
  {
    return m_bboxBotRight;
  }

  bool getIsClosed() const override
  {
    return m_isClosed;
  }

  std::vector<PMDShapePoint> getPoints() const override
  {
    return m_points;
  }

  uint8_t shapeType() const override
  {
    return SHAPE_TYPE_POLY;
  }

  PMDFillProperties getFillProperties() const override
  {
    return m_fillProps;
  }

  PMDStrokeProperties getStrokeProperties() const override
  {
    return m_strokeProps;
  }

  std::string getText() const override
  {
    return "";
  }

  std::vector<PMDCharProperties> getCharProperties() const override
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  std::vector<PMDParaProperties> getParaProperties() const override
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  librevenge::RVNGBinaryData getBitmap() const override
  {
    librevenge::RVNGBinaryData temp;
    return temp;
  }

  ~PMDPolygon() override
  {
  }
};

class PMDTextBox : public PMDLineSet
{
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  PMDXForm m_xFormContainer;
  std::string m_text;
  std::vector<PMDCharProperties> m_charProps;
  std::vector<PMDParaProperties> m_paraProps;

public:
  PMDTextBox(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const PMDXForm &xFormContainer, const std::string text, const std::vector<PMDCharProperties> charProps, const std::vector<PMDParaProperties> paraProps)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight),m_xFormContainer(xFormContainer), m_text(text), m_charProps(charProps), m_paraProps(paraProps)
  { }

  double getRotation() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_rotationDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  double getSkew() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_skewDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  PMDShapePoint getXformTopLeft() const override
  {
    return m_xFormContainer.m_xformTopLeft;
  }

  PMDShapePoint getXformBotRight() const override
  {
    return m_xFormContainer.m_xformBotRight;
  }

  PMDShapePoint getRotatingPoint() const override
  {
    return m_xFormContainer.m_rotatingPoint;
  }

  PMDShapePoint getBboxTopLeft() const override
  {
    return m_bboxTopLeft;
  }

  PMDShapePoint getBboxBotRight() const override
  {
    return m_bboxBotRight;
  }

  bool getIsClosed() const override
  {
    return true;
  }

  std::vector<PMDShapePoint> getPoints() const override
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_bboxTopLeft);

    return points;
  }

  uint8_t shapeType() const override
  {
    return SHAPE_TYPE_TEXTBOX;
  }

  PMDFillProperties getFillProperties() const override
  {
    return PMDFillProperties(0,0,0,0);
  }

  PMDStrokeProperties getStrokeProperties() const override
  {
    return PMDStrokeProperties(0,0,0,0,0);
  }

  std::string getText() const override
  {
    return m_text;
  }

  std::vector<PMDCharProperties> getCharProperties() const override
  {
    return m_charProps;
  }

  std::vector<PMDParaProperties> getParaProperties() const override
  {
    return m_paraProps;
  }

  librevenge::RVNGBinaryData getBitmap() const override
  {
    librevenge::RVNGBinaryData temp;
    return temp;
  }

  ~PMDTextBox() override
  {
  }
};

class PMDRectangle : public PMDLineSet
{
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  PMDXForm m_xFormContainer;
  PMDFillProperties m_fillProps;
  PMDStrokeProperties m_strokeProps;

public:
  PMDRectangle(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const PMDXForm &xFormContainer, const PMDFillProperties fillProps, const PMDStrokeProperties strokeProps)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight),m_xFormContainer(xFormContainer), m_fillProps(fillProps), m_strokeProps(strokeProps)
  { }

  double getRotation() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_rotationDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  double getSkew() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_skewDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  PMDShapePoint getXformTopLeft() const override
  {
    return m_xFormContainer.m_xformTopLeft;
  }

  PMDShapePoint getXformBotRight() const override
  {
    return m_xFormContainer.m_xformBotRight;
  }

  PMDShapePoint getRotatingPoint() const override
  {
    return m_xFormContainer.m_rotatingPoint;
  }

  PMDShapePoint getBboxTopLeft() const override
  {
    return m_bboxTopLeft;
  }

  PMDShapePoint getBboxBotRight() const override
  {
    return m_bboxBotRight;
  }

  bool getIsClosed() const override
  {
    return true;
  }

  std::vector<PMDShapePoint> getPoints() const override
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_bboxTopLeft);
    points.push_back(PMDShapePoint(m_bboxBotRight.m_x, m_bboxTopLeft.m_y));
    points.push_back(m_bboxBotRight);
    points.push_back(PMDShapePoint(m_bboxTopLeft.m_x, m_bboxBotRight.m_y));

    return points;
  }

  uint8_t shapeType() const override
  {
    return SHAPE_TYPE_RECT;
  }

  PMDFillProperties getFillProperties() const override
  {
    return m_fillProps;
  }

  PMDStrokeProperties getStrokeProperties() const override
  {
    return m_strokeProps;
  }

  std::string getText() const override
  {
    return "";
  }

  std::vector<PMDCharProperties> getCharProperties() const override
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  std::vector<PMDParaProperties> getParaProperties() const override
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  librevenge::RVNGBinaryData getBitmap() const override
  {
    librevenge::RVNGBinaryData temp;
    return temp;
  }

  ~PMDRectangle() override
  {
  }
};

class PMDEllipse : public PMDLineSet
{
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  PMDXForm m_xFormContainer;
  PMDFillProperties m_fillProps;
  PMDStrokeProperties m_strokeProps;

public:
  PMDEllipse(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const PMDXForm &xFormContainer, const PMDFillProperties fillProps, const PMDStrokeProperties strokeProps)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight), m_xFormContainer(xFormContainer), m_fillProps(fillProps), m_strokeProps(strokeProps)
  { }

  double getRotation() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_rotationDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  double getSkew() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_skewDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  PMDShapePoint getXformTopLeft() const override
  {
    return m_xFormContainer.m_xformTopLeft;
  }

  PMDShapePoint getXformBotRight() const override
  {
    return m_xFormContainer.m_xformBotRight;
  }

  PMDShapePoint getRotatingPoint() const override
  {
    return m_xFormContainer.m_rotatingPoint;
  }

  bool getIsClosed() const override
  {
    return true;
  }

  std::vector<PMDShapePoint> getPoints() const override
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_bboxTopLeft);
    points.push_back(m_bboxBotRight);

    return points;
  }

  PMDShapePoint getBboxTopLeft() const override
  {
    return m_bboxTopLeft;
  }

  PMDShapePoint getBboxBotRight() const override
  {
    return m_bboxBotRight;
  }

  uint8_t shapeType() const override
  {
    return SHAPE_TYPE_ELLIPSE;
  }

  PMDFillProperties getFillProperties() const override
  {
    return m_fillProps;
  }

  PMDStrokeProperties getStrokeProperties() const override
  {
    return m_strokeProps;
  }

  std::string getText() const override
  {
    return "";
  }

  std::vector<PMDCharProperties> getCharProperties() const override
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  std::vector<PMDParaProperties> getParaProperties() const override
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  librevenge::RVNGBinaryData getBitmap() const override
  {
    librevenge::RVNGBinaryData temp;
    return temp;
  }

  ~PMDEllipse() override
  {
  }
};

class PMDBitmap : public PMDLineSet
{
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  PMDXForm m_xFormContainer;
  librevenge::RVNGBinaryData m_bitmap;

public:
  PMDBitmap(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const PMDXForm &xFormContainer, const librevenge::RVNGBinaryData &bitmap)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight), m_xFormContainer(xFormContainer),m_bitmap(bitmap)
  { }

  double getRotation() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_rotationDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  double getSkew() const override
  {
    int32_t temp = (int32_t)m_xFormContainer.m_skewDegree;
    return (-1 * (double)temp/1000 * (M_PI/180));
  }

  PMDShapePoint getXformTopLeft() const override
  {
    return m_xFormContainer.m_xformTopLeft;
  }

  PMDShapePoint getXformBotRight() const override
  {
    return m_xFormContainer.m_xformBotRight;
  }

  PMDShapePoint getRotatingPoint() const override
  {
    return m_xFormContainer.m_rotatingPoint;
  }

  PMDShapePoint getBboxTopLeft() const override
  {
    return m_bboxTopLeft;
  }

  PMDShapePoint getBboxBotRight() const override
  {
    return m_bboxBotRight;
  }

  bool getIsClosed() const override
  {
    return true;
  }

  std::vector<PMDShapePoint> getPoints() const override
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_bboxTopLeft);
    points.push_back(PMDShapePoint(m_bboxBotRight.m_x, m_bboxTopLeft.m_y));
    points.push_back(m_bboxBotRight);
    points.push_back(PMDShapePoint(m_bboxTopLeft.m_x, m_bboxBotRight.m_y));

    return points;
  }

  uint8_t shapeType() const override
  {
    return SHAPE_TYPE_BITMAP;
  }

  PMDFillProperties getFillProperties() const override
  {
    return PMDFillProperties(0,0,0,0);
  }

  PMDStrokeProperties getStrokeProperties() const override
  {
    return PMDStrokeProperties(0,0,0,0,0);
  }

  std::string getText() const override
  {
    return "";
  }

  std::vector<PMDCharProperties> getCharProperties() const override
  {
    std::vector<PMDCharProperties> temp;
    temp.push_back(PMDCharProperties(0,0,0,0,0,0,0,0,0,0,0));
    return temp;
  }

  std::vector<PMDParaProperties> getParaProperties() const override
  {
    std::vector<PMDParaProperties> temp;
    temp.push_back(PMDParaProperties(0,0,0,0,0,0,0));
    return temp;
  }

  librevenge::RVNGBinaryData getBitmap() const override
  {
    return m_bitmap;
  }

  ~PMDBitmap() override
  {
  }
};

class TransformationMatrix
{
  double m_tl, m_tr, m_bl, m_br;

public:
  TransformationMatrix(double bboxTopLeft, double topRight, double bottomLeft, double bottomRight)
    : m_tl(bboxTopLeft), m_tr(topRight), m_bl(bottomLeft), m_br(bottomRight)
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

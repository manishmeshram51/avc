/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __LIBPAGEMAKER_OUTPUTSHAPE_H__
#define __LIBPAGEMAKER_OUTPUTSHAPE_H__

#include "geometry.h"
#include "PMDExceptions.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include "libpagemaker_utils.h"
namespace libpagemaker
{

class OutputShape
{
  bool m_isClosed;
  uint8_t m_shapeType;
  std::vector<InchPoint> m_points;
  double m_rotation;
  double m_skew;
  double m_bboxLeft, m_bboxTop, m_bboxRight, m_bboxBot;
  PMDFillProperties m_fillProps;
  PMDStrokeProperties m_strokeProps;
  std::string m_text;
  std::vector<PMDCharProperties> m_charProps;
  std::vector<PMDParaProperties> m_paraProps;
  librevenge::RVNGBinaryData m_bitmap;
  double m_width,m_height;

public:
  OutputShape(bool isClosed, int shape, double rotation, double skew, const PMDFillProperties &fillProps, const PMDStrokeProperties &strokeProps)
    : m_isClosed(isClosed), m_shapeType(shape), m_points(), m_rotation(rotation), m_skew(skew),
      m_bboxLeft(), m_bboxTop(), m_bboxRight(), m_bboxBot(), m_fillProps(fillProps), m_strokeProps(strokeProps), m_text(), m_charProps(),  m_paraProps(), m_bitmap(), m_width(), m_height()
  { }

  OutputShape(bool isClosed, int shape, double rotation, double skew, std::string text, std::vector<PMDCharProperties> charProps, std::vector<PMDParaProperties> paraProps)
    : m_isClosed(isClosed), m_shapeType(shape), m_points(), m_rotation(rotation), m_skew(skew),
      m_bboxLeft(), m_bboxTop(), m_bboxRight(), m_bboxBot(),
      m_fillProps(PMDFillProperties(0,0,0,0)),
      m_strokeProps(PMDStrokeProperties(0,0,0,0,0)),
      m_text(text), m_charProps(charProps), m_paraProps(paraProps), m_bitmap(), m_width(), m_height()
  { }

  OutputShape(bool isClosed, int shape, double rotation, double skew, librevenge::RVNGBinaryData bitmap)
    : m_isClosed(isClosed), m_shapeType(shape), m_points(), m_rotation(rotation), m_skew(skew),
      m_bboxLeft(), m_bboxTop(), m_bboxRight(), m_bboxBot(),
      m_fillProps(PMDFillProperties(0,0,0,0)),
      m_strokeProps(PMDStrokeProperties(0,0,0,0,0)),
      m_text(), m_charProps(), m_paraProps(), m_bitmap(bitmap), m_width(), m_height()
  { }

  unsigned numPoints() const
  {
    return m_points.size();
  }

  InchPoint getPoint(unsigned i) const
  {
    return (m_points.size() > i) ? m_points[i] : InchPoint(0, 0);
  }

  bool getIsClosed() const
  {
    return m_isClosed;
  }

  uint8_t shapeType() const
  {
    return m_shapeType;
  }

  const PMDFillProperties &getFillProperties() const
  {
    return m_fillProps;
  }

  const PMDStrokeProperties &getStrokeProperties() const
  {
    return m_strokeProps;
  }

  double getRotation() const
  {
    return m_rotation;
  }

  double getSkew() const
  {
    return m_skew;
  }

  std::string getText() const
  {
    return m_text;
  }

  std::vector<PMDCharProperties> getCharProperties() const
  {
    return m_charProps;
  }

  std::vector<PMDParaProperties> getParaProperties() const
  {
    return m_paraProps;
  }

  librevenge::RVNGBinaryData getBitmap() const
  {
    return m_bitmap;
  }

  std::pair<InchPoint, InchPoint> getBoundingBox() const
  {
    if (m_points.empty() && m_bitmap.empty())
    {
      throw EmptyLineSetException();
    }
    return std::make_pair(InchPoint(m_bboxLeft, m_bboxTop), InchPoint(m_bboxRight, m_bboxBot));
  }

  void addPoint(InchPoint point)
  {
    double x = point.m_x,
           y = point.m_y;
    if (m_points.empty())
    {
      m_bboxLeft = m_bboxRight = x;
      m_bboxTop = m_bboxBot = y;
    }
    else
    {
      if (x < m_bboxLeft) m_bboxLeft = x;
      if (y < m_bboxTop) m_bboxTop = y;
      if (x > m_bboxRight) m_bboxRight = x;
      if (y > m_bboxBot) m_bboxBot = y;
    }
    m_points.push_back(InchPoint(x, y));
  }

  void setDimensions(double width, double height)
  {
    m_width = width,
    m_height = height;
  }

  double getWidth() const
  {
    return m_width;
  }

  double getHeight() const
  {
    return m_height;
  }
};


boost::shared_ptr<OutputShape> newOutputShape(
  const boost::shared_ptr<const PMDLineSet> &lineSet, const InchPoint &translate);

}

#endif /* __LIBPAGEMAKER_OUTPUTSHAPE_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

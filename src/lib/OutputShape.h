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
namespace libpagemaker
{

class OutputShape
{
  bool m_isClosed;
  uint8_t m_shapeType;
  std::vector<InchPoint> m_points;
  double m_rotation;
  double m_skew;
  double m_left, m_top, m_right, m_bot;
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
  OutputShape(bool isClosed, int shape, double rotation, double skew, uint8_t fillType, uint8_t fillColor, uint8_t fillOverprint, uint8_t fillTint, uint8_t strokeType, uint8_t strokeWidth, uint8_t strokeColor, uint8_t strokeOverprint, uint8_t strokeTint)
    : m_isClosed(isClosed), m_shapeType(shape), m_points(), m_rotation(rotation), m_skew(skew),
      m_left(), m_top(), m_right(), m_bot(), m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  unsigned numPoints() const
  {
    return m_points.size();
  }

  InchPoint getPoint(unsigned i) const
  {
    return m_points[i];
  }

  bool getIsClosed() const
  {
    return m_isClosed;
  }

  uint8_t shapeType() const
  {
    return m_shapeType;
  }

  uint8_t getFillType() const
  {
    return m_fillType;
  }

  uint8_t virtual getFillColor() const
  {
    return m_fillColor;
  }

  uint8_t virtual getFillOverprint() const
  {
    return m_fillOverprint;
  }

  uint8_t virtual getFillTint() const
  {
    return m_fillTint;
  }

  uint8_t virtual getStrokeType() const
  {
    return m_strokeType;
  }

  uint8_t virtual getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  uint8_t virtual getStrokeColor() const
  {
    return m_strokeColor;
  }

  uint8_t virtual getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  uint8_t virtual getStrokeTint() const
  {
    return m_strokeTint;
  }

  double getRotation() const
  {
    return m_rotation;
  }

  double getSkew() const
  {
    return m_skew;
  }

  std::pair<InchPoint, InchPoint> getBoundingBox() const
  {
    if (m_points.empty())
    {
      throw EmptyLineSetException();
    }
    return std::make_pair(InchPoint(m_left, m_top), InchPoint(m_right, m_bot));
  }

  void addPoint(InchPoint point)
  {
    double x = point.m_x,
           y = point.m_y;
    if (m_points.empty())
    {
      m_left = m_right = x;
      m_top = m_bot = y;
    }
    else
    {
      if (x < m_left) m_left = x;
      if (y < m_top) m_top = y;
      if (x > m_right) m_right = x;
      if (y > m_bot) m_bot = y;
    }
    m_points.push_back(InchPoint(x, y));
  }
};


boost::shared_ptr<OutputShape> newOutputShape(
  boost::shared_ptr<const PMDLineSet> lineSet, InchPoint translate);

}

#endif /* __LIBPAGEMAKER_OUTPUTSHAPE_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

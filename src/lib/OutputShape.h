#pragma once
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
  double m_left, m_top, m_right, m_bot;
public:
  OutputShape(bool isClosed, int shape, double rotation)
    : m_isClosed(isClosed), m_shapeType(shape), m_points(), m_rotation(rotation),
      m_left(), m_top(), m_right(), m_bot()
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

  double getRotation() const
  {
    return m_rotation;
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
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

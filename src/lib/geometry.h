#pragma once
#include <vector>
#include "Units.h"
namespace libpagemaker
{
template <typename Unit> class Point
{
public:
  Unit m_x;
  Unit m_y;
  
  Point(Unit x, Unit y) : m_x(x), m_y(y)
  { }
};

typedef Point<PMDShapeUnit> PMDShapePoint;
typedef Point<double> InchPoint;

class PMDLineSet
{
  std::vector<PMDShapePoint> m_points;
  bool m_isClosed;
public:
  PMDLineSet(std::vector<PMDShapePoint> points, bool isClosed)
    : m_points(points), m_isClosed(isClosed)
  { }

  const virtual std::vector<PMDShapePoint> &getPoints() const
  {
    return m_points;
  }
  bool virtual getIsClosed() const
  {
    return m_isClosed;
  }

  virtual ~PMDLineSet()
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

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

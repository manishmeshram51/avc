#include <utility>

#include "geometry.h"
#include "PMDExceptions.h"
std::pair<libpagemaker::InchPoint, libpagemaker::InchPoint>
  libpagemaker::getBoundingBox(const PMDLineSet &lineSet, const TransformationMatrix &matrix)
{
  const std::vector<PMDShapePoint> &points = lineSet.getPoints();
  if (points.empty())
  {
    throw EmptyLineSetException();
  }

  InchPoint firstPoint = matrix.transform(points.at(0));
  double minX = firstPoint.m_x,
    maxX = firstPoint.m_x,
    minY = firstPoint.m_y,
    maxY = firstPoint.m_y;

  for (std::vector<PMDShapePoint>::const_iterator i = points.begin() + 1; i != points.end();
    ++i)
  {
    InchPoint point = matrix.transform(*i);
    double x = point.m_x,
      y = point.m_y;
    if (x < minX) minX = x;
    if (y < minY) minY = y;
    if (x > maxX) maxX = x;
    if (y > maxY) maxY = y;
  }

  return std::make_pair(InchPoint(minX, minY), InchPoint(maxX, maxY));
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

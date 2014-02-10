#include "OutputShape.h"
#include "geometry.h"

boost::shared_ptr<libpagemaker::OutputShape> libpagemaker::newOutputShape(
  boost::shared_ptr<PMDLineSet> ptrToLineSet)
{
  boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
    new OutputShape(lineSet->getIsClosed()));
  std::vector<PMDShapePoint> pmdPoints = ptrToLineSet->getPoints();
  for (unsigned i = 0; i < points.size(); ++i)
  {
    double x = points[i].m_x.toInches();
    double y = points[i].m_y.toInches();
    ptrToOutputShape->addPoint(InchPoint(x, y));
  }
  return ptrToOutputShape;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

#include "OutputShape.h"
#include "geometry.h"

boost::shared_ptr<libpagemaker::OutputShape> libpagemaker::newOutputShape(
  boost::shared_ptr<const PMDLineSet> ptrToLineSet, InchPoint translate)
{
  boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
    new OutputShape(ptrToLineSet->getIsClosed()));
  std::vector<PMDShapePoint> pmdPoints = ptrToLineSet->getPoints();
  for (unsigned i = 0; i < pmdPoints.size(); ++i)
  {
    double x = pmdPoints[i].m_x.toInches() + translate.m_x;
    double y = pmdPoints[i].m_y.toInches() + translate.m_y;
    ptrToOutputShape->addPoint(InchPoint(x, y));
  }
  return ptrToOutputShape;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

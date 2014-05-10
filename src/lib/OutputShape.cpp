#include "OutputShape.h"
#include "geometry.h"
#include <math.h>

boost::shared_ptr<libpagemaker::OutputShape> libpagemaker::newOutputShape(
  boost::shared_ptr<const PMDLineSet> ptrToLineSet, InchPoint translate)
{
  boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
    new OutputShape(ptrToLineSet->getIsClosed()));
  std::vector<PMDShapePoint> pmdPoints = ptrToLineSet->getPoints();
  float pmdRotation = ptrToLineSet->getRotation() *(M_PI/180);
  if (pmdRotation == 0)
  {
    for (unsigned i = 0; i < pmdPoints.size(); ++i)
    {
      double x = pmdPoints[i].m_x.toInches() + translate.m_x;
      double y = pmdPoints[i].m_y.toInches() + translate.m_y;
      ptrToOutputShape->addPoint(InchPoint(x, y));
    }
  }
  else
  {
    PMDShapePoint pmdRotatingPoint = ptrToLineSet->getRotatingPoint();
    for (unsigned i = 0; i < pmdPoints.size(); ++i)
    {
      double x = cos(pmdRotation)*( pmdPoints[i].m_x.toInches() - pmdRotatingPoint.m_x.toInches())  - sin(pmdRotation)*(pmdPoints[i].m_y.toInches() - pmdRotatingPoint.m_y.toInches()) + pmdRotatingPoint.m_x.toInches()  + translate.m_x;
      double y = sin(pmdRotation)*( pmdPoints[i].m_x.toInches() - pmdRotatingPoint.m_x.toInches())  + cos(pmdRotation)*(pmdPoints[i].m_y.toInches() - pmdRotatingPoint.m_y.toInches()) + pmdRotatingPoint.m_y.toInches()  + translate.m_y;
      ptrToOutputShape->addPoint(InchPoint(x, y));
    }
  }
  return ptrToOutputShape;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

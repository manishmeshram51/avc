#include "OutputShape.h"
#include "geometry.h"
#include <math.h>
#include "libpagemaker_utils.h"
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
    float length = ptrToLineSet->getLength();
    float breadth = ptrToLineSet->getBreadth();

    double x1 = pmdRotatingPoint.m_x.toInches() + translate.m_x;
    double y1 = pmdRotatingPoint.m_y.toInches() + translate.m_y;

    double x2 = x1 + length*cos(pmdRotation);
    double y2 = y1 + length*sin(pmdRotation);

    double x4 = x1 - breadth*sin(pmdRotation);
    double y4 = y1 + breadth*cos(pmdRotation);

    double x3 = x4 + length*cos(pmdRotation);
    double y3 = y4 + length*sin(pmdRotation);

    ptrToOutputShape->addPoint(InchPoint(x1, y1));
    ptrToOutputShape->addPoint(InchPoint(x2, y2));
    ptrToOutputShape->addPoint(InchPoint(x3, y3));
    ptrToOutputShape->addPoint(InchPoint(x4, y4));

  }
  return ptrToOutputShape;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

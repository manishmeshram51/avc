#include "OutputShape.h"
#include "geometry.h"
#include <math.h>
#include "libpagemaker_utils.h"
boost::shared_ptr<libpagemaker::OutputShape> libpagemaker::newOutputShape(
  boost::shared_ptr<const PMDLineSet> ptrToLineSet, InchPoint translate)
{
  boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
    new OutputShape(ptrToLineSet->getIsClosed(), ptrToLineSet->shapeType(), ptrToLineSet->getRotation(), ptrToLineSet->getSkew(), ptrToLineSet->getFillType(), ptrToLineSet->getFillColor(), ptrToLineSet->getFillOverprint(), ptrToLineSet->getFillTint(), ptrToLineSet->getStrokeType(), ptrToLineSet->getStrokeWidth(), ptrToLineSet->getStrokeColor(), ptrToLineSet->getStrokeOverprint(), ptrToLineSet->getStrokeTint()));

  if (ptrToLineSet->shapeType() == SHAPE_TYPE_LINE || ptrToLineSet->shapeType() == SHAPE_TYPE_POLY || ptrToLineSet->shapeType() == SHAPE_TYPE_RECT)
  {
    std::vector<PMDShapePoint> pmdPoints = ptrToLineSet->getPoints();
    double pmdRotation = ptrToLineSet->getRotation();
    double pmdSkew = ptrToLineSet->getSkew();
    if (pmdRotation == 0 && pmdSkew == 0)
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
      PMDShapePoint pmdXformTopLeft = ptrToLineSet->getXformTopLeft();
      PMDShapePoint pmdXformBotRight = ptrToLineSet->getXformBotRight();

      double length = fabs(pmdXformBotRight.m_x.toInches() - pmdXformTopLeft.m_x.toInches());
      double breadth = fabs(pmdXformBotRight.m_y.toInches() - pmdXformTopLeft.m_y.toInches());

      if (ptrToLineSet->shapeType() == SHAPE_TYPE_RECT)
      {
        PMDShapePoint pmdRotatingPoint = ptrToLineSet->getRotatingPoint();

        double x1 = pmdRotatingPoint.m_x.toInches() + translate.m_x;
        double y1 = pmdRotatingPoint.m_y.toInches() + translate.m_y;

        double x2 = x1 + length*cos(pmdRotation);
        double y2 = y1 + length*sin(pmdRotation);

        double x4 = x1 - breadth*sin(pmdRotation);
        double y4 = y1 + breadth*cos(pmdRotation);

        double x3 = x4 + length*cos(pmdRotation);
        double y3 = y4 + length*sin(pmdRotation);

        x3 += breadth*cos(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);
        y3 += breadth*sin(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);
        x4 += breadth*cos(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);
        y4 += breadth*sin(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);

        ptrToOutputShape->addPoint(InchPoint(x1, y1));
        ptrToOutputShape->addPoint(InchPoint(x2, y2));
        ptrToOutputShape->addPoint(InchPoint(x3, y3));
        ptrToOutputShape->addPoint(InchPoint(x4, y4));
      }
      else
      {
        PMDShapePoint topLeft = ptrToLineSet->getTopLeft();
        PMDShapePoint botRight = ptrToLineSet->getBotRight();

        double tx = (botRight.m_x.toInches() + topLeft.m_x.toInches())/2 + translate.m_x;
        double ty = (botRight.m_y.toInches() + topLeft.m_y.toInches())/2 + translate.m_y;

        for (unsigned i = 0; i < pmdPoints.size(); ++i)
        {

          double temp = pmdPoints[i].m_x.toInches() + tan(pmdSkew)*pmdPoints[i].m_y.toInches();
          double  x = temp*cos(pmdRotation) - pmdPoints[i].m_y.toInches()*sin(pmdRotation) + tx;
          double  y = temp*sin(pmdRotation) + pmdPoints[i].m_y.toInches()*cos(pmdRotation) + ty;

          ptrToOutputShape->addPoint(InchPoint(x, y));
        }
      }
    }
    return ptrToOutputShape;
  }
  else
  {
    std::vector<PMDShapePoint> pmdPoints = ptrToLineSet->getPoints();
    double pmdRotation = ptrToLineSet->getRotation();
    double pmdSkew = ptrToLineSet->getSkew();

    double cx = (pmdPoints[0].m_x.toInches() + pmdPoints[1].m_x.toInches())/2 + translate.m_x;
    double cy = (pmdPoints[0].m_y.toInches() + pmdPoints[1].m_y.toInches())/2 + translate.m_y;
    double rx = 0;
    double ry = 0;

    if (pmdRotation == 0 && pmdSkew == 0)
    {
      rx = fabs(pmdPoints[1].m_x.toInches() - pmdPoints[0].m_x.toInches())/2;
      ry = fabs(pmdPoints[1].m_y.toInches() - pmdPoints[0].m_y.toInches())/2;
    }
    else
    {
      PMDShapePoint pmdXformTopLeft = ptrToLineSet->getXformTopLeft();
      PMDShapePoint pmdXformBotRight = ptrToLineSet->getXformBotRight();

      double length = fabs(pmdXformBotRight.m_x.toInches() - pmdXformTopLeft.m_x.toInches());
      double breadth = fabs(pmdXformBotRight.m_y.toInches() - pmdXformTopLeft.m_y.toInches());

      rx = length/2;
      ry = breadth/2;
    }

    ptrToOutputShape->addPoint(InchPoint(cx, cy));
    ptrToOutputShape->addPoint(InchPoint(rx, ry));
    return ptrToOutputShape;
  }
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

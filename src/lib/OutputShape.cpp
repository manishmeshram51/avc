#include "OutputShape.h"
#include "geometry.h"
#include <math.h>
#include "libpagemaker_utils.h"
boost::shared_ptr<libpagemaker::OutputShape> libpagemaker::newOutputShape(
  boost::shared_ptr<const PMDLineSet> ptrToLineSet, InchPoint translate)
{
  boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
    new OutputShape(ptrToLineSet->getIsClosed(), ptrToLineSet->shapeType(), ptrToLineSet->getRotation()));

  if (ptrToLineSet->shapeType() == SHAPE_TYPE_LINE || ptrToLineSet->shapeType() == SHAPE_TYPE_POLY || ptrToLineSet->shapeType() == SHAPE_TYPE_RECT)
  {
    std::vector<PMDShapePoint> pmdPoints = ptrToLineSet->getPoints();
    double pmdRotation = ptrToLineSet->getRotation();
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

      if (pmdPoints.size() == 4 && ptrToLineSet->getIsClosed())
      {
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
      else
      {
        double rx = pmdRotatingPoint.m_x.toInches() + translate.m_x;
        double ry = pmdRotatingPoint.m_y.toInches() + translate.m_y;

        double x = rx + (length/2)*cos(pmdRotation);
        double y = ry + (length/2)*sin(pmdRotation);
        ptrToOutputShape->addPoint(InchPoint(x, y));

        double xTemp =0;
        double yTemp =0;
        if (ptrToLineSet->getIsClosed())
        {
          for (unsigned i = pmdPoints.size()/2 + 1; i < pmdPoints.size(); ++i)
          {
            xTemp = pmdPoints[i].m_x.toInches() - pmdPoints[i-1].m_x.toInches();
            yTemp = pmdPoints[i].m_y.toInches() - pmdPoints[i-1].m_y.toInches();
            x += xTemp*cos(pmdRotation) - yTemp*sin(pmdRotation);
            y += xTemp*sin(pmdRotation) + yTemp*cos(pmdRotation);
            ptrToOutputShape->addPoint(InchPoint(x, y));
          }

          xTemp = pmdPoints[0].m_x.toInches() - pmdPoints[pmdPoints.size() -1].m_x.toInches();
          yTemp = pmdPoints[0].m_y.toInches() - pmdPoints[pmdPoints.size() -1].m_y.toInches();
          x += xTemp*cos(pmdRotation) - yTemp*sin(pmdRotation);
          y += xTemp*sin(pmdRotation) + yTemp*cos(pmdRotation);
          ptrToOutputShape->addPoint(InchPoint(x, y));

          for (unsigned i = 1; i < pmdPoints.size()/2; ++i)
          {
            xTemp = pmdPoints[i].m_x.toInches() - pmdPoints[i-1].m_x.toInches();
            yTemp = pmdPoints[i].m_y.toInches() - pmdPoints[i-1].m_y.toInches();
            x += xTemp*cos(pmdRotation) - yTemp*sin(pmdRotation);
            y += xTemp*sin(pmdRotation) + yTemp*cos(pmdRotation);
            ptrToOutputShape->addPoint(InchPoint(x, y));
          }
        }
        else
        {
          for (unsigned i = 1; i < pmdPoints.size(); ++i)
          {
            xTemp = pmdPoints[i].m_x.toInches() - pmdPoints[i-1].m_x.toInches();
            yTemp = pmdPoints[i].m_y.toInches() - pmdPoints[i-1].m_y.toInches();
            x += xTemp*cos(pmdRotation) - yTemp*sin(pmdRotation);
            y += xTemp*sin(pmdRotation) + yTemp*cos(pmdRotation);
            ptrToOutputShape->addPoint(InchPoint(x, y));
          }
        }
      }
    }
    return ptrToOutputShape;
  }
  else
  {
    std::vector<PMDShapePoint> pmdPoints = ptrToLineSet->getPoints();
    double pmdRotation = ptrToLineSet->getRotation() *(M_PI/180);

    double cx = (pmdPoints[0].m_x.toInches() + pmdPoints[1].m_x.toInches())/2 + translate.m_x;
    double cy = (pmdPoints[0].m_y.toInches() + pmdPoints[1].m_y.toInches())/2 + translate.m_y;
    double rx = 0;
    double ry = 0;

    if (pmdRotation == 0)
    {
      rx = fabs(pmdPoints[1].m_x.toInches() - pmdPoints[0].m_x.toInches())/2;
      ry = fabs(pmdPoints[1].m_y.toInches() - pmdPoints[0].m_y.toInches())/2;
    }
    else
    {
      rx = ptrToLineSet->getLength()/2;
      ry = ptrToLineSet->getBreadth()/2;
    }

    ptrToOutputShape->addPoint(InchPoint(cx, cy));
    ptrToOutputShape->addPoint(InchPoint(rx, ry));
    return ptrToOutputShape;
  }
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

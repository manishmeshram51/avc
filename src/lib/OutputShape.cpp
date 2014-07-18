/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "OutputShape.h"
#include "geometry.h"
#include <math.h>
#include "libpagemaker_utils.h"
boost::shared_ptr<libpagemaker::OutputShape> libpagemaker::newOutputShape(
  const boost::shared_ptr<const PMDLineSet> &ptrToLineSet, const InchPoint &translate)
{
  if (ptrToLineSet->shapeType() == SHAPE_TYPE_TEXTBOX)
  {

    boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
      new OutputShape(ptrToLineSet->getIsClosed(), ptrToLineSet->shapeType(), ptrToLineSet->getRotation(), ptrToLineSet->getSkew(), ptrToLineSet->getText(), ptrToLineSet->getCharProperties(), ptrToLineSet->getParaProperties()));

    PMDShapePoint bboxTopLeft = ptrToLineSet->getBboxTopLeft();
    PMDShapePoint bboxBotRight = ptrToLineSet->getBboxBotRight();

    double pmdRotation = ptrToLineSet->getRotation();
    double pmdSkew = ptrToLineSet->getSkew();

    if (pmdRotation == 0 && pmdSkew == 0)
    {
      double x = bboxTopLeft.m_x.toInches() + translate.m_x;
      double y = bboxTopLeft.m_y.toInches() + translate.m_y;
      ptrToOutputShape->addPoint(InchPoint(x, y));

      double width = fabs(bboxBotRight.m_x.toInches() - bboxTopLeft.m_x.toInches());
      double height = fabs(bboxBotRight.m_y.toInches() - bboxTopLeft.m_y.toInches());
      ptrToOutputShape->setDimensions(width, height);
    }
    else
    {
      PMDShapePoint pmdXformTopLeft = ptrToLineSet->getXformTopLeft();
      PMDShapePoint pmdXformBotRight = ptrToLineSet->getXformBotRight();
      double width = fabs(pmdXformBotRight.m_x.toInches() - pmdXformTopLeft.m_x.toInches());
      double height = fabs(pmdXformBotRight.m_y.toInches() - pmdXformTopLeft.m_y.toInches());
      ptrToOutputShape->setDimensions(width, height);

      PMDShapePoint pmdRotatingPoint = ptrToLineSet->getRotatingPoint();
      double x = pmdRotatingPoint.m_x.toInches() + translate.m_x;
      double y = pmdRotatingPoint.m_y.toInches() + translate.m_y;
      x += (width*cos(pmdRotation)-height*sin(pmdRotation)-width)/2.0;
      y += (width*sin(pmdRotation)+height*cos(pmdRotation)-height)/2.0;
      ptrToOutputShape->addPoint(InchPoint(x, y));
    }

    double x = bboxBotRight.m_x.toInches() + translate.m_x;
    double y = bboxBotRight.m_y.toInches() + translate.m_y;
    ptrToOutputShape->addPoint(InchPoint(x, y));

    return ptrToOutputShape;

  }
  else if (ptrToLineSet->shapeType() == SHAPE_TYPE_BITMAP)
  {
    boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
      new OutputShape(ptrToLineSet->getIsClosed(), ptrToLineSet->shapeType(), ptrToLineSet->getRotation(), ptrToLineSet->getSkew(), ptrToLineSet->getBitmap()));

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

      double width = fabs(pmdXformBotRight.m_x.toInches() - pmdXformTopLeft.m_x.toInches());
      double height = fabs(pmdXformBotRight.m_y.toInches() - pmdXformTopLeft.m_y.toInches());

      PMDShapePoint pmdRotatingPoint = ptrToLineSet->getRotatingPoint();

      double x1 = pmdRotatingPoint.m_x.toInches() + translate.m_x;
      double y1 = pmdRotatingPoint.m_y.toInches() + translate.m_y;

      if (pmdRotation != 0)
      {
        x1 += (width*cos(pmdRotation)-height*sin(pmdRotation)-width)/2.0;
        y1 += (width*sin(pmdRotation)+height*cos(pmdRotation)-height)/2.0;
      }

      double x2 = x1 + width;
      double y2 = y1;

      double x4 = x1 ;
      double y4 = y1 + height;

      double x3 = x1 + width;
      double y3 = y1 + height;


      ptrToOutputShape->addPoint(InchPoint(x1, y1));
      ptrToOutputShape->addPoint(InchPoint(x2, y2));
      ptrToOutputShape->addPoint(InchPoint(x3, y3));
      ptrToOutputShape->addPoint(InchPoint(x4, y4));
    }

    return ptrToOutputShape;
  }
  else
  {
    boost::shared_ptr<libpagemaker::OutputShape> ptrToOutputShape(
      new OutputShape(ptrToLineSet->getIsClosed(), ptrToLineSet->shapeType(), ptrToLineSet->getRotation(), ptrToLineSet->getSkew(), ptrToLineSet->getFillProperties(), ptrToLineSet->getStrokeProperties()));

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

        double width = fabs(pmdXformBotRight.m_x.toInches() - pmdXformTopLeft.m_x.toInches());
        double height = fabs(pmdXformBotRight.m_y.toInches() - pmdXformTopLeft.m_y.toInches());

        if (ptrToLineSet->shapeType() == SHAPE_TYPE_RECT)
        {
          PMDShapePoint pmdRotatingPoint = ptrToLineSet->getRotatingPoint();

          double x1 = pmdRotatingPoint.m_x.toInches() + translate.m_x;
          double y1 = pmdRotatingPoint.m_y.toInches() + translate.m_y;

          double x2 = x1 + width*cos(pmdRotation);
          double y2 = y1 + width*sin(pmdRotation);

          double x4 = x1 - height*sin(pmdRotation);
          double y4 = y1 + height*cos(pmdRotation);

          double x3 = x4 + width*cos(pmdRotation);
          double y3 = y4 + width*sin(pmdRotation);

          x3 += height*cos(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);
          y3 += height*sin(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);
          x4 += height*cos(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);
          y4 += height*sin(pmdRotation)*sin(pmdSkew)/cos(pmdSkew);

          ptrToOutputShape->addPoint(InchPoint(x1, y1));
          ptrToOutputShape->addPoint(InchPoint(x2, y2));
          ptrToOutputShape->addPoint(InchPoint(x3, y3));
          ptrToOutputShape->addPoint(InchPoint(x4, y4));
        }
        else
        {
          PMDShapePoint bboxTopLeft = ptrToLineSet->getBboxTopLeft();
          PMDShapePoint bboxBotRight = ptrToLineSet->getBboxBotRight();

          double tx = (bboxBotRight.m_x.toInches() + bboxTopLeft.m_x.toInches())/2 + translate.m_x;
          double ty = (bboxBotRight.m_y.toInches() + bboxTopLeft.m_y.toInches())/2 + translate.m_y;

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

        double width = fabs(pmdXformBotRight.m_x.toInches() - pmdXformTopLeft.m_x.toInches());
        double height = fabs(pmdXformBotRight.m_y.toInches() - pmdXformTopLeft.m_y.toInches());

        rx = width/2;
        ry = height/2;
      }

      ptrToOutputShape->addPoint(InchPoint(cx, cy));
      ptrToOutputShape->addPoint(InchPoint(rx, ry));
      return ptrToOutputShape;
    }
  }
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

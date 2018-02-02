/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

  for (auto i = points.begin() + 1; i != points.end(); ++i)
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

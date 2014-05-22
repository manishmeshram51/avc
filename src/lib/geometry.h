#pragma once
#include <vector>
#include "Units.h"
#include "yaml_utils.h"
#include "constants.h"
namespace libpagemaker
{
template <typename Unit> class Point
{
public:
  Unit m_x;
  Unit m_y;

  Point(Unit x, Unit y) : m_x(x), m_y(y)
  { }

  void emitYaml(yaml_emitter_t *emitter) const
  {
    yamlBeginMap(emitter);
    yamlMapObject(emitter, "x", m_x);
    yamlMapObject(emitter, "y", m_y);
    yamlEndMap(emitter);
  }
};

typedef Point<PMDShapeUnit> PMDShapePoint;
typedef Point<double> InchPoint;

class PMDLineSet
{
public:
  virtual std::vector<PMDShapePoint> getPoints() const = 0;
  bool virtual getIsClosed() const = 0;
  virtual double getRotation() const = 0;
  PMDShapePoint virtual getRotatingPoint() const = 0;
  virtual double getLength() const = 0;
  virtual double getBreadth() const = 0;
  uint8_t virtual shapeType() const = 0;

  void emitYaml(yaml_emitter_t *emitter) const
  {
    yamlBeginMap(emitter);
    yamlMapEntry(emitter, "closed", getIsClosed());
    yamlForeach(emitter, "points", getPoints());
    yamlEndMap(emitter);
  }

  virtual ~PMDLineSet()
  {
  }
};

class PMDLine : public PMDLineSet
{
  PMDShapePoint m_first;
  PMDShapePoint m_second;
  bool m_mirrored;

public:
  PMDLine(const PMDShapePoint &first, const PMDShapePoint &second, const bool mirrored)
    : m_first(first), m_second(second), m_mirrored(mirrored)
  { }

  double virtual getRotation() const
  {
    return 0;
  }

  double virtual getLength() const
  {
    return 0;
  }

  double virtual getBreadth() const
  {
    return 0;
  }

  PMDShapePoint virtual getRotatingPoint() const
  {
    return PMDShapePoint(0,0);
  }

  bool virtual getIsClosed() const
  {
    return false;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    if (m_mirrored)
    {
      points.push_back(PMDShapePoint(m_second.m_x,m_first.m_y));
      points.push_back(PMDShapePoint(m_first.m_x,m_second.m_y));
    }
    else
    {
      points.push_back(m_first);
      points.push_back(m_second);
    }
    return points;
  }

  uint8_t virtual shapeType() const
  {
    return SHAPE_TYPE_LINE;
  }

  virtual ~PMDLine()
  {
  }
};


class PMDPolygon : public PMDLineSet
{
  std::vector<PMDShapePoint> m_points;
  bool m_isClosed;
  double m_rotation;
  PMDShapePoint m_rotatingPoint;
  double m_length;
  double m_breadth;

public:
  PMDPolygon(std::vector<PMDShapePoint> points, bool isClosed, const double rotationRadian, const PMDShapePoint &rotatingPoint, const double length, const double breadth)
    : m_points(points), m_isClosed(isClosed), m_rotation(rotationRadian), m_rotatingPoint(rotatingPoint), m_length(length), m_breadth(breadth)
  { }

  double virtual getRotation() const
  {
    return m_rotation;
  }

  double virtual getLength() const
  {
    return m_length;
  }

  double virtual getBreadth() const
  {
    return m_breadth;
  }

  PMDShapePoint virtual getRotatingPoint() const
  {
    return m_rotatingPoint;
  }

  bool virtual getIsClosed() const
  {
    return m_isClosed;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    return m_points;
  }

  uint8_t virtual shapeType() const
  {
    return SHAPE_TYPE_POLY;
  }
  virtual ~PMDPolygon()
  {
  }
};

class PMDRectangle : public PMDLineSet
{
  PMDShapePoint m_topLeft;
  PMDShapePoint m_botRight;
  double m_rotation;
  PMDShapePoint m_rotatingPoint;
  double m_length;
  double m_breadth;

public:
  PMDRectangle(const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const double rotation, const PMDShapePoint rotatingPoint, const double length, const double breadth)
    : m_topLeft(topLeft), m_botRight(botRight), m_rotation(rotation), m_rotatingPoint(rotatingPoint), m_length(length), m_breadth(breadth)
  { }

  double virtual getRotation() const
  {
    return m_rotation;
  }

  double virtual getLength() const
  {
    return m_length;
  }

  double virtual getBreadth() const
  {
    return m_breadth;
  }

  PMDShapePoint virtual getRotatingPoint() const
  {
    return m_rotatingPoint;
  }

  bool virtual getIsClosed() const
  {
    return true;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_topLeft);
    points.push_back(PMDShapePoint(m_botRight.m_x, m_topLeft.m_y));
    points.push_back(m_botRight);
    points.push_back(PMDShapePoint(m_topLeft.m_x, m_botRight.m_y));

    return points;
  }

  uint8_t virtual shapeType() const
  {
    return SHAPE_TYPE_RECT;
  }

  virtual ~PMDRectangle()
  {
  }
};

class PMDEllipse : public PMDLineSet
{
  PMDShapePoint m_bboxTopLeft;
  PMDShapePoint m_bboxBotRight;
  double m_rotation;
  double m_length;
  double m_breadth;

public:
  PMDEllipse(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const double rotation, const double length, const double breadth)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight), m_rotation(rotation), m_length(length), m_breadth(breadth)
  { }

  double virtual getRotation() const
  {
    return m_rotation;
  }

  double virtual getLength() const
  {
    return m_length;
  }

  double virtual getBreadth() const
  {
    return m_breadth;
  }

  PMDShapePoint virtual getRotatingPoint() const
  {
    return PMDShapePoint(0,0);
  }
  bool virtual getIsClosed() const
  {
    return true;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    points.push_back(m_bboxTopLeft);
    points.push_back(m_bboxBotRight);

    return points;
  }

  uint8_t virtual shapeType() const
  {
    return SHAPE_TYPE_ELLIPSE;
  }

  virtual ~PMDEllipse()
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

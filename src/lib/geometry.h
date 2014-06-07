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
  double virtual getSkew() const = 0;
  PMDShapePoint virtual getRotatingPoint() const = 0;
  PMDShapePoint virtual getXformTopLeft() const = 0;
  PMDShapePoint virtual getXformBotRight() const = 0;
  uint8_t virtual shapeType() const = 0;
  PMDShapePoint virtual getTopLeft() const = 0;
  PMDShapePoint virtual getBotRight() const = 0;
  uint8_t virtual getFillType() const = 0;
  uint8_t virtual getFillColor() const = 0;
  uint8_t virtual getFillOverprint() const = 0;
  uint8_t virtual getFillTint() const = 0;
  uint8_t virtual getStrokeType() const = 0;
  uint8_t virtual getStrokeWidth() const = 0;
  uint8_t virtual getStrokeColor() const = 0;
  uint8_t virtual getStrokeOverprint() const = 0;
  uint8_t virtual getStrokeTint() const = 0;

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
  PMDShapePoint m_topLeft;
  PMDShapePoint m_botRight;
  bool m_mirrored;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDLine(const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const bool mirrored, const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_topLeft(topLeft), m_botRight(botRight), m_mirrored(mirrored), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  double virtual getRotation() const
  {
    return 0;
  }

  double virtual getSkew() const
  {
    return 0;
  }

  PMDShapePoint virtual getXformTopLeft() const
  {
    return PMDShapePoint(0,0);
  }

  PMDShapePoint virtual getXformBotRight() const
  {
    return PMDShapePoint(0,0);
  }

  PMDShapePoint virtual getRotatingPoint() const
  {
    return PMDShapePoint(0,0);
  }

  bool virtual getIsClosed() const
  {
    return false;
  }

  PMDShapePoint virtual getTopLeft() const
  {
    return m_topLeft;
  }

  PMDShapePoint virtual getBotRight() const
  {
    return m_botRight;
  }

  virtual std::vector<PMDShapePoint> getPoints() const
  {
    std::vector<PMDShapePoint> points;

    if (m_mirrored)
    {
      points.push_back(PMDShapePoint(m_botRight.m_x,m_topLeft.m_y));
      points.push_back(PMDShapePoint(m_topLeft.m_x,m_botRight.m_y));
    }
    else
    {
      points.push_back(m_topLeft);
      points.push_back(m_botRight);
    }
    return points;
  }

  uint8_t virtual shapeType() const
  {
    return SHAPE_TYPE_LINE;
  }

  uint8_t virtual getFillType() const
  {
    return FILL_SOLID;
  }

  uint8_t virtual getFillColor() const
  {
    return 0;
  }

  uint8_t virtual getFillOverprint() const
  {
    return 0;
  }

  uint8_t virtual getFillTint() const
  {
    return 0;
  }

  uint8_t virtual getStrokeType() const
  {
    return m_strokeType;
  }

  uint8_t virtual getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  uint8_t virtual getStrokeColor() const
  {
    return m_strokeColor;
  }

  uint8_t virtual getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  uint8_t virtual getStrokeTint() const
  {
    return m_strokeTint;
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
  double m_skew;
  PMDShapePoint m_topLeft;
  PMDShapePoint m_botRight;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  uint8_t m_fillType;
  uint8_t m_fillColor;
  uint8_t m_fillOverprint;
  uint8_t m_fillTint;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDPolygon(std::vector<PMDShapePoint> points, bool isClosed, const double rotation, const double skew, const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const uint8_t fillType, const uint8_t fillColor, const uint8_t fillOverprint, const uint8_t fillTint, const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_points(points), m_isClosed(isClosed), m_rotation(rotation), m_skew(skew), m_topLeft(topLeft), m_botRight(botRight), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  double virtual getRotation() const
  {
    return m_rotation;
  }

  double virtual getSkew() const
  {
    return m_skew;
  }

  PMDShapePoint virtual getXformTopLeft() const
  {
    return m_xformTopLeft;
  }

  PMDShapePoint virtual getXformBotRight() const
  {
    return m_xformBotRight;
  }

  PMDShapePoint virtual getRotatingPoint() const
  {
    return PMDShapePoint(0,0);
  }

  PMDShapePoint virtual getTopLeft() const
  {
    return m_topLeft;
  }

  PMDShapePoint virtual getBotRight() const
  {
    return m_botRight;
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

  uint8_t virtual getFillType() const
  {
    return m_fillType;
  }

  uint8_t virtual getFillColor() const
  {
    return m_fillColor;
  }

  uint8_t virtual getFillOverprint() const
  {
    return m_fillOverprint;
  }

  uint8_t virtual getFillTint() const
  {
    return m_fillTint;
  }

  uint8_t virtual getStrokeType() const
  {
    return m_strokeType;
  }

  uint8_t virtual getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  uint8_t virtual getStrokeColor() const
  {
    return m_strokeColor;
  }

  uint8_t virtual getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  uint8_t virtual getStrokeTint() const
  {
    return m_strokeTint;
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
  double m_skew;
  PMDShapePoint m_rotatingPoint;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  uint8_t m_fillType;
  uint8_t m_fillColor;
  uint8_t m_fillOverprint;
  uint8_t m_fillTint;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDRectangle(const PMDShapePoint &topLeft, const PMDShapePoint &botRight, const double rotation, const double skew, const PMDShapePoint rotatingPoint, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const uint8_t fillType, const uint8_t fillColor, const uint8_t fillOverprint, const uint8_t fillTint,  const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_topLeft(topLeft), m_botRight(botRight), m_rotation(rotation), m_skew(skew), m_rotatingPoint(rotatingPoint), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  double virtual getRotation() const
  {
    return m_rotation;
  }

  double virtual getSkew() const
  {
    return m_skew;
  }

  PMDShapePoint virtual getXformTopLeft() const
  {
    return m_xformTopLeft;
  }

  PMDShapePoint virtual getXformBotRight() const
  {
    return m_xformBotRight;
  }

  PMDShapePoint virtual getRotatingPoint() const
  {
    return m_rotatingPoint;
  }

  PMDShapePoint virtual getTopLeft() const
  {
    return m_topLeft;
  }

  PMDShapePoint virtual getBotRight() const
  {
    return m_botRight;
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

  uint8_t virtual getFillType() const
  {
    return m_fillType;
  }

  uint8_t virtual getFillColor() const
  {
    return m_fillColor;
  }

  uint8_t virtual getFillOverprint() const
  {
    return m_fillOverprint;
  }

  uint8_t virtual getFillTint() const
  {
    return m_fillTint;
  }

  uint8_t virtual getStrokeType() const
  {
    return m_strokeType;
  }

  uint8_t virtual getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  uint8_t virtual getStrokeColor() const
  {
    return m_strokeColor;
  }

  uint8_t virtual getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  uint8_t virtual getStrokeTint() const
  {
    return m_strokeTint;
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
  double m_skew;
  PMDShapePoint m_xformTopLeft;
  PMDShapePoint m_xformBotRight;
  uint8_t m_fillType;
  uint8_t m_fillColor;
  uint8_t m_fillOverprint;
  uint8_t m_fillTint;
  uint8_t m_strokeType;
  uint8_t m_strokeWidth;
  uint8_t m_strokeColor;
  uint8_t m_strokeOverprint;
  uint8_t m_strokeTint;

public:
  PMDEllipse(const PMDShapePoint &bboxTopLeft, const PMDShapePoint &bboxBotRight, const double rotation, const double skew, const PMDShapePoint xformTopLeft, const PMDShapePoint xformBotRight, const uint8_t fillType, const uint8_t fillColor, const uint8_t fillOverprint, const uint8_t fillTint,  const uint8_t strokeType, const uint8_t strokeWidth, const uint8_t strokeColor, const uint8_t strokeOverprint, const uint8_t strokeTint)
    : m_bboxTopLeft(bboxTopLeft), m_bboxBotRight(bboxBotRight), m_rotation(rotation), m_skew(skew), m_xformTopLeft(xformTopLeft), m_xformBotRight(xformBotRight), m_fillType(fillType), m_fillColor(fillColor), m_fillOverprint(fillOverprint), m_fillTint(fillTint), m_strokeType(strokeType), m_strokeWidth(strokeWidth), m_strokeColor(strokeColor), m_strokeOverprint(strokeOverprint), m_strokeTint(strokeTint)
  { }

  double virtual getRotation() const
  {
    return m_rotation;
  }

  double virtual getSkew() const
  {
    return m_skew;
  }

  PMDShapePoint virtual getXformTopLeft() const
  {
    return m_xformTopLeft;
  }

  PMDShapePoint virtual getXformBotRight() const
  {
    return m_xformBotRight;
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

  PMDShapePoint virtual getTopLeft() const
  {
    return m_bboxTopLeft;
  }

  PMDShapePoint virtual getBotRight() const
  {
    return m_bboxBotRight;
  }

  uint8_t virtual shapeType() const
  {
    return SHAPE_TYPE_ELLIPSE;
  }

  uint8_t virtual getFillType() const
  {
    return m_fillType;
  }

  uint8_t virtual getFillColor() const
  {
    return m_fillColor;
  }

  uint8_t virtual getFillOverprint() const
  {
    return m_fillOverprint;
  }

  uint8_t virtual getFillTint() const
  {
    return m_fillTint;
  }

  uint8_t virtual getStrokeType() const
  {
    return m_strokeType;
  }

  uint8_t virtual getStrokeWidth() const
  {
    return m_strokeWidth;
  }

  uint8_t virtual getStrokeColor() const
  {
    return m_strokeColor;
  }

  uint8_t virtual getStrokeOverprint() const
  {
    return m_strokeOverprint;
  }

  uint8_t virtual getStrokeTint() const
  {
    return m_strokeTint;
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

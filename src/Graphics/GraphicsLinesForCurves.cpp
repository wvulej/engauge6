#include "Curve.h"
#include "CurveStyles.h"
#include "EngaugeAssert.h"
#include "GraphicsLinesForCurve.h"
#include "GraphicsLinesForCurves.h"
#include "GraphicsPointAbstractBase.h"
#include "GraphicsScene.h"
#include "Logger.h"
#include "Point.h"
#include <QGraphicsItem>
#include "QtToString.h"
#include "Transformation.h"

GraphicsLinesForCurves::GraphicsLinesForCurves()
{
}

void GraphicsLinesForCurves::moveLinesWithDraggedPoint (const QString &pointIdentifier,
                                                        const QPointF &scenePos)
{
  QString curveName = Point::curveNameFromPointIdentifier (pointIdentifier);

  if (curveName != AXIS_CURVE_NAME) {

    ENGAUGE_ASSERT (m_graphicsLinesForCurve.contains (curveName));

    m_graphicsLinesForCurve [curveName]->moveLinesWithDraggedPoint (pointIdentifier,
                                                                    scenePos);
  }
}

void GraphicsLinesForCurves::savePoint (GraphicsScene &scene,
                                        const QString &curveName,
                                        const QString &pointIdentifier,
                                        double ordinal,
                                        GraphicsPoint &point)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsLinesForCurves::savePoint"
                              << " curve=" << curveName.toLatin1().data()
                              << " identifier=" << pointIdentifier.toLatin1().data()
                              << " pos=" << QPointFToString (point.pos()).toLatin1().data();

  // No lines are drawn for the axis points, other than the axes checker box
  if (curveName != AXIS_CURVE_NAME) {

    if (!m_graphicsLinesForCurve.contains (curveName)) {

      GraphicsLinesForCurve *item = new GraphicsLinesForCurve(curveName);
      scene.addItem (item);

      m_graphicsLinesForCurve [curveName] = item;
    }

    m_graphicsLinesForCurve [curveName]->savePoint (pointIdentifier,
                                                    ordinal,
                                                    point);
  }
}

void GraphicsLinesForCurves::updateFinish (const CurveStyles &curveStyles)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsLinesForCurves::updateFinish";

  GraphicsLinesContainer::const_iterator itr;
  for (itr = m_graphicsLinesForCurve.begin (); itr != m_graphicsLinesForCurve.end (); itr++) {

    QString curveName = itr.key();
    GraphicsLinesForCurve *graphicsLines = itr.value();

    graphicsLines->updateFinish (curveStyles.lineStyle (curveName));
  }
}

void GraphicsLinesForCurves::updateGraphicsLinesToMatchGraphicsPoints (const CurveStyles &curveStyles)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsLinesForCurves::updateGraphicsLinesToMatchGraphicsPoints";

  GraphicsLinesContainer::const_iterator itr;
  for (itr = m_graphicsLinesForCurve.begin (); itr != m_graphicsLinesForCurve.end (); itr++) {

    QString curveName = itr.key();
    m_graphicsLinesForCurve [curveName]->updateGraphicsLinesToMatchGraphicsPoints(curveStyles.lineStyle (curveName));
  }
}

void GraphicsLinesForCurves::updatePointOrdinalsAfterDrag (const CurveStyles &curveStyles,
                                                           const Transformation &transformation)
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsLinesForCurves::updatePointOrdinalsAfterDrag";

  GraphicsLinesContainer::const_iterator itr;
  for (itr = m_graphicsLinesForCurve.begin (); itr != m_graphicsLinesForCurve.end (); itr++) {

    QString curveName = itr.key();
    GraphicsLinesForCurve *graphicsLines = itr.value();

    graphicsLines->updatePointOrdinalsAfterDrag (curveStyles.lineStyle (curveName),
                                                 transformation);
  }
}

void GraphicsLinesForCurves::updateStart ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "GraphicsLinesForCurves::updateStart";

  GraphicsLinesContainer::const_iterator itr;
  for (itr = m_graphicsLinesForCurve.begin (); itr != m_graphicsLinesForCurve.end (); itr++) {

    GraphicsLinesForCurve *graphicsLines = itr.value();

    graphicsLines->updateStart ();
  }
}

#ifndef CALLBACK_BOUNDING_RECTS_H
#define CALLBACK_BOUNDING_RECTS_H

#include "CallbackSearchReturn.h"
#include <QPointF>
#include <QRectF>
#include <QString>

class Point;

/// Callback for computing the bounding rectangles of the screen and graph coordinates of the points in the Document.
class CallbackBoundingRects
{
public:
  /// Single constructor
  CallbackBoundingRects();

  /// Graph coordinate bounding rectangle
  QRectF boundingRectGraph (bool &isEmpty) const;

  /// Screen coordinate bounding rectangle
  QRectF boundingRectScreen (bool &isEmpty) const;

  /// Callback method.
  CallbackSearchReturn callback (const QString &curveName,
                                 const Point &point);

private:

  void mergeCoordinates (const QPointF &pos,
                         QRectF &boundingRect);

  bool m_isEmpty;
  QRectF m_boundingRectGraph;
  QRectF m_boundingRectScreen;
};

#endif // CALLBACK_BOUNDING_RECTS_H

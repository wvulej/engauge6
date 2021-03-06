#ifndef CALLBACK_SCENE_UPDATE_AFTER_COMMAND_H
#define CALLBACK_SCENE_UPDATE_AFTER_COMMAND_H

#include "CallbackSearchReturn.h"
#include "PointIdentifierToGraphicsPoint.h"
#include "PointStyle.h"

class Document;
class GraphicsScene;
class Point;

/// Callback for updating the QGraphicsItems in the scene after a command may have modified Points in Curves.
class CallbackSceneUpdateAfterCommand
{
public:
  /// Single constructor.
  CallbackSceneUpdateAfterCommand(PointIdentifierToGraphicsPoint &pointIdentifierToGraphicsPoint,
                                  GraphicsScene &scene,
                                  const Document &document);

  /// Callback method.
  CallbackSearchReturn callback (const QString & /* curveName */,
                                 const Point &point);

private:
  CallbackSceneUpdateAfterCommand();

  PointIdentifierToGraphicsPoint &m_pointIdentifierToGraphicsPoint;
  GraphicsScene &m_scene;
  const Document &m_document;
};

#endif // CALLBACK_SCENE_UPDATE_AFTER_COMMAND_H

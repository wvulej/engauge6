#include "Curve.h"
#include "CurvesGraphs.h"
#include "DocumentSerialize.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "Point.h"
#include <QDebug>
#include <QMap>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Transformation.h"
#include "Xml.h"

const QString AXIS_CURVE_NAME ("Axes");
const QString DEFAULT_GRAPH_CURVE_NAME ("Curve1");
const QString TAB_DELIMITER ("\t");

typedef QMap<double, QString> XOrThetaToPointIdentifier;

Curve::Curve(const QString &curveName,
             const ColorFilterSettings &colorFilterSettings,
             const CurveStyle &curveStyle) :
  m_curveName (curveName),
  m_colorFilterSettings (colorFilterSettings),
  m_curveStyle (curveStyle)
{
}

Curve::Curve (const Curve &curve) :
  m_curveName (curve.curveName ()),
  m_points (curve.points ()),
  m_colorFilterSettings (curve.colorFilterSettings ()),
  m_curveStyle (curve.curveStyle ())
{
}

Curve::Curve (QXmlStreamReader &reader)
{
  loadXml(reader);
}

Curve &Curve::operator=(const Curve &curve)
{
  m_curveName = curve.curveName ();
  m_points = curve.points ();
  m_colorFilterSettings = curve.colorFilterSettings ();
  m_curveStyle = curve.curveStyle ();

  return *this;
}

void Curve::addPoint (Point point)
{
  m_points.push_back (point);
}

void Curve::applyTransformation (const Transformation &transformation)
{
  QList<Point>::iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {

    // Get current screen coordinates
    Point &point = *itr;
    QPointF posScreen = point.posScreen();
    QPointF posGraph;
    transformation.transformScreenToRawGraph (posScreen,
                                              posGraph);

    // Overwrite old graph coordinates
    point.setPosGraph (posGraph);
  }
}

ColorFilterSettings Curve::colorFilterSettings () const
{
  return m_colorFilterSettings;
}

QString Curve::curveName () const
{
  return  m_curveName;
}

CurveStyle Curve::curveStyle() const
{
  return m_curveStyle;
}

void Curve::editPoint (const QPointF &posGraph,
                       const QString &identifier)
{
  // Search for the point with matching identifier
  QList<Point>::iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {

    Point &point = *itr;
    if (point.identifier () == identifier) {

      point.setPosGraph (posGraph);
      break;

    }
  }
}

void Curve::exportToClipboard (const QHash<QString, bool> &selectedHash,
                               bool transformIsDefined,
                               QTextStream &strCsv,
                               QTextStream &strHtml,
                               CurvesGraphs &curvesGraphs) const
{
  // This method assumes Copy is only allowed when Transformation is valid

  bool isFirst = true;
  QList<Point>::const_iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {

    const Point &point = *itr;
    if (selectedHash.contains (point.identifier ())) {

      if (isFirst) {

        // Insert headers to identify the points that follow
        isFirst = false;
        strCsv << "X" << TAB_DELIMITER << m_curveName << "\n";
        strHtml << "<table>\n"
                << "<tr><th>X</th><th>" << m_curveName << "</th></tr>\n";
      }

      // Default curve style
      CurveStyle curveStyleDefault;
      curveStyleDefault.setLineStyle(LineStyle::defaultAxesCurve());
      curveStyleDefault.setPointStyle(PointStyle::defaultGraphCurve (curvesGraphs.numCurves ()));

      // Check if this curve already exists from a previously exported point
      if (curvesGraphs.curveForCurveName (m_curveName) == 0) {
        Curve curve(m_curveName,
                    ColorFilterSettings::defaultFilter (),
                    curveStyleDefault);
        curvesGraphs.addGraphCurveAtEnd(curve);
      }

      double x = point.posScreen ().x (), y = point.posScreen ().y ();
      if (transformIsDefined) {
        x = point.posGraph ().x ();
        y = point.posGraph ().y ();
      }

      // Add point to text going to clipboard
      strCsv << x << TAB_DELIMITER << y << "\n";
      strHtml << "<tr><td>" << x << "</td><td>" << y << "</td></tr>\n";

      // Add point to list for undo/redo
      curvesGraphs.curveForCurveName (m_curveName)->addPoint (point);
    }
  }

  if (!isFirst) {
    strHtml << "</table>\n";
  }
}

void Curve::iterateThroughCurvePoints (const Functor2wRet<const QString &, const Point&, CallbackSearchReturn> &ftorWithCallback) const
{
  QList<Point>::const_iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {

    const Point &point = *itr;

    CallbackSearchReturn rtn = ftorWithCallback (m_curveName, point);

    if (rtn == CALLBACK_SEARCH_RETURN_INTERRUPT) {
      break;
    }
  }
}

void Curve::iterateThroughCurveSegments (const Functor2wRet<const Point&, const Point&, CallbackSearchReturn> &ftorWithCallback) const
{
  int index;
  typedef QMap<double, int> MapOrdinalToIndex;

  // Compile a map of ordinals to list indexes
  MapOrdinalToIndex mapOrdinalToIndex;
  QList<Point>::const_iterator itr;
  for (index = 0; index < m_points.count(); index++) {

    const Point &point = m_points [index];
    mapOrdinalToIndex [point.ordinal()] = index;
  }

  // Loop through Points in order of their ordinals
  for (index = 1; index < mapOrdinalToIndex.count(); index++) {

    const Point &pointMinus1 = m_points [index - 1];
    const Point &point = m_points [index];

    CallbackSearchReturn rtn = ftorWithCallback (pointMinus1,
                                                 point);

    if (rtn == CALLBACK_SEARCH_RETURN_INTERRUPT) {
      break;
    }
  }
}

void Curve::loadCurvePoints(QXmlStreamReader &reader)
{
  LOG4CPP_INFO_S ((*mainCat)) << "Curve::loadCurvePoints";

  bool success = true;

  while ((reader.tokenType() != QXmlStreamReader::EndElement) ||
         (reader.name() != DOCUMENT_SERIALIZE_CURVE_POINTS)) {

    QXmlStreamReader::TokenType tokenType = loadNextFromReader(reader);

    if (reader.atEnd()) {
      success = false;
      break;
    }

    if (tokenType == QXmlStreamReader::StartElement) {

      if (reader.name () == DOCUMENT_SERIALIZE_POINT) {

        Point point (reader);
        m_points.push_back (point);
      }
    }
  }

  if (!success) {
    reader.raiseError("Cannot read curve data");
  }
}

void Curve::loadXml(QXmlStreamReader &reader)
{
  LOG4CPP_INFO_S ((*mainCat)) << "Curve::loadXml";

  bool success = true;

  QXmlStreamAttributes attributes = reader.attributes();

  if (attributes.hasAttribute (DOCUMENT_SERIALIZE_CURVE_NAME)) {

    setCurveName (attributes.value (DOCUMENT_SERIALIZE_CURVE_NAME).toString());

    // Read until end of this subtree
    while ((reader.tokenType() != QXmlStreamReader::EndElement) ||
           (reader.name() != DOCUMENT_SERIALIZE_CURVE)){

      QXmlStreamReader::TokenType tokenType = loadNextFromReader(reader);

      if (reader.atEnd()) {
        success = false;
        break;
      }

      if (tokenType == QXmlStreamReader::StartElement) {

        if (reader.name() == DOCUMENT_SERIALIZE_COLOR_FILTER) {
          m_colorFilterSettings.loadXml(reader);
        } else if (reader.name() == DOCUMENT_SERIALIZE_CURVE_POINTS) {
          loadCurvePoints(reader);
        } else if (reader.name() == DOCUMENT_SERIALIZE_CURVE_STYLE) {
          m_curveStyle.loadXml(reader);
        } else {
          success = false;
          break;
        }
      }

      if (reader.hasError()) {
        // No need to set success flag to indicate failure, which raises the error, since the error was already raised. Just
        // need to exit the loop immediately
        break;
      }
    }
  } else {
    success = false;
  }

  if (!success) {
    reader.raiseError ("Cannot read curve data");
  }
}

void Curve::movePoint (const QString &pointIdentifier,
                       const QPointF &deltaScreen)
{
  Point *point = pointForPointIdentifier (pointIdentifier);

  QPointF posScreen = deltaScreen + point->posScreen ();
  point->setPosScreen (posScreen);
}

int Curve::numPoints () const
{
  return m_points.count ();
}

Point *Curve::pointForPointIdentifier (const QString pointIdentifier)
{
  Points::iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {
    Point &point = *itr;
    if (pointIdentifier == point.identifier ()) {
      return &point;
    }
  }

  ENGAUGE_ASSERT (false);
  return 0;
}

QPointF Curve::positionGraph (const QString &pointIdentifier) const
{
  QPointF posGraph;

  // Search for point with matching identifier
  Points::const_iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {
    const Point &point = *itr;
    if (pointIdentifier == point.identifier ()) {
      posGraph = point.posGraph ();
      break;
    }
  }

  return posGraph;
}

QPointF Curve::positionScreen (const QString &pointIdentifier) const
{
  QPointF posScreen;

  // Search for point with matching identifier
  Points::const_iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {
    const Point &point = *itr;
    if (pointIdentifier == point.identifier ()) {
      posScreen = point.posScreen ();
      break;
    }
  }

  return posScreen;
}

const Points Curve::points () const
{
  return m_points;
}

void Curve::removePoint (const QString &identifier)
{
  // Search for point with matching identifier
  Points::iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {
    Point point = *itr;
    if (point.identifier () == identifier) {
      m_points.erase (itr);
      break;
    }
  }
}

void Curve::saveXml(QXmlStreamWriter &writer) const
{
  LOG4CPP_INFO_S ((*mainCat)) << "Curve::saveXml";

  writer.writeStartElement(DOCUMENT_SERIALIZE_CURVE);
  writer.writeAttribute(DOCUMENT_SERIALIZE_CURVE_NAME, m_curveName);
  m_colorFilterSettings.saveXml (writer);
  m_curveStyle.saveXml (writer,
                        m_curveName);

  // Loop through points
  writer.writeStartElement(DOCUMENT_SERIALIZE_CURVE_POINTS);
  Points::const_iterator itr;
  for (itr = m_points.begin (); itr != m_points.end (); itr++) {
    const Point &point = *itr;
    point.saveXml (writer);
  }
  writer.writeEndElement();

  writer.writeEndElement();
}

void Curve::setColorFilterSettings (const ColorFilterSettings &colorFilterSettings)
{
  m_colorFilterSettings = colorFilterSettings;
}

void Curve::setCurveName (const QString &curveName)
{
  m_curveName = curveName;
}

void Curve::setCurveStyle (const CurveStyle &curveStyle)
{
  m_curveStyle = curveStyle;
}

void Curve::updatePointOrdinals ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "Curve::updatePointOrdinals";

  if (m_curveStyle.lineStyle().curveConnectAs() == CONNECT_AS_FUNCTION_SMOOTH ||
      m_curveStyle.lineStyle().curveConnectAs() == CONNECT_AS_FUNCTION_STRAIGHT) {

    // Make sure ordinals are properly ordered

    // Get a map of x/theta values as keys with point identifiers as the values
    XOrThetaToPointIdentifier xOrThetaToPointIdentifier;
    Points::iterator itr;
    for (itr = m_points.begin (); itr != m_points.end (); itr++) {
      Point &point = *itr;
      xOrThetaToPointIdentifier [point.posGraph().x()] = point.identifier();
    }

    // Since m_points is a list (and therefore does not provide direct access to elements), we build a temporary map of
    // point identifier to ordinal, by looping through the sorted x/theta values. Since QMap is used, the x/theta keys are sorted
    QMap<QString, double> pointIdentifierToOrdinal;
    int ordinal = 0;
    XOrThetaToPointIdentifier::const_iterator itrX;
    for (itrX = xOrThetaToPointIdentifier.begin(); itrX != xOrThetaToPointIdentifier.end(); itrX++) {

      QString pointIdentifier = itrX.value();
      pointIdentifierToOrdinal [pointIdentifier] = ordinal++;
    }

    // Override the old ordinal values
    for (itr = m_points.begin(); itr != m_points.end(); itr++) {
      Point &point = *itr;
      int ordinalNew = pointIdentifierToOrdinal [point.identifier()];
      point.setOrdinal (ordinalNew);
    }
  }
}

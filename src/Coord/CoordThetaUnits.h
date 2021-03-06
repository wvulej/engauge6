#ifndef COORD_THETA_UNITS_H
#define COORD_THETA_UNITS_H

#include <QString>

extern const QChar THETA;

enum CoordThetaUnits {
  COORD_THETA_UNITS_DEGREES,
  COORD_THETA_UNITS_DEGREES_MINUTES,
  COORD_THETA_UNITS_DEGREES_MINUTES_SECONDS,
  COORD_THETA_UNITS_GRADIANS,
  COORD_THETA_UNITS_RADIANS,
  COORD_THETA_UNITS_TURNS
};

extern QString coordThetaUnitsToString (CoordThetaUnits coordThetaUnits);

#endif // COORD_THETA_UNITS_H

#include "quaternion_math.h"

Vector::Vector() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
}

Vector::Vector(float in_x, float in_y, float in_z) {
  x = in_x;
  y = in_y;
  z = in_z;
}

float Vector::GetMagnitude() {
  return sqrt(x*x + y*y + z*z);
}

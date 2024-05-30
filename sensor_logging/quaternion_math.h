#ifndef QUATERNION_MATH_H
#define QUATERNION_MATH_H

#include <Arduino.h>

// Define Vector class and operators to help with calculations
class Vector {
public:
  float x;
  float y;
  float z;

  Vector();
  Vector(float in_x, float in_y, float in_z);

  float GetMagnitude();

  Vector operator+(const Vector& other) const {
        return Vector(x + other.x, y + other.y, z + other.z);
  }
  Vector operator*(const float& other) const {
        return Vector(x * other, y * other, z * other);
  }
  Vector operator/(const float& other) const {
        return Vector(x / other, y / other, z / other);
  }
};

#endif

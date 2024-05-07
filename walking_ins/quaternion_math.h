#ifndef QUATERNION_MATH_H
#define QUATERNION_MATH_H

#include <Arduino.h>

class Quaternion {
public:
  float w;
  float x;
  float y;
  float z;

  Quaternion();
  Quaternion(float in_w, float in_x, float in_y, float in_z);
  void Print();

  Quaternion GetProduct(Quaternion other);
  Quaternion GetConjugate();
  float GetMagnitude();
  void Normalize();
  Quaternion GetNormalized();
};

class Vector {
public:
  float x;
  float y;
  float z;

  Vector();
  Vector(float in_x, float in_y, float in_z);
  void Print();

  Quaternion ToQuaternion();
  float GetMagnitude();
  void Normalize();
  Vector GetNormalized();
  float DotProduct(Vector other);
  Vector CrossProduct(Vector other);
  Vector Average(Vector other);
  void Rotate(Quaternion q);
  Vector GetRotated(Quaternion q);

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

extern Quaternion GetRotationBetween(Vector a, Vector b);

#endif

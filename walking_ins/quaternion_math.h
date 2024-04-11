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
};

class Vector {
public:
  float x;
  float y;
  float z;

  Vector();
  Vector(float in_x, float in_y, float in_z);

  void Print();

  Vector operator+(const Vector& other) const {
        return Vector(x + other.x, y + other.y, z + other.z);
  }
  Vector operator/(const float& other) const {
        return Vector(x / other, y / other, z / other);
  }
};

extern Quaternion EulerToQuaternion(float roll, float pitch, float yaw);
extern float VectorDot(Vector v1, Vector v2);
extern float QuaternionDot(Quaternion q1, Quaternion q2);
extern Vector CrossProd(Vector v1, Vector v2);
extern Quaternion OffsetQ(Vector sensorRead, Vector g);

#endif

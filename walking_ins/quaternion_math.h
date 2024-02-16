#ifndef QUATERNION_MATH_H
#define QUATERNION_MATH_H

#include <math.h>
#include "Arduino.h"

class Quaternion {
public:
  float w;
  float x;
  float y;
  float z;
  
  Quaternion();
  Quaternion(float in_w, float in_x, float in_y, float in_z);

  void print();
};

extern Quaternion EulerToQuaternion(float roll, float pitch, float yaw);

#endif

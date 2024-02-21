#include "quaternion_math.h"

Quaternion::Quaternion() {
  w = 1.0f;
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
}

Quaternion::Quaternion(float in_w, float in_x, float in_y, float in_z) {
  w = in_w;
  x = in_x;
  y = in_y;
  z = in_z;
}

void Quaternion::Print() {
  Serial.print("[ ");
  Serial.print(w);
  Serial.print(", ");
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(", ");
  Serial.print(z);
  Serial.println(" ]");
}

Quaternion EulerToQuaternion(float roll, float pitch, float yaw)  // roll (x), pitch (y), yaw (z), angles are in degrees
{
  // Abbreviations for the various angular functions

  float cr = cos(roll * (2 * M_PI / 360) * 0.5);
  float sr = sin(roll * (2 * M_PI / 360) * 0.5);
  float cp = cos(pitch * (2 * M_PI / 360) * 0.5);
  float sp = sin(pitch * (2 * M_PI / 360) * 0.5);
  float cy = cos(yaw * (2 * M_PI / 360) * 0.5);
  float sy = sin(yaw * (2 * M_PI / 360) * 0.5);

  Quaternion q;
  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;

  return q;
}

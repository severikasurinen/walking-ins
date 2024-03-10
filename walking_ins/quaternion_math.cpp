#include "quaternion_math.h"
#include <cmath>

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

void Vector::Print() {
  Serial.print("[ ");
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

Quaternion OffsetQ(Vector sensorRead) // algorithm to generate the offset quaternion, where the vector sensorRead contains the linear accelerations returned by the sensor
{
  Vector g; //the normalized gravity vector
  g.x = 0;
  g.y = 0;
  g.z = 1;

  Vector c; //c for cross product
  c.x = sensorRead.y * g.z - sensorRead.z * g.y
  c.y = sensorRead.x * g.z - sensorRead.z * g.x
  c.z = sensorRead.x * g.y - sensorRead.y * g.x

  float norm_s = sensorRead.x * sensorRead.x + sensorRead.y * sensorRead.y + sensorRead.z * sensorRead.z
  float norm_g = 1 //norm of the normalized g is one
  float dot_sg = sensorRead.z //x and y components evaluate to zero

  Quaternion offset; //the unnormalized local to global rotation quaternion
  offset.w = sqrt(norm_s * norm_g) + dot_sg
  offset.x = c.x
  offset.y = c.y
  offset.z = c.z

  float norm_o = sqrt(offset.w * offset.w + offset.x * offset.x + offset.y * offset.y + offset.z * offset.z)

  Quaternion normalized_offset;
  normalized_offset.w = (1/norm_o)*offset.w
  normalized_offset.x = (1/norm_o)*offset.x
  normalized_offset.y = (1/norm_o)*offset.y
  normalized_offset.z = (1/norm_o)*offset.z

  return normalized_offset

}




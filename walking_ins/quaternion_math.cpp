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

float Quaternion::Dot(Quaternion other)
{
  float dot;
  dot = w * other.w + x * other.x + y * other.y + z * other.z;

  return dot;
}

float Quaternion::Norm()
{
  Quaternion q = Quaternion(w, x, y, z);
  return sqrt(q.Dot(q));
}

void Quaternion::Print() {
  Serial.print("Quaternion: [ ");
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
  Serial.print("Vector: [ ");
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(", ");
  Serial.print(z);
  Serial.println(" ]");
}

float Vector::Dot(Vector other) {
  return x * other.x + y * other.y + z * other.z;
}

Vector Vector::Cross(Vector other)
{
  Vector c;
  c.x = y * other.z - z * other.y;
  c.y = x * other.z - z * other.x;
  c.z = x * other.y - y * other.x;

  return c;
}

float Vector::Norm() {
  Vector v(x, y, z);
  return sqrt(v.Dot(v));
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

Quaternion OffsetQ(Vector sensorRead, Vector g) // algorithm to generate the offset quaternion from vectors sensorRead and g
{
  Vector c = sensorRead.Cross(g);

  float norm_s = sensorRead.Dot(sensorRead);
  float norm_g = g.Dot(g);
  float dot_sg = sensorRead.Dot(g);

  Quaternion offset; //the unnormalized local to global rotation quaternion
  offset.w = sqrt(norm_s * norm_g) + dot_sg;
  offset.x = c.x;
  offset.y = c.y;
  offset.z = c.z;

  float norm_o = offset.Norm();

  Quaternion normalized_offset;
  normalized_offset.w = (1/norm_o)*offset.w;
  normalized_offset.x = (1/norm_o)*offset.x;
  normalized_offset.y = (1/norm_o)*offset.y;
  normalized_offset.z = (1/norm_o)*offset.z;

  return normalized_offset;
}




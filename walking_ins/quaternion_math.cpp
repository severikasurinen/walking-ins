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

Quaternion Quaternion::GetProduct(Quaternion other) {
  return Quaternion(
    w * other.w - x * other.x - y * other.y - z * other.z,
    w * other.x + x * other.w - y * other.z + z * other.y,
    w * other.y + x * other.z + y * other.w - z * other.x,
    w * other.z - x * other.y + y * other.x + z * other.w);
}

Quaternion Quaternion::GetConjugate() {
  return Quaternion(w, -x, -y, -z);
}

float Quaternion::GetMagnitude() {
  return sqrt(w*w + x*x + y*y + z*z);
}

void Quaternion::Normalize() {
  float m = GetMagnitude();
  w /= m;
  x /= m;
  y /= m;
  z /= m;
}

Quaternion Quaternion::GetNormalized() {
  Quaternion n = Quaternion(w, x, y, z);
  n.Normalize();
  return n;
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

Quaternion Vector::ToQuaternion()  // roll (x), pitch (y), yaw (z), angles are in degrees
{
  // Abbreviations for the various angular functions

  float cr = cos(x * (2 * M_PI / 360) * 0.5);
  float sr = sin(x * (2 * M_PI / 360) * 0.5);
  float cp = cos(y * (2 * M_PI / 360) * 0.5);
  float sp = sin(y * (2 * M_PI / 360) * 0.5);
  float cy = cos(z * (2 * M_PI / 360) * 0.5);
  float sy = sin(z * (2 * M_PI / 360) * 0.5);

  Quaternion q;
  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;

  return q;
}

float Vector::GetMagnitude() {
  return sqrt(x*x + y*y + z*z);
}

void Vector::Normalize() {
  float m = GetMagnitude();
  x /= m;
  y /= m;
  z /= m;
}

Vector Vector::GetNormalized() {
  Vector n = Vector(x, y, z);
  n.Normalize();
  return n;
}

float Vector::DotProduct(Vector other) {
  return x * other.x + y * other.y + z * other.z;
}

Vector Vector::CrossProduct(Vector other) {
  return Vector(
    y * other.z - z * other.y,
    z * other.x - x * other.z,
    x * other.y - y * other.x);
}

void Vector::Rotate(Quaternion q) {
  Quaternion p = Quaternion(0, x, y, z);

  p = q.GetProduct(p);
  p = p.GetProduct(q.GetConjugate());

  x = p.x;
  y = p.y;
  z = p.z;
}

Vector Vector::GetRotated(Quaternion q) {
  Vector n = Vector(x, y, z);
  n.Rotate(q);
  return n;
}


Quaternion OffsetQ(Vector sensorRead, Vector g) // algorithm to generate the offset quaternion from vectors sensorRead and g
{
  Vector c = sensorRead.CrossProduct(g);

  float norm_s = sensorRead.DotProduct(sensorRead);
  float norm_g = g.DotProduct(g);
  float dot_sg = sensorRead.DotProduct(g);

  Quaternion offset; //the unnormalized local to global rotation quaternion
  offset.w = sqrt(norm_s * norm_g) + dot_sg;
  offset.x = c.x;
  offset.y = c.y;
  offset.z = c.z;

  float norm_o = offset.GetMagnitude();

  Quaternion normalized_offset;
  normalized_offset.w = (1/norm_o)*offset.w;
  normalized_offset.x = (1/norm_o)*offset.x;
  normalized_offset.y = (1/norm_o)*offset.y;
  normalized_offset.z = (1/norm_o)*offset.z;

  return normalized_offset;
}

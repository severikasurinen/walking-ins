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

Vector Vector::Average(Vector other) {
  return Vector((x + other.x) / 2.0, (y + other.y) / 2.0, (z + other.z) / 2.0);
}

void Vector::Rotate(Quaternion q) {
  Quaternion p = Quaternion(0, x, y, z);

  p = q.GetConjugate().GetProduct(p).GetProduct(q);

  x = p.x;
  y = p.y;
  z = p.z;
}

Vector Vector::GetRotated(Quaternion q) {
  Vector n = Vector(x, y, z);
  n.Rotate(q);
  return n;
}

Quaternion GetRotationBetween(Vector a, Vector b)
{
  Vector crossVal = a.CrossProduct(b);
  Quaternion q = { sqrt(pow(a.GetMagnitude(), 2.0) * pow(b.GetMagnitude(), 2.0)) + a.DotProduct(b), crossVal.x, crossVal.y, crossVal.z};
  return q.GetNormalized();
}

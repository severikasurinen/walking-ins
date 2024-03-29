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

Transform::Transform() {
  pos = Vector();
  rot = Quaternion();
}

void Transform::Print() {
  pos.Print();
  Serial.println("-");
  rot.Print();
}

Transform::Transform(Vector in_pos, Quaternion in_rot) {
  pos = in_pos;
  rot = in_rot;
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

float VectorDot(Vector v1, Vector v2) 
{
  float dot;
  dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float QuaternionDot(Quaternion q1, Quaternion q2)
{
  float dot;
  dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
}

float Cross(Vector v1, Vector v2)
{
  Vector c;
  c.x = v1.y * v2.z - v1.z * v2.y;
  c.y = v1.x * v2.z - v1.z * v2.x;
  c.z = v1.x * v2.y - v1.y * v2.x;
}

Quaternion OffsetQ(Vector sensorRead, Vector g) // algorithm to generate the offset quaternion from vectors sensorRead and g
{

  Vector c; //c for cross product
  c = Cross(sensorRead, g);

  float norm_s = VectorDot(sensorRead, sensorRead);
  float norm_g = VectorDot(g,g);
  float dot_sg = VectorDot(sensorRead, g); 

  Quaternion offset; //the unnormalized local to global rotation quaternion
  offset.w = sqrt(norm_s * norm_g) + dot_sg;
  offset.x = c.x;
  offset.y = c.y;
  offset.z = c.z;

  float norm_o = sqrt(QuaternionDot(offset, offset));

  Quaternion normalized_offset;
  normalized_offset.w = (1/norm_o)*offset.w;
  normalized_offset.x = (1/norm_o)*offset.x;
  normalized_offset.y = (1/norm_o)*offset.y;
  normalized_offset.z = (1/norm_o)*offset.z;

  return normalized_offset;

}




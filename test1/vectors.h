#pragma once

#include <cmath>

template<int size>
class Vector
{
public:
  Vector()
  {
    for (int i = 0; i < size; ++i) {
      coords[i] = 0;
    }
  }

  explicit Vector(float v)
  {
    for (int i = 0; i < size; ++i) {
      coords[i] = v;
    }
  }

  float coords[size];

  float norm_sq() const
  {
    return *this * *this;
  }

  float norm() const
  {
    return sqrt(this->norm_sq());
  }

  Vector<size> normalize() const
  {
    return (1.0/this->norm()) * *this;
  }

  float &operator[](int i)
  {
    return coords[i];
  }

  float operator[](int i) const
  {
    return coords[i];
  }

  Vector<size> const &operator+=(Vector<size> const &b)
  {
    for (int i = 0; i < size; ++i)
      coords[i] += b.coords[i];
    return *this;
  }

  Vector<size> const &operator*=(float b)
  {
    for (int i = 0; i < size; ++i)
      coords[i] *= b;
    return *this;
  }
};

// Dot product
template<int size>
inline float operator*(Vector<size> const &a, Vector<size> const &b)
{
  float result = 0.0;
  for (int i = 0; i < size; ++i)
    result += a[i] * b[i];
  return result;
}

// Scalar/vector product
template<int size>
inline Vector<size> operator*(float a, Vector<size> const &b)
{
  Vector<size> result(b);
  return result *= a;
}

// Cross product
inline Vector<3> operator%(Vector<3> const &a, Vector<3> const &b)
{
  Vector<3> result;
  for (int i = 0; i < 3; ++i)
    result[i] = a[(i+1)%3] * b[(i+2)%3] - a[(i+2)%3] * b[(i+1)%3];
  return result;
}

// Vector addition
template<int size>
inline Vector<size> operator+(Vector<size> const &a, Vector<size> const &b)
{
  Vector<size> result(a);
  return result += b;
}

// Vector subtraction
template<int size>
inline Vector<size> operator-(Vector<size> const &a, Vector<size> const &b)
{
  Vector<size> result;
  for (int i = 0; i < size; ++i)
    result[i] = a[i] - b[i];
  return result;
}

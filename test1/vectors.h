template<int size>
class Vector
{
public:
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
inline float operator*(float a, Vector<size> const &b)
{
  Vector<size> result;
  for (int i = 0; i < size; ++i)
    result[i] = a * b[i];
  return result;
}

// Cross product
inline Vector<3> operator%(Vector<size> const &a, Vector<size> const &b)
{
  Vector<3> result;
  for (int i = 0; i < size; ++i)
    result[i] = a[(i+1)%3] * b[(i+2)%3] - a[(i+2)%3] * b[(i+1)%3];
  return result;
}

// Vector addition
template<int size>
inline Vector<size> operator+(Vector<size> const &a, Vector<size> const &b)
{
  Vector<size> result;
  for (int i = 0; i < size; ++i)
    result[i] = a[i] + b[i];
  return result;
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

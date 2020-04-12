#ifndef VECTOR2_HPP
#   define VECTOR2_HPP


struct Vector2
{
    int x = 0;
    int y = 0;
};

// Operators.

inline Vector2 operator+(const Vector2& a, const Vector2& b) noexcept
{
    return {a.x + b.x, a.y + b.y};
}


#endif // VECTOR2_HPP
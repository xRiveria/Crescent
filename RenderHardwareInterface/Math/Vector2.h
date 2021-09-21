#pragma once

struct Vector2
{
    Vector2()
    {
        this->x = 0.0f;
        this->y = 0.0f;
    }

    Vector2(float x, float y)
    {
        this->x = x;
        this->y = y;
    }

    bool operator==(const Vector2& otherVector)
    {
        return otherVector.x == this->x && otherVector.y == this->y;
    }

    bool operator !=(const Vector2& otherVector)
    {
        return otherVector.x != this->x || otherVector.y != this->y;
    }

    float x = 0.0f;
    float y = 0.0f;

    static Vector2 Zero;
};


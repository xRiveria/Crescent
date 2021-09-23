#pragma once

namespace Math::Utilities
{
    template <typename T>
    T Min(T comparison1, T comparison2)
    {
        if (comparison1 > comparison2) { return comparison2; } else { return comparison1; }
    }
}
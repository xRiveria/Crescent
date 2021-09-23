#pragma once

// STL
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>

// Internal
#include "../Math/MathUtilities.h"

#define RHI_ASSERT_STATEMENT(statement)                   \
if (!(statement))                                         \
{                                                         \
    throw std::invalid_argument("Assertion Failed!");     \
}                                                         

#define RHI_ASSERT_DATA(objectPointer)                \
if (objectPointer == nullptr)                         \
{                                                     \
    throw std::invalid_argument("Pointer Invalid");   \
}
#pragma once

// STL
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>

#define RHI_ASSERT_STATEMENT(statement)                   \
if (!(statement))                                         \
{                                                         \
    std::cout << "Assertion Failed!\n";                     \
}                                                         

#define RHI_ASSERT_DATA(objectPointer)                \
if (objectPointer == nullptr)                         \
{                                                     \
    std::cout << "Invalid pointer to object!\n";      \
}
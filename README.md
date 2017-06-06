# STM32F103DriverLib
Easier way to program on STM32F103 in C++
======

To make program on STM32 more easier, we packet every peripheral and module as a C++ class, mask operation steps instead of function-oriented interface.

> eg: How to use USART to send data
```cpp
# include "USART.h"
USART com(1,115200);
void main()
{
    com<<"test string\r\n";
}
```

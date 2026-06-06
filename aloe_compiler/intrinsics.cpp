#include "pch.h"
#include <stdio.h>

extern "C" void vera(const char* msg)
{
	printf("VERA: %s\n", msg);
}

#include "pch.h"

extern  
void aloe::log1(FILE* stream, const char* text)
{
	fprintf(stream, text);
	fflush(stream);
}
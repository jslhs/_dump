#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cstdarg>

void ctrace(const char *fmt, ...)
{
	char buf[256] = {0};
	va_list ap;
	va_start(ap, fmt);

	vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);

	//va_end(ap);

	OutputDebugStringA(buf);
	printf_s(buf);
}

void ctrace(const wchar_t *fmt, ...)
{
	wchar_t buf[256] = {0};
	va_list ap;
	va_start(ap, fmt);

	_vsnwprintf_s(buf, sizeof(buf) / sizeof(wchar_t), _TRUNCATE, fmt, ap);

 	//va_end(ap);

	OutputDebugStringW(buf);
	wprintf_s(buf);
}

#pragma once

#include <sys/types.h>

int iswctype(wint_t wc, wctype_t desc);
int iswlower(wint_t wc);
int iswupper(wint_t wc);

wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);

wctype_t wctype(const char* name);
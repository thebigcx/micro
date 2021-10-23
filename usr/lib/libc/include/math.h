#pragma once

#define HUGE_VAL __builtin_huge_val()

double pow(double x, double y);
double ceil(double x);
double floor(double x);
double log(double x);
double fabs(double x);
double sqrt(double x);
double fmod(double x, double y);
double trunc(double d);
double log10(double x);
double log2(double x);
double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double x, double y);
double frexp(double x, int* exp);
double exp(double x);
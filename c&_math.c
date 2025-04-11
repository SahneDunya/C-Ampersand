#include "c&_math.h"
#include <math.h>   // Standart C matematik fonksiyonları
#include <stdlib.h> // abs fonksiyonu için

int c_ampersand_abs(int n) {
    return abs(n);
}

double c_ampersand_sqrt(double x) {
    return sqrt(x);
}

double c_ampersand_pow(double x, double y) {
    return pow(x, y);
}

double c_ampersand_sin(double x) {
    return sin(x);
}

double c_ampersand_cos(double x) {
    return cos(x);
}

double c_ampersand_tan(double x) {
    return tan(x);
}

double c_ampersand_floor(double x) {
    return floor(x);
}

double c_ampersand_ceil(double x) {
    return ceil(x);
}

double c_ampersand_round(double x) {
    return round(x);
}

double c_ampersand_log(double x) {
    return log(x);
}

double c_ampersand_log10(double x) {
    return log10(x);
}

// ... diğer matematiksel fonksiyonların implementasyonları buraya gelebilir
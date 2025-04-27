#include "c&_math.h"

// Standart C matematik fonksiyonları.
// ÖNEMLİ: Bu implementasyon, SAHNE64'ün C araç zincirinin
// <math.h> ve <stdlib.h>'de bildirilen standart C matematik
// fonksiyonlarının no_std ortamında kullanılabilen bir
// implementasyonunu (libm) sağladığı varsayımına dayanmaktadır.
// Aksi takdirde, bu çağrılar derlenemez veya çalışma zamanında hata verir.
#include <math.h>   // sqrt, pow, sin, cos, tan, floor, ceil, round, log, log10 için
#include <stdlib.h> // abs için (C99 ve sonrası math.h de içerebilir)


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

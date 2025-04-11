#ifndef C_AMPERSAND_MATH_H
#define C_AMPERSAND_MATH_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <math.h>   // Standart C matematik fonksiyonları için (gerekli tanımlar)

// Pi sabiti
#define C_AMPERSAND_PI 3.14159265358979323846

// Mutlak değer (integer için)
int c_ampersand_abs(int n);

// Karekök
double c_ampersand_sqrt(double x);

// Üs alma (x üzeri y)
double c_ampersand_pow(double x, double y);

// Sinüs
double c_ampersand_sin(double x);

// Kosinüs
double c_ampersand_cos(double x);

// Tanjant
double c_ampersand_tan(double x);

// Taban fonksiyonu (kendisine eşit veya kendisinden küçük en büyük tam sayı)
double c_ampersand_floor(double x);

// Tavan fonksiyonu (kendisine eşit veya kendisinden büyük en küçük tam sayı)
double c_ampersand_ceil(double x);

// En yakın tam sayıya yuvarlama
double c_ampersand_round(double x);

// Logaritma (doğal logaritma - ln)
double c_ampersand_log(double x);

// 10 tabanlı logaritma
double c_ampersand_log10(double x);

// ... diğer matematiksel fonksiyonlar eklenebilir (örneğin, trigonometrik ters fonksiyonlar, hiperbolik fonksiyonlar vb.)

#endif // C_AMPERSAND_MATH_H
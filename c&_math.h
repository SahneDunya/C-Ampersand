#ifndef C_AMPERSAND_MATH_H
#define C_AMPERSAND_MATH_H

// Temel C& tipler ve hata yönetimi için (bu modülde doğrudan kullanılmıyor olabilir ama standart dahil etme)
#include "c&_stdlib.h"

// Standart C matematik fonksiyonları için tanımlar.
// ÖNEMLİ: SAHNE64'ün C araç zincirinin <math.h>'de bildirilen
// fonksiyonların no_std ortamında kullanılabilen bir implementasyonunu (libm)
// sağladığı varsayılmaktadır. Aksi takdirde, bu fonksiyonların
// C& runtime içinde yazılımsal olarak implemente edilmesi gerekir.
#include <math.h>

// abs fonksiyonu için stdlib.h'ye ihtiyaç duyulabilir (C89/C90 uyumluluğu için)
// C99 ve sonrası için <math.h> de abs içerebilir ama stdlib.h dahil etmek daha güvenlidir.
#include <stdlib.h> // abs için

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

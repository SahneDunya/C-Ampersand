#ifndef C_AMPERSAND_TIME_H
#define C_AMPERSAND_TIME_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için
#include <stdint.h> // uint64_t gibi kesin boyutlu tipler için

// Zaman damgası için tip tanımı (milisaniye cinsinden Unix epoch'tan itibaren)
typedef uint64_t c_ampersand_timestamp;

// Şu anki zamanı (timestamp olarak) alır.
c_ampersand_result c_ampersand_time_now(c_ampersand_timestamp *timestamp);

// Belirli bir süre bekler (milisaniye cinsinden).
// Not: Bu fonksiyon c&_process.h'de de tanımlanmıştır. İhtiyaca göre burada tekrar tanımlanabilir
// veya sadece c&_process.h'den kullanılabilir.
c_ampersand_result c_ampersand_time_sleep(uint64_t milliseconds);

// ... diğer zaman yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, formatlama, farklı zaman birimlerine dönüştürme)

#endif // C_AMPERSAND_TIME_H
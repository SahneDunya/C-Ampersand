#ifndef C_AMPERSAND_TIME_H
#define C_AMPERSAND_TIME_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result)
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (u64 tipi ve sahne_error_t için)
#include "sahne.h"
// uint64_t gibi kesin boyutlu tipler için (c&_stdlib.h'de de var)
#include <stdint.h>

// Zaman damgası için tip tanımı (milisaniye cinsinden Unix epoch'tan itibaren)
typedef uint64_t c_ampersand_timestamp;

// Şu anki zamanı (timestamp olarak) alır.
// timestamp: Şu anki zamanın (milisaniye cinsinden) yazılacağı uint64_t* işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_time_now(c_ampersand_timestamp *timestamp);

// Belirli bir süre bekler (milisaniye cinsinden).
// Bu fonksiyon genellikle c&_process.h'deki c_ampersand_process_sleep fonksiyonunu çağırır.
// milliseconds: Sürecin/thread'in uyuyacağı milisaniye cinsinden süre.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_time_sleep(uint64_t milliseconds);

// ... diğer zaman yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, formatlama, farklı zaman birimlerine dönüştürme)

#endif // C_AMPERSAND_TIME_H

#ifndef C_AMPERSAND_PROCESS_H
#define C_AMPERSAND_PROCESS_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (u64 tipi için)
#include "sahne.h"

// Süreç ID'si tipi (Sahne64'ün Handle tipiyle tutarlı u64 kullanır)
typedef u64 c_ampersand_process_id;

// Geçersiz Süreç ID değeri (Varsayım: Sahne64'te 0 geçersizdir)
#define C_AMPERSAND_INVALID_PROCESS_ID 0


// Yeni bir süreç oluşturur (spawn).
// path: Çalıştırılacak programın dosya yolu.
// pid: Oluşturulan sürecin ID'sinin yazılacağı u64* işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Dosya bulunamadı, izin hatası, bellek yetersizliği gibi hataları raporlar.
c_ampersand_result c_ampersand_process_create(const char *path, c_ampersand_process_id *pid);

// Mevcut süreci sonlandırır. Bu fonksiyon geri dönmez.
// code: Sürecin çıkış kodu.
void c_ampersand_process_exit(int code);

// Mevcut (çağıran) sürecin ID'sini alır.
// pid: Mevcut sürecin ID'sinin yazılacağı u64* işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_process_get_pid(c_ampersand_process_id *pid);

// Süreci belirli bir süre uyutur (bloklar).
// milliseconds: Sürecin uyuyacağı milisaniye cinsinden süre.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Uyku mekanizması hatası gibi durumları raporlar.
c_ampersand_result c_ampersand_process_sleep(uint64_t milliseconds);

// ... diğer süreç yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, sinyal gönderme, süreci bekleme (wait) vb.)

#endif // C_AMPERSAND_PROCESS_H

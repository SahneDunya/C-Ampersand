#ifndef C_AMPERSAND_PROCESS_H
#define C_AMPERSAND_PROCESS_H

#include "c&_stdlib.h" // Temel tipler ve hata yönetimi için

// Süreç ID'si
typedef sahne64_pid_t c_ampersand_process_id;

// Yeni bir süreç oluşturur.
c_ampersand_result c_ampersand_process_create(const char *path, c_ampersand_process_id *pid);

// Mevcut süreci sonlandırır.
void c_ampersand_process_exit(int code);

// Mevcut sürecin ID'sini alır.
c_ampersand_result c_ampersand_process_get_pid(c_ampersand_process_id *pid);

// Süreci belirli bir süre uyutur.
c_ampersand_result c_ampersand_process_sleep(uint64_t milliseconds);

// ... diğer süreç yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, sinyal gönderme, bekleme vb.)

#endif // C_AMPERSAND_PROCESS_H
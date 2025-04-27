#ifndef C_AMPERSAND_THREAD_H
#define C_AMPERSAND_THREAD_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result)
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (u64 tipi ve sahne_error_t için)
#include "sahne.h"


// Thread ID'si tipi (Sahne64'ün Handle tipiyle tutarlı u64 kullanır)
typedef u64 c_ampersand_thread_id;

// Geçersiz Thread ID değeri (Varsayım: Sahne64'te 0 geçersizdir)
#define C_AMPERSAND_INVALID_THREAD_ID 0

// Thread giriş noktası fonksiyon tipi
typedef void (*c_ampersand_thread_entry_point)(void *);


// Yeni bir thread oluşturur (spawn).
// entry_point: Thread'in çalıştıracağı fonksiyon.
// stack_size: Thread'in kullanacağı stack boyutu (0 varsayılan olabilir).
// arg: Thread giriş fonksiyonuna iletilecek argüman pointer'ı.
// thread_id: Oluşturulan thread'in ID'sinin yazılacağı u64* işaretçi.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Bellek yetersizliği, geçersiz argüman gibi hataları raporlar.
c_ampersand_result c_ampersand_thread_create(c_ampersand_thread_entry_point entry_point, size_t stack_size, void *arg, c_ampersand_thread_id *thread_id);

// Mevcut (çağıran) thread'i sonlandırır. Bu fonksiyon geri dönmez.
// code: Thread'in çıkış kodu.
void c_ampersand_thread_exit(int code);

// ... diğer thread yönetimi ile ilgili fonksiyonlar eklenebilir (örneğin, thread'e katılma (join), ID alma vb.)


#endif // C_AMPERSAND_THREAD_H

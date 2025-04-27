#ifndef C_AMPERSAND_PACKAGE_FORMAT_H
#define C_AMPERSAND_PACKAGE_FORMAT_H

// Temel C& tipler ve hata yönetimi için (c_ampersand_result, C_AMPERSAND_* hataları)
#include "c&_stdlib.h"
// C& G/Ç işlemleri ve Handle tipi için (c_ampersand_io_open, c_ampersand_file_handle)
#include "c&_io_operations.h" // c_ampersand_file_handle tipi u64 olmalıdır.

#include <stddef.h> // size_t için
#include <stdbool.h> // bool için

// C Ampersand paketinin temel meta bilgilerini tutan yapı (heap'te ayrılmış stringler içerir)
typedef struct {
    char *name;
    size_t name_allocated_size; // name stringi için ayrılan boyut (free için)
    char *version;
    size_t version_allocated_size; // version stringi için ayrılan boyut (free için)
    char *description;
    size_t description_allocated_size; // description stringi için ayrılan boyut (free için)
    // ... diğer meta bilgiler (yazarlar, bağımlılıklar vb.) eklenebilir
} c_ampersand_package_metadata;

// Bir C Ampersand paket dosyasını açar (salt okunur modda).
// file_path: Açılacak paket dosyasının yolu.
// package_handle: Açılan paket dosyasının Handle'ının yazılacağı pointer.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_package_open(const char *file_path, c_ampersand_file_handle *package_handle);

// Bir C Ampersand paket dosyasını kapatır.
// package_handle: Kapatılacak paket dosyasının Handle'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_package_close(c_ampersand_file_handle package_handle);

// Açık bir C Ampersand paketinden meta bilgileri okur ve metadata yapısına doldurur.
// metadata yapısındaki string alanları heap'te tahsis edilir ve serbest bırakılmalıdır.
// package_handle: Meta bilginin okunacağı paket dosyasının Handle'ı.
// metadata: Okunan meta bilginin yazılacağı yapı pointer'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Paket formatı hatası, I/O hatası, bellek hatası içerebilir.
c_ampersand_result c_ampersand_package_read_metadata(c_ampersand_file_handle package_handle, c_ampersand_package_metadata *metadata);

// Bir c_ampersand_package_metadata yapısındaki heap'te ayrılmış string alanlarını serbest bırakır.
// metadata: Serbest bırakılacak yapı pointer'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result.
c_ampersand_result c_ampersand_package_free_metadata(c_ampersand_package_metadata *metadata);


// Açık bir C Ampersand paketinden belirli bir dosyayı çıkarır.
// package_handle: Dosyanın çıkarılacağı paket dosyasının Handle'ı.
// file_name: Paketten çıkarılacak dosyanın adı.
// destination_path: Dosyanın çıkarılacağı hedef dosya yolu.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Paket formatı hatası, I/O hatası içerebilir.
c_ampersand_result c_ampersand_package_extract_file(c_ampersand_file_handle package_handle, const char *file_name, const char *destination_path);

// Açık bir C Ampersand paketindeki tüm dosyaların adlarının listesini yazdırır (şimdilik stdout'a).
// package_handle: Dosyaların listeleneceği paket dosyasının Handle'ı.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Paket formatı hatası, I/O hatası içerebilir.
c_ampersand_result c_ampersand_package_list_files(c_ampersand_file_handle package_handle);

// Bir dizindeki dosyaları kullanarak yeni bir C Ampersand paketi oluşturur.
// source_directory: Paketlenecek dosyaların bulunduğu dizin yolu.
// output_file_path: Oluşturulacak paket dosyasının yolu.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Dizin okuma hatası, I/O hatası, bellek hatası içerebilir.
c_ampersand_result c_ampersand_package_create(const char *source_directory, const char *output_file_path);

// Bir C Ampersand paket dosyasının temel format geçerliliğini kontrol eder.
// file_path: Doğrulanacak paket dosyasının yolu.
// Dönüş: Başarı veya hata belirten c_ampersand_result. Geçersiz format, I/O hatası içerebilir.
c_ampersand_result c_ampersand_package_validate(const char *file_path);

// ... diğer paket formatı ile ilgili fonksiyonlar eklenebilir (örneğin, imza kontrolü)

#endif // C_AMPERSAND_PACKAGE_FORMAT_H

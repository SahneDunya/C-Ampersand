#include "c&_package_format.h"
// c_ampersand_result, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT, c_ampersand_println, c_ampersand_error_with_message için
#include "c&_stdlib.h"
// Sahne64 API başlık dosyası (sahne_resource_seek ve sahne_error_t için)
#include "sahne.h"
// C& G/Ç operasyonları (c_ampersand_io_*, c_ampersand_file_handle, C_AMPERSAND_O_* bayrakları için)
#include "c&_io_operations.h"
// C& bellek yönetimi fonksiyonları için
#include "c&_memory.h"

#include <string.h> // strlen, strcmp, memcpy için
#include <stdint.h> // s64, u32 gibi tipler için (sahne.h de sağlayabilir)

// --- Dahili Sabitler ve Yapılar ---

// Basit bir paket formatı varsayımı:
// Dosya, metin bazlı markerlar ([METADATA], [FILE], vb.) kullanarak yapılandırılmıştır.
// Her [FILE] bloğu, dosya adı bilgisini ve ardından dosya içeriğini içerir.

#define C_AMPERSAND_PACKAGE_METADATA_START "[METADATA]\n" // Yeni satır ekleyelim
#define C_AMPERSAND_PACKAGE_METADATA_END   "[/METADATA]\n" // Yeni satır ekleyelim
#define C_AMPERSAND_PACKAGE_FILE_START     "[FILE]\n"     // Yeni satır ekleyelim
#define C_AMPERSAND_PACKAGE_FILE_END       "[/FILE]\n"     // Yeni satır ekleyelim
#define C_AMPERSAND_PACKAGE_FILENAME_KEY   "filename=" // Anahtar=Değer formatı

// Paket formatı hataları için özel hata kodu tanımlayalım (varsayım: c&_stdlib.h'de tanımlı değilse)
#ifndef C_AMPERSAND_ERROR_PACKAGE_FORMAT
#define C_AMPERSAND_ERROR_PACKAGE_FORMAT (C_AMPERSAND_ERROR_CUSTOM_START + 0) // Veya uygun bir değer
#endif

// Helper function to map Sahne64 error codes to C& error codes
// Varsayım: map_sahne_error_to_camper_error fonksiyonu c&_memory.c veya başka bir common yerde tanımlı
static c_ampersand_error_code map_sahne_error_to_camper_error(sahne_error_t sahne_err);


// --- Yardımcı Fonksiyonlar ---

// Bir c_ampersand_package_metadata yapısındaki heap'te ayrılmış string alanlarını serbest bırakır.
// c_ampersand_free kullanır.
c_ampersand_result c_ampersand_package_free_metadata(c_ampersand_package_metadata *metadata) {
    if (metadata == NULL) {
        return C_AMPERSAND_OK_RESULT; // NULL metadata yapısını serbest bırakmak güvenlidir
    }

    c_ampersand_result final_result = C_AMPERSAND_OK_RESULT;

    // name stringini serbest bırak
    if (metadata->name != NULL && metadata->name_allocated_size > 0) {
        c_ampersand_result res = c_ampersand_free(metadata->name, metadata->name_allocated_size);
        if (res.code != C_AMPERSAND_OK) { if (final_result.code == C_AMPERSAND_OK) final_result = res; }
        metadata->name = NULL;
        metadata->name_allocated_size = 0;
    }
    // version stringini serbest bırak
    if (metadata->version != NULL && metadata->version_allocated_size > 0) {
         c_ampersand_result res = c_ampersand_free(metadata->version, metadata->version_allocated_size);
        if (res.code != C_AMPERSAND_OK) { if (final_result.code == C_AMPERSAND_OK) final_result = res; }
        metadata->version = NULL;
        metadata->version_allocated_size = 0;
    }
    // description stringini serbest bırak
    if (metadata->description != NULL && metadata->description_allocated_size > 0) {
         c_ampersand_result res = c_ampersand_free(metadata->description, metadata->description_allocated_size);
        if (res.code != C_AMPERSAND_OK) { if (final_result.code == C_AMPERSAND_OK) final_result = res; }
        metadata->description = NULL;
        metadata->description_allocated_size = 0;
    }

    return final_result; // Serbest bırakma işlemlerinin sonucunu döndür
}


// Bir paket dosyasından meta bilgileri okur ve metadata yapısına doldurur.
// package_handle: Dosyanın Handle'ı. Dosya okuma konumundan itibaren okur.
// metadata: Okunan bilginin yazılacağı yapı. Stringler heap'te tahsis edilir.
// Dönüş: Başarı veya hata (format hatası, I/O hatası, bellek hatası).
// Basit format varsayımıyla implementasyon taslağı.
static c_ampersand_result read_metadata_from_handle(c_ampersand_file_handle package_handle, c_ampersand_package_metadata *metadata) {
    if (metadata == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // metadata yapısını başlangıçta sıfırla
    memset(metadata, 0, sizeof(c_ampersand_package_metadata));

    char line_buffer[512]; // Bir satır okuma buffer'ı
    size_t bytes_read;
    c_ampersand_result res;

    // [METADATA] marker'ını oku ve doğrula
    res = c_ampersand_io_read_line(package_handle, line_buffer, sizeof(line_buffer), &bytes_read);
    if (res.code != C_AMPERSAND_OK) {
        // I/O hatası veya dosya sonu (metadata beklenirken)
        return c_ampersand_error_with_message(res.code, "Paket metadata başlangıç markerı okunamadı.");
    }
    if (strcmp(line_buffer, C_AMPERSAND_PACKAGE_METADATA_START) != 0) {
        // Beklenen marker bulunamadı, format hatası
        return c_ampersand_error_with_message(C_AMPERSAND_ERROR_PACKAGE_FORMAT, "Geçersiz paket formatı: Meta bilgi başlangıç markerı (%s) bekleniyordu.", C_AMPERSAND_PACKAGE_METADATA_START);
    }

    // Meta bilgi satırlarını oku
    while (1) {
        res = c_ampersand_io_read_line(package_handle, line_buffer, sizeof(line_buffer), &bytes_read);
        if (res.code != C_AMPERSAND_OK) {
             // I/O hatası veya dosya sonu (meta bilgi beklenirken)
             // Kısmen okunan metadatayı temizlememiz gerekir.
             c_ampersand_package_free_metadata(metadata);
             return c_ampersand_error_with_message(res.code, "Paket meta bilgi içeriği okunamadı.");
        }

        // [/METADATA] marker'ını bulduk mu?
        if (strcmp(line_buffer, C_AMPERSAND_PACKAGE_METADATA_END) == 0) {
            break; // Meta bilgi sonu, döngüden çık
        }

        // Anahtar=Değer formatında satırları ayrıştır (basit implementasyon)
        // Bu kısım daha sağlam hale getirilmelidir.
        char *key_start = line_buffer;
        char *value_start = strchr(line_buffer, '=');
        if (value_start == NULL) {
            // '=' bulunamadı, format hatası
            c_ampersand_package_free_metadata(metadata);
            return c_ampersand_error_with_message(C_AMPERSAND_ERROR_PACKAGE_FORMAT, "Geçersiz paket formatı: Meta bilgi satırı anahtar=değer formatında değil.");
        }
        *value_start = '\0'; // Anahtar ve değer arasına null sonlandırıcı koy
        value_start++; // Değerin başladığı yere ilerle

        // Anahtara göre ilgili metadata alanını doldur
        // Değer stringini kopyala ve boyutu kaydet (c_ampersand_allocate kullan)
        size_t value_len = strlen(value_start);
        size_t alloc_size = value_len + 1; // null sonlandırıcı dahil

        if (strcmp(key_start, "name") == 0) {
            if (metadata->name != NULL) { /* Zaten set edilmiş, hata veya overwrite? */ }
            c_ampersand_allocate(alloc_size, (void**)&metadata->name);
            if (metadata->name == NULL) { c_ampersand_package_free_metadata(metadata); return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, NULL, 0}; }
            memcpy(metadata->name, value_start, alloc_size);
            metadata->name_allocated_size = alloc_size;
        } else if (strcmp(key_start, "version") == 0) {
            if (metadata->version != NULL) { /* Zaten set edilmiş */ }
            c_ampersand_allocate(alloc_size, (void**)&metadata->version);
            if (metadata->version == NULL) { c_ampersand_package_free_metadata(metadata); return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, NULL, 0}; }
            memcpy(metadata->version, value_start, alloc_size);
            metadata->version_allocated_size = alloc_size;
        } else if (strcmp(key_start, "description") == 0) {
             if (metadata->description != NULL) { /* Zaten set edilmiş */ }
            c_ampersand_allocate(alloc_size, (void**)&metadata->description);
            if (metadata->description == NULL) { c_ampersand_package_free_metadata(metadata); return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY, NULL, 0}; }
            memcpy(metadata->description, value_start, alloc_size);
            metadata->description_allocated_size = alloc_size;
        }
        // ... diğer meta bilgi alanları için eklemeler
    }

    // Tüm meta bilgiler okundu ve ayrıştırıldı
    return C_AMPERSAND_OK_RESULT;
}


// --- Paket Formatı Fonksiyonlarının Implementasyonu ---

// Bir C Ampersand paket dosyasını açar (salt okunur modda).
// c_ampersand_io_open kullanır.
c_ampersand_result c_ampersand_package_open(const char *file_path, c_ampersand_file_handle *package_handle) {
    // Parametre doğrulama
    if (file_path == NULL || package_handle == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    *package_handle = C_AMPERSAND_INVALID_FILE_HANDLE; // Başlangıçta geçersiz Handle

    // Dosyayı salt okunur modda aç
    // c_ampersand_io_open Sahne64 kaynak API'sini sarmalar ve c_ampersand_result döner.
    c_ampersand_result open_res = c_ampersand_io_open(file_path, C_AMPERSAND_O_RDONLY, package_handle);

    if (open_res.code != C_AMPERSAND_OK) {
        // Açma hatasını c_ampersand_error_with_message ile sarmalayabiliriz veya olduğu gibi iletebiliriz.
        // c_ampersand_io_open zaten hata mesajı içerebilir (eğer sarmalama öyle implemente edildiyse).
        // Şimdilik olduğu gibi iletelim.
        return open_res; // Dosya açma hatasını ilet (örn. FILE_NOT_FOUND, PERMISSION_DENIED)
    }

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Bir C Ampersand paket dosyasını kapatır.
// c_ampersand_io_close kullanır.
c_ampersand_result c_ampersand_package_close(c_ampersand_file_handle package_handle) {
     // Parametre doğrulama
    if (package_handle == C_AMPERSAND_INVALID_FILE_HANDLE) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0}; // Geçersiz Handle
    }

    // Dosyayı kapat
    // c_ampersand_io_close Sahne64 kaynak API'sini sarmalar ve c_ampersand_result döner.
    c_ampersand_result close_res = c_ampersand_io_close(package_handle);

    if (close_res.code != C_AMPERSAND_OK) {
        // Kapatma hatasını olduğu gibi iletelim
        return close_res; // Dosya kapatma hatasını ilet
    }

    return C_AMPERSAND_OK_RESULT; // Başarı
}

// Açık bir C Ampersand paketinden meta bilgileri okur.
// Dosyanın başına geri döner (sahne_resource_seek kullanarak) ve sonra meta bilgiyi okur.
c_ampersand_result c_ampersand_package_read_metadata(c_ampersand_file_handle package_handle, c_ampersand_package_metadata *metadata) {
    // Parametre doğrulama
    if (package_handle == C_AMPERSAND_INVALID_FILE_HANDLE || metadata == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }
    // metadata yapısını başlangıçta sıfırla (free edilebilmesi için gerekli)
    memset(metadata, 0, sizeof(c_ampersand_package_metadata));

    // Dosyanın başına geri dön
    // Varsayım: sahne_resource_seek sahne.h'de tanımlı, sahne_error_t döner.
    sahne_error_t seek_err = sahne_resource_seek(package_handle, 0, SEEK_SET); // OFFSET 0, ORIGIN SEEK_SET
    if (seek_err != SAHNE_SUCCESS) {
        // Seek hatasını c_ampersand_result'a çevir ve döndür
        // map_sahne_error_to_camper_error fonksiyonu gereklidir.
        c_ampersand_error_code err_code = map_sahne_error_to_camper_error(seek_err);
        return c_ampersand_error_with_message(err_code, "Paket dosyasında seek hatası (metadata okuma başı). SAHNE hatası: %d", seek_err);
    }

    // Meta bilgiyi dosyadan oku (yardımcı fonksiyonu kullan)
    return read_metadata_from_handle(package_handle, metadata); // Hata olursa olduğu gibi iletilir
}

// Açık bir C Ampersand paketinden belirli bir dosyayı çıkarır.
// Paket formatını ayrıştırması gerekir ([FILE] markerlarını arayarak).
c_ampersand_result c_ampersand_package_extract_file(c_ampersand_file_handle package_handle, const char *file_name, const char *destination_path) {
    // Parametre doğrulama
    if (package_handle == C_AMPERSAND_INVALID_FILE_HANDLE || file_name == NULL || destination_path == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_println("PACKAGE NOT: Paketten dosya çıkarılıyor: %s -> %s", file_name, destination_path);

    // --- Implementasyon Taslağı ---
    // 1. package_handle kullanarak dosyanın başından itibaren oku (sahne_resource_seek(0, SEEK_SET) gerekebilir).
    // 2. c_ampersand_io_read_line kullanarak satır satır oku.
    // 3. "[FILE]\n" markerını bul.
    // 4. Bulduktan sonra, bir sonraki satırı (veya satırları) oku ve "filename=" anahtarını ara.
    // 5. Çıkarılacak "file_name" ile eşleşip eşleşmediğini kontrol et (strcmp).
    // 6. Eşleşme bulunursa, "[FILE]\n" ile "[/FILE]\n" arasındaki dosya içeriğini oku.
    // 7. destination_path yolunda yeni bir dosya oluştur (c_ampersand_io_open, C_AMPERSAND_O_WRONLY | C_AMPERSAND_O_CREAT | C_AMPERSAND_O_TRUNC bayraklarıyla).
    // 8. Okunan dosya içeriğini yeni dosyaya yaz (c_ampersand_io_write kullanarak, belki bufferlı okuma/yazma).
    // 9. Yeni dosyayı kapat (c_ampersand_io_close).
    // 10. "[/FILE]\n" markerını oku ve doğrula.
    // 11. Dosya içeriği okuma bitmeden "[/FILE]\n" bulunursa veya format bozuksa hata ver (C_AMPERSAND_ERROR_PACKAGE_FORMAT).
    // 12. Dosya bulunamazsa (tüm "[FILE]" blokları gezildiyse ve eşleşme olmadıysa) C_AMPERSAND_ERROR_FILE_NOT_FOUND hatası dön.
    // 13. I/O hatalarını (okuma, yazma, açma, kapatma) uygun şekilde raporla.
    // 14. Bellek tahsis hatalarını (bufferlar için) raporla.
    // ...

    // Örnek: Şu an sadece bir placeholder.
     return c_ampersand_error_with_message(C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, "Dosya çıkarma henüz implemente edilmedi.");

    c_ampersand_println("PACKAGE NOT: Dosya çıkarma implementasyon taslağı burada çalışacak.");

    // Placeholder başarı dönüşü
    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

// Açık bir C Ampersand paketindeki tüm dosyaların adlarının listesini yazdırır.
// Paket formatını ayrıştırması gerekir ([FILE] markerlarını ve filename anahtarını arayarak).
c_ampersand_result c_ampersand_package_list_files(c_ampersand_file_handle package_handle) {
     // Parametre doğrulama
    if (package_handle == C_AMPERSAND_INVALID_FILE_HANDLE) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_println("PACKAGE NOT: Paketteki dosyalar listeleniyor:");

    // --- Implementasyon Taslağı ---
    // 1. package_handle kullanarak dosyanın başından itibaren oku (sahne_resource_seek(0, SEEK_SET) gerekebilir).
    // 2. c_ampersand_io_read_line kullanarak satır satır oku.
    // 3. "[FILE]\n" markerını bul.
    // 4. Bulduktan sonra, bir sonraki satırı oku ve "filename=" anahtarını ara.
    // 5. Bulunan dosya adını (eşittir işaretinden sonraki kısmı) c_ampersand_println ile yazdır.
    // 6. "[/FILE]\n" markerına kadar olan dosya içeriğini atla (veya okuyup göz ardı et).
    // 7. Dosya sonuna gelene kadar 3. adıma geri dön.
    // 8. Paket formatı hatalarını (örn. "[FILE]" var ama "filename=" yok) raporla.
    // 9. I/O hatalarını raporla.
    // ...

    // Örnek: Şu an sadece bir placeholder.
     c_ampersand_println("PACKAGE NOT: Dosya listeleme implementasyon taslağı burada çalışacak.");
     c_ampersand_println("- ornek_program"); // Placeholder çıktı
     c_ampersand_println("- kaynak_kod.c&"); // Placeholder çıktı

    // Placeholder başarı dönüşü
    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

// Bir dizinden C Ampersand paketi oluşturur.
// source_directory: Paketlenecek dosyaların bulunduğu dizin.
// output_file_path: Oluşturulacak paket dosyasının yolu.
// Paket formatına göre output_file_path'e yazar.
c_ampersand_result c_ampersand_package_create(const char *source_directory, const char *output_file_path) {
    // Parametre doğrulama
    if (source_directory == NULL || output_file_path == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_println("PACKAGE NOT: Paket oluşturuluyor: %s -> %s", source_directory, output_file_path);

    // --- Implementasyon Taslağı ---
    // 1. output_file_path'te yeni bir dosya oluştur (c_ampersand_io_open, C_AMPERSAND_O_WRONLY | C_AMPERSAND_O_CREAT | C_AMPERSAND_O_TRUNC bayraklarıyla).
    // 2. Oluşturulan dosyaya meta bilgi blokunu yaz ([METADATA]\n...\n[/METADATA]\n). Meta bilgiyi oluşturmak için inputlardan (veya varsayılanlardan) faydalanılabilir.
    // 3. source_directory içeriğini listele (Sahne64 dizin okuma API'si veya C& sarmalayıcısı gereklidir).
    // 4. Listelenen her dosya için:
    //   a. "[FILE]\n" markerını yaz.
    //   b. "filename=<dosya_adı>\n" satırını yaz.
    //   c. source_directory içindeki ilgili dosyayı aç (c_ampersand_io_open, C_AMPERSAND_O_RDONLY).
    //   d. Açılan dosyanın içeriğini oku (c_ampersand_io_read) ve paket dosyasına yaz (c_ampersand_io_write). Bufferlı okuma/yazma gerekebilir.
    //   e. Kaynak dosyayı kapat (c_ampersand_io_close).
    //   f. "[/FILE]\n" markerını yaz.
    // 5. Paket dosyasını kapat (c_ampersand_io_close).
    // 6. Tüm I/O ve bellek hatalarını (bufferlar için) uygun şekilde raporla.
    // ...

    // Örnek: Şu an sadece bir placeholder.
    // return c_ampersand_error_with_message(C_AMPERSAND_ERROR_UNSUPPORTED_OPERATION, "Paket oluşturma henüz implemente edilmedi.");

    c_ampersand_println("PACKAGE NOT: Paket oluşturma implementasyon taslağı burada çalışacak.");

    // Placeholder başarı dönüşü
    return C_AMPERSAND_OK_RESULT; // Başarı (placeholder)
}

// Bir C Ampersand paket dosyasının temel format geçerliliğini kontrol eder.
// Paketi açar, meta bilgiyi okur, dosya listesini okumaya çalışır ve markerları kontrol eder.
c_ampersand_result c_ampersand_package_validate(const char *file_path) {
    // Parametre doğrulama
    if (file_path == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT, NULL, 0};
    }

    c_ampersand_println("PACKAGE NOT: Paket doğrulanıyor: %s", file_path);

    c_ampersand_file_handle package_handle = C_AMPERSAND_INVALID_FILE_HANDLE;
    // Paketi aç
    c_ampersand_result open_result = c_ampersand_package_open(file_path, &package_handle);
    if (open_result.code != C_AMPERSAND_OK) {
        // Açma hatasını olduğu gibi ilet
         c_ampersand_println("Paket açma hatası: %s", file_path); // Loglama yerine hata döndürüyoruz.
        return open_result;
    }

    c_ampersand_package_metadata metadata;
    // Meta bilgiyi oku ve doğrula (read_metadata_from_handle format kontrolünü yapar)
    c_ampersand_result metadata_result = c_ampersand_package_read_metadata(package_handle, &metadata);
    if (metadata_result.code != C_AMPERSAND_OK) {
        // Meta bilgi okuma hatası veya format hatası. Paketi kapat ve hatayı ilet.
         c_ampersand_println("Meta bilgi okuma hatası."); // Loglama yerine hata döndürüyoruz.
        c_ampersand_package_close(package_handle);
        return metadata_result;
    }

    // Başarılı okunan meta bilginin heap'teki stringlerini serbest bırak
    c_ampersand_package_free_metadata(&metadata); // Hata dönüş değerini kontrol etmiyoruz (kritik hata olabilir)


    // --- Ek Doğrulama Adımları (Placeholder) ---
    // 1. Paket formatının geri kalanını (dosya bloklarını) oku ve markerların ([FILE], [/FILE], filename=)
    //    doğru sırada ve doğru yapıda olup olmadığını kontrol et. Dosya içeriklerini atla veya kontrol et (checksum?).
    // 2. c_ampersand_package_list_files implementasyonundaki parsing mantığı burada kullanılabilir.
    // 3. sahne_resource_seek(0, SEEK_SET) ile dosyanın başına dönmek gerekebilir metadata okuduktan sonra.
    // ...

    c_ampersand_println("PACKAGE NOT: Paket ek doğrulama taslağı burada çalışacak.");


    // Paketi kapat
    c_ampersand_result close_result = c_ampersand_package_close(package_handle);
    if (close_result.code != C_AMPERSAND_OK) {
        // Kapatma hatasını ilet
        return close_result;
    }


    c_ampersand_println("PACKAGE NOT: Paket doğrulama başarılı (temel format). %s", file_path);
    return C_AMPERSAND_OK_RESULT; // Başarı
}

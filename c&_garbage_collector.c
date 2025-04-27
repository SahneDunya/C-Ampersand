#include "c&_garbage_collector.h"
// c_ampersand_result yapısı, C_AMPERSAND_* hataları, C_AMPERSAND_OK_RESULT ve c_ampersand_println için
#include "c&_stdlib.h"
// SAHNE64 C API başlığı (GC heap'ini almak ve geri vermek için)
// #include "sahne64_api.h" // Eski isim
#include "sahne.h"
#include <stddef.h>  // size_t, NULL için
#include <stdbool.h> // bool için
#include <string.h>  // memset için (güvenlik/başlatma)


// --- Dahili Sabitler ve Veri Yapıları ---

#define HEAP_SIZE (1024 * 1024) // Örnek heap boyutu (1MB)

static char *gc_heap_base = NULL;     // SAHNE64'ten alınan bloğun başlangıcı
static char *gc_heap_end = NULL;      // SAHNE64'ten alınan bloğun sonu
static char *gc_next_alloc = NULL;    // Heap içinde bir sonraki tahsis edilecek adres
static c_ampersand_gc_object_header *free_list = NULL; // Serbest bırakılan blokların listesi

// Kök nesnelerin listesi (GC taraması buradan başlar)
#define MAX_ROOTS 128 // Kaydedilebilecek maksimum kök sayısı
static void **roots[MAX_ROOTS]; // İşaretçilerin işaret ettiği pointer'ların adresleri
static int root_count = 0;      // Kaydedilmiş kök sayısı

// Çöp toplayıcı başlatıldı mı?
static bool gc_initialized = false;

// --- Yardımcı Fonksiyonlar ---

// Belirtilen boyutta (header hariç) bir bellek bloğu bulur veya heap'ten ayırır.
// Heap içinde bellek yetersizse NULL döner.
static c_ampersand_gc_object_header *allocate_block_from_heap(size_t size) {
    size_t total_size = sizeof(c_ampersand_gc_object_header) + size;

    // 1. Serbest listede uygun boyutta bir blok ara (First-fit)
    c_ampersand_gc_object_header *current = free_list;
    c_ampersand_gc_object_header *prev = NULL;

    while (current != NULL) {
        if (current->size >= size) {
            // Uygun boyutta serbest blok bulundu
            // Bloğu serbest listeden çıkar
            if (prev == NULL) {
                free_list = current->next_free;
            } else {
                prev->next_free = current->next_free;
            }

            // Eğer bulunan blok istenen boyuttan büyükse, kalan kısmı yeni bir serbest blok olarak ekle
            // Bu fragmentasyon yaratır. Bitişik serbest blokları birleştirme (coalescing) daha iyidir.
            // Şimdilik basitlik için birleştirme yapılmıyor.
            // Kalan boyut en az header boyutunda olmalı ki yeni serbest blok olabilsin.
            if (current->size > size + sizeof(c_ampersand_gc_object_header)) {
                 // Kalan kısım için yeni header
                 c_ampersand_gc_object_header *remaining = (c_ampersand_gc_object_header *)((char *)current + total_size);
                 remaining->size = current->size - total_size;
                 // marked ve next_free alanları sweep sırasında ayarlanacak (veya burada 0'la)
                 memset(remaining, 0, sizeof(c_ampersand_gc_object_header)); // Başlangıçta sıfırla
                 free_block(remaining); // Kalan kısmı serbest listeye ekle
                 // current'ın boyutunu istenen boyuta düşür
                 current->size = size;
            }


            // Bloğun header alanlarını ayarla (marked ve next_free serbest listeden çıkarılınca anlamsız)
            current->marked = false; // Yeni tahsis edildi, işaretlenmemiş olmalı
            current->next_free = NULL; // Serbest listeden çıkarıldı

            return current; // Tahsis edilecek bloğun header'ını döndür
        }
        prev = current;
        current = current->next_free;
    }

    // 2. Serbest listede uygun blok bulunamadı, heap'in kullanılmayan kısmından ayır
    if (gc_next_alloc != NULL && gc_next_alloc + total_size <= gc_heap_end) {
        c_ampersand_gc_object_header *header = (c_ampersand_gc_object_header *)gc_next_alloc;
        header->size = size;
        header->marked = false;
        header->next_free = NULL; // Serbest listesinde değil

        gc_next_alloc += total_size; // Bir sonraki tahsis adresi

        return header; // Yeni tahsis edilen bloğun header'ını döndür
    }

    return NULL; // Heap'te yeterli bellek yok
}

// Bir bellek bloğunu serbest listeye ekler.
// Not: Birleştirme (Coalescing) burada yapılmıyor.
static void free_block(c_ampersand_gc_object_header *header) {
    if (header == NULL) return; // Güvenlik
    // Zaten serbest listesinde olabilir mi? Kontrol etmek gerekebilir.

    // Bloğu serbest listesinin başına ekle
    header->next_free = free_list;
    free_list = header;

    // Header alanlarını sıfırla (güvenlik için, özellikle marked bayrağı)
    header->marked = false;
    // header->size ve header->next_free zaten ayarlandı
}

// Heap üzerindeki tüm tahsis edilmiş nesnelerin işaretini kaldırır.
// gc_heap_base'den gc_next_alloc'a kadar olan alanı gezer.
static void unmark_all() {
    if (!gc_initialized || gc_heap_base == NULL) return; // GC başlatılmadıysa işlem yapma

    char *current_ptr = gc_heap_base; // Heap'in başından başla

    while (current_ptr < gc_next_alloc) { // Henüz tahsis edilmiş alanın sonuna gelmediysen
        c_ampersand_gc_object_header *header = (c_ampersand_gc_object_header *)current_ptr;

        // Header geçerli görünüyor mu? (Basic check - size 0 olmamalı vb.)
        // Daha sağlam bir GC, bu noktada heap'in bozulup bozulmadığını kontrol edebilir.
        if (header->size == 0) {
             // Çok ciddi hata: Heap bozulmuş olabilir. Panik veya hata raporu.
             c_ampersand_println("GC Hatası: Heap bozulmuş (size 0). Panik!");
             // Burada paniklemeli veya programı sonlandırmalı.
             return; // Güvenlik için dön, ama durum kurtarılamaz.
        }

        header->marked = false; // İşareti kaldır

        // Bir sonraki bloğun adresini hesapla
        current_ptr += (sizeof(c_ampersand_gc_object_header) + header->size);
    }
}

// Köklerden başlayarak canlı nesneleri işaretler (Mark fazı).
// Bu taslakta SADECE kök işaretçilerin işaret ettiği ilk seviyedeki nesneleri işaretler.
// Gerçek implementasyon, nesnelerin içindeki pointerları takip etmelidir (layout bilgisi gerektirir).
static void mark_live_objects() {
    if (!gc_initialized || gc_heap_base == NULL) return; // GC başlatılmadıysa işlem yapma

    for (int i = 0; i < root_count; ++i) {
        void *ptr = *roots[i]; // Kök işaretçinin gösterdiği değeri al
        if (ptr != NULL) {
            // Bu pointer'ı işaretleme fonksiyonuna gönder
            // c_ampersand_gc_mark zaten heap içinde olup olmadığını kontrol ediyor
            c_ampersand_gc_mark(ptr);

            // ÖNEMLİ EKSİKLİK: Burada ptr'ın işaret ettiği nesnenin tipi belirlenmeli
            // ve o nesnenin içindeki diğer pointer'lar (eğer varsa) bulunarak
            // onlar da özyinelemeli olarak işaretlenmelidir.
            // Örneğin: if (object_is_struct_with_pointers(ptr)) { ... mark internal pointers ... }
        }
    }
    // Tam bir GC, programın stack'ini ve register'larını da tarayarak kökleri bulmaya çalışır (daha karmaşık).
}

// İşaretsiz nesneleri topla (serbest listeye ekle) - Sweep fazı.
// gc_heap_base'den gc_next_alloc'a kadar olan alanı gezer.
static void sweep() {
    if (!gc_initialized || gc_heap_base == NULL) return; // GC başlatılmadıysa işlem yapma

    // Sweep sırasında serbest listeyi yeniden oluşturacağız.
    free_list = NULL;

    char *current_ptr = gc_heap_base; // Heap'in başından başla

    while (current_ptr < gc_next_alloc) { // Henüz tahsis edilmiş alanın sonuna gelmediysen
        c_ampersand_gc_object_header *header = (c_ampersand_gc_object_header *)current_ptr;

         // Header geçerli görünüyor mu? (Basic check)
        if (header->size == 0) {
             // Çok ciddi hata: Heap bozulmuş olabilir. Panik veya hata raporu.
             c_ampersand_println("GC Hatası: Heap bozulmuş (size 0) süpürme sırasında. Panik!");
             // Burada paniklemeli veya programı sonlandırmalı.
             return; // Güvenlik için dön
        }

        if (!header->marked) {
            // Nesne işaretlenmemiş (canlı değil), serbest bırak
            free_block(header); // Serbest listesine ekle
        }
        // else { // Nesne işaretlenmiş (canlı)
        //    // Canlı nesneye dokunma
        // }

        // Bir sonraki bloğun adresini hesapla
        current_ptr += (sizeof(c_ampersand_gc_object_header) + header->size);
    }

    // SÜPÜRME EKSİKLİĞİ: Sweep fazı tamamlandıktan sonra, serbest listedeki bitişik
    // boş blokları birleştirme (coalescing) işlemi yapılmalıdır.
    // Aksi takdirde, küçük boş bloklar oluşur ve büyük tahsisler için yeterli yer olmayabilir.
}

// --- Çöp Toplama Fonksiyonlarının Implementasyonu ---

void *c_ampersand_gc_allocate(size_t size) {
    // İstenen boyut header boyutundan az olamaz
    if (size == 0) size = 1; // 0 boyutlu tahsisleri engelle veya yönet

    if (!gc_initialized) {
        // Çöp toplayıcı başlatılmamış
        c_ampersand_println("GC Hatası: Çöp toplayıcı başlatılmamış.");
        return NULL; // Hata, NULL dön
    }

    // Heap içinde blok ayırmayı dene
    c_ampersand_gc_object_header *header = allocate_block_from_heap(size);

    if (header != NULL) {
        // Başarılı olursa, kullanıcıya header'dan sonraki kısmı ver
        return (void *)((char *)header + sizeof(c_ampersand_gc_object_header));
    } else {
        // Bellek yetersiz (hem serbest liste hem de boş heap alanı bitti)
        c_ampersand_println("GC Uyarı: Bellek yetersiz, çöp toplamayı tetikliyor...");

        // Çöp toplamayı tetikle
        c_ampersand_gc_collect();

        // Çöp toplamadan sonra tekrar ayırmayı dene
        header = allocate_block_from_heap(size);

        if (header != NULL) {
            // Çöp toplamadan sonra bellek bulundu
            return (void *)((char *)header + sizeof(c_ampersand_gc_object_header));
        }

        // Çöp toplamadan sonra bile bellek yetersiz
        c_ampersand_println("GC Hatası: Çöp toplamadan sonra bile bellek yetersiz (istenen %zu byte).", size);
        return NULL; // Gerçekten bellek yetersiz
    }
}

void c_ampersand_gc_mark(void *ptr) {
    if (ptr == NULL || !gc_initialized || gc_heap_base == NULL) {
        return; // Geçersiz işaretçi veya GC başlatılmamış
    }

    // Pointer'ın GC heap alanı içinde olup olmadığını kontrol et
    // Header pointer'ını hesapla
    c_ampersand_gc_object_header *header = (c_ampersand_gc_object_header *)((char *)ptr - sizeof(c_ampersand_gc_object_header));

    // Heap sınırları içinde mi? (gc_heap_base <= header < gc_next_alloc)
    // gc_next_alloc, tahsis edilmiş son bloğun bitişinden sonrasını gösterir.
    // header, gc_heap_base'den başlamalı ve gc_next_alloc'tan önce olmalıdır.
    if ((char *)header >= gc_heap_base && (char *)header < gc_next_alloc) {

         // Header'ın işaretli olup olmadığını kontrol et
        if (!header->marked) {
            // İşaretlenmemişse işaretle
            header->marked = true;

            // ÖNEMLİ EKSİKLİK: Burası özyinelemeli taramanın başladığı yerdir.
            // İşaretlenen nesnenin tipi belirlenmeli (bu bilgi header'da veya başka yerde saklanmalı)
            // ve bu tipin içindeki tüm pointer alanları bulunarak her biri için
            // c_ampersand_gc_mark özyinelemeli olarak çağrılmalıdır.
            // Bu, dildeki veri tipleri ve bunların bellek layout'u hakkında bilgi sahibi olmayı gerektirir.
            // Örneğin:
            // c_ampersand_type obj_type = get_object_type(header); // Varsayımsal fonksiyon
            // for each pointer_field in obj_type.pointers:
            //    void* internal_ptr = *((void**)((char*)ptr + pointer_field.offset));
            //    c_ampersand_gc_mark(internal_ptr);
        }
    }
    // else { // Pointer heap içinde değilse işaretleme yapma (stack, globals, Sahne64 belleği vb.)
    //    // Bu geçerli bir durumdur.
    // }
}

void c_ampersand_gc_collect() {
    if (!gc_initialized || gc_heap_base == NULL) {
        c_ampersand_println("GC Uyarı: Çöp toplayıcı başlatılmamış, toplama yapılamaz.");
        return;
    }
    c_ampersand_println("Çöp toplama başlatılıyor...");

    // 1. Unmark (Tüm nesnelerin işaretini kaldır)
    unmark_all();

    // 2. Mark (Köklerden başlayarak canlı nesneleri işaretle)
    mark_live_objects(); // Önemli Eksiklik: Rekürsif işaretleme ve tip bilgisi gerekiyor

    // 3. Sweep (İşaretsiz nesneleri topla ve serbest listeye ekle)
    sweep(); // Önemli Eksiklik: Bitişik boş blokları birleştirme (coalescing) gerekiyor

    c_ampersand_println("Çöp toplama tamamlandı.");
}

void c_ampersand_gc_register_root(void **root_ptr) {
    if (root_ptr == NULL) {
         c_ampersand_println("GC Uyarı: NULL kök işaretçisi kaydedilemez.");
         return;
    }
    if (root_count < MAX_ROOTS) {
        // Aynı kök pointer'ın birden fazla kez kaydedilmediğini kontrol etmek iyi olur.
        // Basitlik adına bu kontrol atlanmıştır.
        roots[root_count++] = root_ptr;
    } else {
        c_ampersand_println("GC Uyarı: MAX_ROOTS limitine ulaşıldı (%d). Kök kaydedilemedi.", MAX_ROOTS);
    }
}

void c_ampersand_gc_unregister_root(void **root_ptr) {
    if (root_ptr == NULL) {
         //c_ampersand_println("GC Uyarı: NULL kök işaretçisi kaydı kaldırılamaz."); // Gerekli değil
         return;
    }
    for (int i = 0; i < root_count; ++i) {
        if (roots[i] == root_ptr) {
            // Diziden çıkar (elemanları kaydırarak)
            for (int j = i; j < root_count - 1; ++j) {
                roots[j] = roots[j + 1];
            }
            root_count--;
            return; // Bulundu ve kaldırıldı
        }
    }
    c_ampersand_println("GC Uyarı: Kaydı kaldırılmak istenen kök işaretçisi bulunamadı.");
}

c_ampersand_result c_ampersand_gc_init() {
    if (gc_initialized) {
        c_ampersand_println("GC Uyarı: Çöp toplayıcı zaten başlatılmış.");
        return C_AMPERSAND_OK_RESULT; // Zaten başlatılmışsa başarı dön
    }

    // Sahne64'ten büyük bir bellek bloğu tahsis et
    void *allocated_block = NULL;
    sahne_error_t sahne_err = sahne_mem_allocate(HEAP_SIZE, &allocated_block);

    if (sahne_err != SAHNE_SUCCESS || allocated_block == NULL) {
        // Bellek tahsis hatası
        gc_heap_base = NULL; // İşaretçileri NULL yap
        gc_heap_end = NULL;
        gc_next_alloc = NULL;
        // Varsayım: C_AMPERSAND_ERROR_OUT_OF_MEMORY c&_stdlib.h'de tanımlı
        return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY}; // OOM hatası dön
    }

    // Tahsis başarılı, global GC değişkenlerini ayarla
    gc_heap_base = (char *)allocated_block;
    gc_heap_end = gc_heap_base + HEAP_SIZE;
    gc_next_alloc = gc_heap_base; // İlk tahsis heap'in başından başlayacak

    free_list = NULL; // Başlangıçta serbest liste boş
    root_count = 0; // Başlangıçta kök yok
    // roots dizisini de sıfırlamak iyi bir uygulamadır.
    memset(roots, 0, sizeof(void**) * MAX_ROOTS);

    gc_initialized = true; // Başlatıldı bayrağını ayarla

    c_ampersand_println("Çöp toplayıcı başlatıldı (Heap boyutu: %zu byte)", HEAP_SIZE);

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

c_ampersand_result c_ampersand_gc_shutdown() {
    if (!gc_initialized || gc_heap_base == NULL) {
        c_ampersand_println("GC Uyarı: Çöp toplayıcı zaten kapalı.");
        return C_AMPERSAND_OK_RESULT; // Zaten kapalıysa başarı dön
    }

    // Sahne64'ten alınan bellek bloğunu serbest bırak
     sahne_error_t sahne_err =
    sahne_mem_release(gc_heap_base, HEAP_SIZE);
    // Serbest bırakma hatası durumunda loglama yapılabilir.

    // Global değişkenleri sıfırla
    gc_heap_base = NULL;
    gc_heap_end = NULL;
    gc_next_alloc = NULL;
    free_list = NULL;
    root_count = 0;
    memset(roots, 0, sizeof(void**) * MAX_ROOTS); // Kök dizisini temizle

    gc_initialized = false; // Başlatıldı bayrağını sıfırla

    c_ampersand_println("Çöp toplayıcı kapatıldı.");

    return C_AMPERSAND_OK_RESULT; // Başarı döner
}

// ... diğer çöp toplama ile ilgili fonksiyonlar eklenebilir

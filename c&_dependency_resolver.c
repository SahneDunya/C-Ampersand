#include "c&_dependency_resolver.h"
#include "c&_packet_manager.h" // Varsayım: c_ampersand_package_name, c_ampersand_package_version, c_ampersand_pm_is_package_installed burada tanımlı
#include "c&_repository.h"     // Varsayım: c_ampersand_repository, c_ampersand_package_metadata (name, version, description, dependencies_string, ve ilgili size alanları) ve c_ampersand_pm_repository_get_package_metadata burada tanımlı
#include "c&_stdlib.h"         // Varsayım: c_ampersand_result, C_AMPERSAND_* hataları (OK, INVALID_ARGUMENT, OUT_OF_MEMORY, DEPENDENCY_NOT_MET, REPOSITORY_ERROR), c_ampersand_println burada tanımlı
#include "sahne.h"             // SAHNE64 C API başlığı (Bellek yönetimi için)

#include <string.h>            // strcmp, strlen, memcpy için

// --- Yardımcı Fonksiyonlar ---

// Bir paketin meta bilgisinden bağımlılıkları alır.
// Döndürülen c_ampersand_dependency dizisi ve içindeki stringler SAHNE64 belleğinde ayrılır.
// Çağıranın bu belleği sahne_mem_release ile serbest bırakması gerekir.
static c_ampersand_result get_package_dependencies(c_ampersand_package_name package_name, c_ampersand_package_version version, c_ampersand_dependency **dependencies, size_t *dependency_count) {
    if (package_name == NULL || dependencies == NULL || dependency_count == NULL) {
        return (c_ampersand_result){C_AMPERSAND_ERROR_INVALID_ARGUMENT};
    }

    c_ampersand_package_metadata metadata;
    // metadata structını sıfırla (güvenlik için)
    memset(&metadata, 0, sizeof(c_ampersand_package_metadata));

    // Şu anda sadece ilk depodan meta bilgi alınıyor. Daha gelişmiş çözüm tüm depoları kontrol etmeli.
    // Varsayım: c_ampersand_pm_repository_get_package_metadata SAHNE64 belleği kullanır ve metadata'daki string alanları (name, version, description, dependencies_string) ve boyutlarını doldurur.
    c_ampersand_result result = c_ampersand_pm_repository_get_package_metadata(NULL /* depo handle/ismi */, package_name, &metadata);

    // Meta bilgi alma hatası durumunda
    if (result.code != C_AMPERSAND_OK) {
        // Hata durumunda metadata içindeki tahsis edilmiş stringleri serbest bırakmayı dene (eğer kısmen tahsis edildiyse)
        // Varsayım: metadata struct'ı ilgili size alanlarına sahip (name_allocated_size gibi)
        if (metadata.name.ptr) sahne_mem_release(metadata.name.ptr, metadata.name.size);
        if (metadata.version.ptr) sahne_mem_release(metadata.version.ptr, metadata.version.size);
        if (metadata.description.ptr) sahne_mem_release(metadata.description.ptr, metadata.description.size);
        if (metadata.dependencies_string.ptr) sahne_mem_release(metadata.dependencies_string.ptr, metadata.dependencies_string.size); // Varsayım: dependencies_string alanı var

        return result; // Depo hatasını doğrudan ilet
    }

    // --- Metadata içindeki bağımlılık stringini ayrıştırma ---
    // Bu kısım gerçek bağımlılık ayrıştırma mantığını içermelidir.
    // metadata.dependencies_string.ptr stringini parse edip c_ampersand_dependency structlarına doldurmalı.
    // Her bir dependency struct'ının name ve version_constraint alanlarını SAHNE64 belleğinde ayırmalı.
    // Sonra bu structlardan oluşan bir diziyi SAHNE64 belleğinde ayırıp döndürmeli.

    // Bu örnekte hala bağımlılıkları statik olarak tanımlıyoruz, ancak Sahne64 bellek kullanarak:
    // Varsayım: "ornek_paket_1" bağımlılığı "baska_paket:>=2.0"
    if (strcmp(package_name, "ornek_paket_1") == 0) {
        *dependency_count = 1;
        size_t array_allocation_size = sizeof(c_ampersand_dependency) * (*dependency_count);
        c_ampersand_dependency *deps_array = NULL;

        // Bağımlılık dizisi için bellek tahsis et (Sahne64 API)
        sahne_error_t sahne_err = sahne_mem_allocate(array_allocation_size, (void**)&deps_array);
        if (sahne_err != SAHNE_SUCCESS) {
             // Array tahsis hatası, metadata stringlerini serbest bırak ve hata dön
            if (metadata.name.ptr) sahne_mem_release(metadata.name.ptr, metadata.name.size);
            if (metadata.version.ptr) sahne_mem_release(metadata.version.ptr, metadata.version.size);
            if (metadata.description.ptr) sahne_mem_release(metadata.description.ptr, metadata.description.size);
            if (metadata.dependencies_string.ptr) sahne_mem_release(metadata.dependencies_string.ptr, metadata.dependencies_string.size);
            *dependencies = NULL;
            *dependency_count = 0;
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY}; // veya map_sahne_error_to_camper(sahne_err)
        }

        // Bağımlılık struct'ını sıfırla
        memset(&deps_array[0], 0, sizeof(c_ampersand_dependency));

        // Bağımlılık adı ve versiyon kısıtlaması stringlerini kopyala (Sahne64 API)
        const char *dep_name_str = "baska_paket";
        size_t name_len = strlen(dep_name_str);
        size_t name_allocation_size = name_len + 1;
        char *name_ptr = NULL;
        sahne_err = sahne_mem_allocate(name_allocation_size, (void**)&name_ptr);
        if (sahne_err != SAHNE_SUCCESS) {
            // String tahsis hatası, daha önce tahsis edilen array'i ve metadata stringlerini serbest bırak
            sahne_mem_release(deps_array, array_allocation_size);
            if (metadata.name.ptr) sahne_mem_release(metadata.name.ptr, metadata.name.size);
            if (metadata.version.ptr) sahne_mem_release(metadata.version.ptr, metadata.version.size);
            if (metadata.description.ptr) sahne_mem_release(metadata.description.ptr, metadata.description.size);
            if (metadata.dependencies_string.ptr) sahne_mem_release(metadata.dependencies_string.ptr, metadata.dependencies_string.size);
            *dependencies = NULL;
            *dependency_count = 0;
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        memcpy(name_ptr, dep_name_str, name_allocation_size);
        deps_array[0].name.ptr = name_ptr;
        deps_array[0].name.size = name_allocation_size;

        const char *version_constraint_str = ">=2.0";
        size_t vc_len = strlen(version_constraint_str);
        size_t vc_allocation_size = vc_len + 1;
        char *vc_ptr = NULL;
        sahne_err = sahne_mem_allocate(vc_allocation_size, (void**)&vc_ptr);
        if (sahne_err != SAHNE_SUCCESS) {
             // VC string tahsis hatası, daha önce tahsis edilenleri serbest bırak
            if (deps_array[0].name.ptr) sahne_mem_release(deps_array[0].name.ptr, deps_array[0].name.size); // Daha önce tahsis edilen stringi serbest bırak
            sahne_mem_release(deps_array, array_allocation_size); // Array'i serbest bırak
            if (metadata.name.ptr) sahne_mem_release(metadata.name.ptr, metadata.name.size);
            if (metadata.version.ptr) sahne_mem_release(metadata.version.ptr, metadata.version.size);
            if (metadata.description.ptr) sahne_mem_release(metadata.description.ptr, metadata.description.size);
            if (metadata.dependencies_string.ptr) sahne_mem_release(metadata.dependencies_string.ptr, metadata.dependencies_string.size);
            *dependencies = NULL;
            *dependency_count = 0;
            return (c_ampersand_result){C_AMPERSAND_ERROR_OUT_OF_MEMORY};
        }
        memcpy(vc_ptr, version_constraint_str, vc_allocation_size);
        deps_array[0].version_constraint.ptr = vc_ptr;
        deps_array[0].version_constraint.size = vc_allocation_size;

        // Tüm tahsisler başarılı, pointer'ı ayarla
        *dependencies = deps_array;

    } else {
        // Başka paketlerin bağımlılığı yok (örnekte)
        *dependency_count = 0;
        *dependencies = NULL;
    }

    // Not: metadata üzerindeki name, version ve description alanları
    // repository modülü tarafından SAHNE64 belleğinde ayrıldığı varsayılarak serbest bırakılıyor.
    if (metadata.name.ptr) sahne_mem_release(metadata.name.ptr, metadata.name.size);
    if (metadata.version.ptr) sahne_mem_release(metadata.version.ptr, metadata.version.size);
    if (metadata.description.ptr) sahne_mem_release(metadata.description.ptr, metadata.description.size);
    if (metadata.dependencies_string.ptr) sahne_mem_release(metadata.dependencies_string.ptr, metadata.dependencies_string.size); // dependencies_string alanını da serbest bırak

    return C_AMPERSAND_OK_RESULT; // Bağımlılıklar başarıyla alındı (veya hiç bağımlılık yok)
}

// Bir bağımlılığın kurulu olup olmadığını ve versiyon kısıtlamasını karşılayıp karşılamadığını kontrol eder.
// Varsayım: c_ampersand_pm_is_package_installed Sahne64 belleği kullanmaz ve sadece boolean döner.
static bool is_dependency_satisfied(const c_ampersand_dependency *dependency) {
    if (dependency == NULL || dependency->name.ptr == NULL) {
        return false; // Geçersiz bağımlılık tanımı
    }

    // Burada kurulu paketlerin listesi alınmalı ve versiyon kısıtlaması kontrol edilmelidir.
    // c_ampersand_pm_is_package_installed(dependency->name.ptr); // Sadece isminin kurulu olup olmadığını kontrol eder
    // c_ampersand_pm_get_installed_version(dependency->name.ptr); // Kurulu versiyonu alır
    // version_satisfies_constraint(installed_version, dependency->version_constraint.ptr); // Versiyonu kısıtlamayla karşılaştırır

    // Bu örnekte sadece paketin isminin kurulu olup olmadığı kontrol ediliyor (versiyon kısıtlaması göz ardı ediliyor).
    // Varsayım: c_ampersand_pm_is_package_installed paketin ismini const char* olarak alır.
    return c_ampersand_pm_is_package_installed(dependency->name.ptr);
}

// --- Bağımlılık Çözme Fonksiyonlarının Implementasyonu ---

c_ampersand_result c_ampersand_dependency_check(c_ampersand_package_name package_name, c_ampersand_package_version version) {
    c_ampersand_dependency *dependencies = NULL; // get_package_dependencies tarafından doldurulacak array pointer'ı
    size_t dependency_count = 0;                 // get_package_dependencies tarafından doldurulacak bağımlılık sayısı

    // Paketin bağımlılık listesini al
    c_ampersand_result result = get_package_dependencies(package_name, version, &dependencies, &dependency_count);

    // Bağımlılık listesi alma hatası durumunda
    if (result.code != C_AMPERSAND_OK) {
        // get_package_dependencies hata durumunda belleği kendisi serbest bırakır,
        // sadece hata kodunu döndür.
        return result;
    }

    bool all_satisfied = true;
    for (size_t i = 0; i < dependency_count; ++i) {
        // Her bağımlılığın karşılanıp karşılanmadığını kontrol et
        if (!is_dependency_satisfied(&dependencies[i])) {
            // Karşılanmayan bağımlılığı yazdır
            c_ampersand_println("Bağımlılık karşılanmadı: %s %s",
                                (dependencies[i].name.ptr != NULL ? dependencies[i].name.ptr : "NULL"), // NULL pointer kontrolü
                                (dependencies[i].version_constraint.ptr != NULL ? dependencies[i].version_constraint.ptr : "NULL")); // NULL pointer kontrolü
            all_satisfied = false;
        }
        // Bağımlılık struct'ı içindeki string belleklerini serbest bırak
        if (dependencies[i].name.ptr) sahne_mem_release(dependencies[i].name.ptr, dependencies[i].name.size);
        if (dependencies[i].version_constraint.ptr) sahne_mem_release(dependencies[i].version_constraint.ptr, dependencies[i].version_constraint.size);
    }
    // Bağımlılıklar dizisi için ayrılan belleği serbest bırak
    if (dependencies != NULL) {
         // sahne_error_t err =
         sahne_mem_release(dependencies, dependency_count * sizeof(c_ampersand_dependency));
         // Log hata?
    }


    if (!all_satisfied) {
        // Tüm bağımlılıklar karşılanmadıysa hata dön
        // Varsayım: C_AMPERSAND_ERROR_DEPENDENCY_NOT_MET c&_stdlib.h'de tanımlı
        return (c_ampersand_result){C_AMPERSAND_ERROR_DEPENDENCY_NOT_MET};
    }

    // Tüm bağımlılıklar karşılandı
    return C_AMPERSAND_OK_RESULT;
}

c_ampersand_result c_ampersand_dependency_resolve(c_ampersand_package_name package_name, c_ampersand_package_version version) {
    c_ampersand_dependency *dependencies = NULL;
    size_t dependency_count = 0;

    // Paketin bağımlılık listesini al
    c_ampersand_result result = get_package_dependencies(package_name, version, &dependencies, &dependency_count);

    // Bağımlılık listesi alma hatası durumunda
    if (result.code != C_AMPERSAND_OK) {
        return result;
    }

    c_ampersand_println("Bağımlılıklar çözülüyor: %s", package_name);

    // Kurulması gereken bağımlılıkları belirle ve potansiyel olarak kur
    for (size_t i = 0; i < dependency_count; ++i) {
        // Bağımlılık karşılanmadıysa (kurulu değilse veya versiyon uymuyorsa)
        if (!is_dependency_satisfied(&dependencies[i])) {
            // Kullanıcıya bilgi ver
            c_ampersand_println("Kurulması gereken bağımlılık: %s %s",
                                (dependencies[i].name.ptr != NULL ? dependencies[i].name.ptr : "NULL"),
                                (dependencies[i].version_constraint.ptr != NULL ? dependencies[i].version_constraint.ptr : "NULL"));

            // Burada bağımlılığı kurmak için paket yöneticisine çağrı yapılabilir.
            // Bu çağrı özyinelemeli olarak bağımlılıkların bağımlılıklarını da çözecektir.
            // c_ampersand_result install_res = c_ampersand_pm_install_package(dependencies[i].name.ptr, dependencies[i].version_constraint.ptr);
            // if (install_res.code != C_AMPERSAND_OK) {
            //    // Kurulum hatası durumunda temizlik yap ve hatayı ilet
            //    if (dependencies[i].name.ptr) sahne_mem_release(dependencies[i].name.ptr, dependencies[i].name.size);
            //    if (dependencies[i].version_constraint.ptr) sahne_mem_release(dependencies[i].version_constraint.ptr, dependencies[i].version_constraint.size);
            //    // Geri kalan stringleri ve array'i serbest bırak
            //    for (size_t j = i + 1; j < dependency_count; ++j) {
            //        if (dependencies[j].name.ptr) sahne_mem_release(dependencies[j].name.ptr, dependencies[j].name.size);
            //        if (dependencies[j].version_constraint.ptr) sahne_mem_release(dependencies[j].version_constraint.ptr, dependencies[j].version_constraint.size);
            //    }
            //    if (dependencies != NULL) sahne_mem_release(dependencies, dependency_count * sizeof(c_ampersand_dependency));
            //    return install_res; // Kurulum hatasını ilet
            // }
        }
         // Bağımlılık struct'ı içindeki string belleklerini serbest bırak
        if (dependencies[i].name.ptr) sahne_mem_release(dependencies[i].name.ptr, dependencies[i].name.size);
        if (dependencies[i].version_constraint.ptr) sahne_mem_release(dependencies[i].version_constraint.ptr, dependencies[i].version_constraint.size);
    }
    // Bağımlılıklar dizisi için ayrılan belleği serbest bırak
    if (dependencies != NULL) {
         // sahne_error_t err =
         sahne_mem_release(dependencies, dependency_count * sizeof(c_ampersand_dependency));
         // Log hata?
    }


    c_ampersand_println("Bağımlılık çözme tamamlandı: %s", package_name);
    return C_AMPERSAND_OK_RESULT; // Çözme işlemi (veya gerekli paketleri belirleme) tamamlandı
}

c_ampersand_result c_ampersand_dependency_check_conflicts(c_ampersand_package_name package_name1, c_ampersand_package_version version1,
                                                         c_ampersand_package_name package_name2, c_ampersand_package_version version2) {
    // Bu fonksiyon, iki paket ve onların bağımlılık ağaçları arasında olası bağımlılık çakışmalarını kontrol etmek için implemente edilebilir.
    // Örneğin, aynı paketin farklı uyumsuz versiyonlarına bağımlılıkları olabilirler.
    // Bu implementasyon için iki paketin tüm bağımlılıklarını rekürsif olarak alıp karşılaştırmak gerekir.

    c_ampersand_println("Bağımlılık çakışmaları kontrol ediliyor: %s (%s) ve %s (%s)",
                        (package_name1 != NULL ? package_name1 : "NULL"), (version1 != NULL ? version1 : "NULL"),
                        (package_name2 != NULL ? package_name2 : "NULL"), (version2 != NULL ? version2 : "NULL"));

    // Bu örnekte basit bir çakışma kontrolü yapılmıyor.
    // Gerçek implementasyon için recursive bağımlılık toplama ve versiyon karşılaştırma mantığı gerekir.

    c_ampersand_println("Basit çakışma kontrolü tamamlandı (çakışma bulunamadı).");
    // Gerçek bir çakışma bulunursa C_AMPERSAND_ERROR_CONFLICT gibi bir hata kodu dönülmelidir.
    return C_AMPERSAND_OK_RESULT;
}

// ... diğer bağımlılık çözme ile ilgili fonksiyonlar eklenebilir

// C& Standart Kütüphane ana başlığı (diğer modülleri içerir)
#include "c&_stdlib.h"
// Kullanılan diğer C& modülleri (c&_stdlib.h zaten include ediyor olabilir, ama açıkça belirtmek iyi).
#include "c&_packet_manager.h"
#include "c&_repository.h"
 #include "c&_dependency_resolver.h" // Eğer bu modül kullanılmıyorsa kaldırılabilir.
// c_ampersand_println zaten c&_stdlib.h'den geliyor.


// --- Yardımcı Fonksiyonlar ---

// Kullanım mesajını yazdırır.
static void print_usage() { // static eklendi
    c_ampersand_println("Kullanım: c&pm <komut> [argümanlar]");
    c_ampersand_println("");
    c_ampersand_println("Komutlar:");
    c_ampersand_println("  install <paket_adı> [versiyon]  - Belirtilen paketi kurar.");
    c_ampersand_println("  uninstall <paket_adı>           - Belirtilen paketi kaldırır.");
    c_ampersand_println("  list                              - Kurulu paketleri listeler.");
    c_ampersand_println("  update                            - Paket depolarını günceller.");
    c_ampersand_println("  repo add <url>                    - Yeni bir paket deposu ekler.");
    c_ampersand_println("  repo remove <url>                 - Bir paket deposunu kaldırır.");
    c_ampersand_println("  repo update                       - Yapılandırılmış tüm depoları günceller.");
    c_ampersand_println("  info <paket_adı>                  - Bir paket hakkında bilgi gösterir (placeholder)."); // Yorum güncellendi
    c_ampersand_println("  help                              - Bu yardım mesajını gösterir.");
    c_ampersand_println("");
}

// --- Ana Fonksiyon ---

// Sahne64 runtime'ının bu main imzasını sağladığı varsayılır.
int main(int argc, char *argv[]) {
    // C& runtime ve modüllerini başlat
    c_ampersand_result init_res = c_ampersand_init(); // c&_stdlib.c içinde tanımlı varsayılır
    if (init_res.code != C_AMPERSAND_OK) {
        // Başlatma hatası, mesajı yazdır (init fonksiyonu zaten loglamış olabilir)
        c_ampersand_println("C& Runtime başlatılırken hata: %s", init_res.message ? init_res.message : "Bilinmeyen Hata");
        c_ampersand_result_free_message(&init_res); // Hata mesajını serbest bırak
        // Başlatma hatasında kapanış fonksiyonları çağrılmayabilir veya init hata temizliği yapmalıdır.
        return 1; // Hata çıkış kodu
    }


    int exit_code = 0; // Başlangıçta başarı çıkış kodu varsayalım

    if (argc < 2) {
        print_usage();
        exit_code = 1;
    } else {

        const char *command = argv[1];

        // strcmp yerine c_ampersand_string_compare kullan
        if (c_ampersand_string_compare(command, "install") == 0) {
            if (argc < 3) {
                c_ampersand_println("Hata: 'install' komutu bir paket adı gerektirir.");
                exit_code = 1;
            } else {
                const char *package_name = argv[2];
                const char *version = (argc > 3) ? argv[3] : NULL;
                c_ampersand_result result = c_ampersand_pm_install_package(package_name, version);
                if (result.code != C_AMPERSAND_OK) {
                    // Hata mesajını kullan (varsa)
                    c_ampersand_println("Paket kurulum hatası: %s (Hata Kodu: %d)", result.message ? result.message : "Bilinmeyen Hata", result.code);
                    c_ampersand_result_free_message(&result); // Mesajı serbest bırak
                    exit_code = 1;
                }
            }
        // strcmp yerine c_ampersand_string_compare kullan
        } else if (c_ampersand_string_compare(command, "uninstall") == 0) {
            if (argc < 3) {
                c_ampersand_println("Hata: 'uninstall' komutu bir paket adı gerektirir.");
                exit_code = 1;
            } else {
                const char *package_name = argv[2];
                c_ampersand_result result = c_ampersand_pm_uninstall_package(package_name);
                if (result.code != C_AMPERSAND_OK) {
                    // Hata mesajını kullan (varsa)
                    c_ampersand_println("Paket kaldırma hatası: %s (Hata Kodu: %d)", result.message ? result.message : "Bilinmeyen Hata", result.code);
                    c_ampersand_result_free_message(&result); // Mesajı serbest bırak
                    exit_code = 1;
                }
            }
        // strcmp yerine c_ampersand_string_compare kullan
        } else if (c_ampersand_string_compare(command, "list") == 0) {
            c_ampersand_result result = c_ampersand_pm_list_installed_packages();
            if (result.code != C_AMPERSAND_OK) {
                // Hata mesajını kullan (varsa)
                c_ampersand_println("Kurulu paketleri listeleme hatası: %s (Hata Kodu: %d)", result.message ? result.message : "Bilinmeyen Hata", result.code);
                c_ampersand_result_free_message(&result); // Mesajı serbest bırak
                exit_code = 1;
            }
        // strcmp yerine c_ampersand_string_compare kullan
        } else if (c_ampersand_string_compare(command, "update") == 0) {
            c_ampersand_result result = c_ampersand_pm_repository_update_all();
            if (result.code != C_AMPERSAND_OK) {
                // Hata mesajını kullan (varsa)
                c_ampersand_println("Depo güncelleme hatası: %s (Hata Kodu: %d)", result.message ? result.message : "Bilinmeyen Hata", result.code);
                c_ampersand_result_free_message(&result); // Mesajı serbest bırak
                exit_code = 1;
            }
        // strcmp yerine c_ampersand_string_compare kullan
        } else if (c_ampersand_string_compare(command, "repo") == 0) {
            if (argc < 3) {
                c_ampersand_println("Hata: 'repo' komutu bir alt komut gerektirir ('add', 'remove', 'update').");
                exit_code = 1;
            } else {
                const char *repo_command = argv[2];
                // strcmp yerine c_ampersand_string_compare kullan
                if (c_ampersand_string_compare(repo_command, "add") == 0) {
                    if (argc < 4) {
                        c_ampersand_println("Hata: 'repo add' komutu bir depo URL'i gerektirir.");
                        exit_code = 1;
                    } else {
                        const char *repo_url = argv[3];
                        c_ampersand_result result = c_ampersand_pm_repository_add(repo_url);
                        if (result.code != C_AMPERSAND_OK) {
                            // Hata mesajını kullan (varsa)
                            c_ampersand_println("Depo ekleme hatası: %s (Hata Kodu: %d)", result.message ? result.message : "Bilinmeyen Hata", result.code);
                            c_ampersand_result_free_message(&result); // Mesajı serbest bırak
                            exit_code = 1;
                        }
                    }
                // strcmp yerine c_ampersand_string_compare kullan
                } else if (c_ampersand_string_compare(repo_command, "remove") == 0) {
                    if (argc < 4) {
                        c_ampersand_println("Hata: 'repo remove' komutu bir depo URL'i gerektirir.");
                        exit_code = 1;
                    } else {
                        const char *repo_url = argv[3];
                        c_ampersand_result result = c_ampersand_pm_repository_remove(repo_url);
                        if (result.code != C_AMPERSAND_OK) {
                            // Hata mesajını kullan (varsa)
                            c_ampersand_println("Depo kaldırma hatası: %s (Hata Kodu: %d)", result.message ? result.message : "Bilinmeyen Hata", result.code);
                            c_ampersand_result_free_message(&result); // Mesajı serbest bırak
                            exit_code = 1;
                        }
                    }
                // strcmp yerine c_ampersand_string_compare kullan
                } else if (c_ampersand_string_compare(repo_command, "update") == 0) {
                    c_ampersand_result result = c_ampersand_pm_repository_update_all();
                    if (result.code != C_AMPERSAND_OK) {
                        // Hata mesajını kullan (varsa)
                        c_ampersand_println("Depoları güncelleme hatası: %s (Hata Kodu: %d)", result.message ? result.message : "Bilinmeyen Hata", result.code);
                        c_ampersand_result_free_message(&result); // Mesajı serbest bırak
                        exit_code = 1;
                    }
                } else {
                    // strcmp yerine c_ampersand_string_compare kullan
                    c_ampersand_println("Hata: Geçersiz 'repo' alt komutu: %s", repo_command);
                    exit_code = 1;
                }
            }
        // strcmp yerine c_ampersand_string_compare kullan
        } else if (c_ampersand_string_compare(command, "info") == 0) {
            // Bilgi komutu henüz implemente edilmedi placeholder.
            if (argc < 3) {
                 c_ampersand_println("Hata: 'info' komutu bir paket adı gerektirir (placeholder).");
                 exit_code = 1;
            } else {
                 const char *package_name = argv[2];
                 c_ampersand_println("Paket bilgisi gösteriliyor (placeholder): %s", package_name);
                 c_ampersand_println("Bilgi komutu henüz tam implemente edilmedi.");
                 // Gerçek implementasyon, depolardan paket meta bilgisini alıp yazdıracaktır.
                  c_ampersand_result result = c_ampersand_pm_repository_get_package_metadata( /* ... */ );
                 // ...
            }

        // strcmp yerine c_ampersand_string_compare kullan
        } else if (c_ampersand_string_compare(command, "help") == 0) {
            print_usage();
        } else {
            // Geçersiz komut
            c_ampersand_println("Hata: Geçersiz komut: %s", command);
            print_usage();
            exit_code = 1;
        }
    }

    // C& runtime ve modüllerini kapat
    c_ampersand_result shutdown_res = c_ampersand_shutdown(); // c&_stdlib.c içinde tanımlı varsayılır
    if (shutdown_res.code != C_AMPERSAND_OK) {
        // Kapanış hatası, logla. Çıkış kodunu etkileyebilir veya etkilemeyebilir.
        // Genellikle kapanış hataları loglanır ama program yine de sıfırla çıkabilir.
         c_ampersand_println("C& Runtime kapatılırken hata: %s", shutdown_res.message ? shutdown_res.message : "Bilinmeyen Hata");
         c_ampersand_result_free_message(&shutdown_res); // Hata mesajını serbest bırak
          exit_code = 1; // Kapanış hatasında da 1 ile çıkmak istenirse
    }


    return exit_code; // Başarı (0) veya hata (1) çıkış kodu
}

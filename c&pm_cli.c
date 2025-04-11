#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c&_packet_manager.h"
#include "c&_repository.h"
#include "c&_dependency_resolver.h"
#include "c&_stdlib.h" // c_ampersand_println için

// --- Yardımcı Fonksiyonlar ---

void print_usage() {
    c_ampersand_println("Kullanım: c&pm <komut> [argümanlar]");
    c_ampersand_println("");
    c_ampersand_println("Komutlar:");
    c_ampersand_println("  install <paket_adı> [versiyon]  - Belirtilen paketi kurar.");
    c_ampersand_println("  uninstall <paket_adı>           - Belirtilen paketi kaldırır.");
    c_ampersand_println("  list                              - Kurulu paketleri listeler.");
    c_ampersand_println("  update                            - Paket depolarını günceller.");
    c_ampersand_println("  repo add <url>                    - Yeni bir paket deposu ekler.");
    c_ampersand_println("  repo remove <url>                 - Bir paket deposunu kaldırır.");
    c_ampersand_println("  repo update                       - Yapılandırılmış tüm depoları günceller.");
    c_ampersand_println("  info <paket_adı>                  - Bir paket hakkında bilgi gösterir (henüz implemente edilmedi).");
    c_ampersand_println("  help                              - Bu yardım mesajını gösterir.");
    c_ampersand_println("");
}

// --- Ana Fonksiyon ---

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "install") == 0) {
        if (argc < 3) {
            c_ampersand_println("Hata: 'install' komutu bir paket adı gerektirir.");
            return 1;
        }
        const char *package_name = argv[2];
        const char *version = (argc > 3) ? argv[3] : NULL;
        c_ampersand_result result = c_ampersand_pm_install_package(package_name, version);
        if (result.code != C_AMPERSAND_OK) {
            c_ampersand_println("Paket kurulum hatası: %s (%d)", c_ampersand_result_to_string(result), result.code);
            return 1;
        }
    } else if (strcmp(command, "uninstall") == 0) {
        if (argc < 3) {
            c_ampersand_println("Hata: 'uninstall' komutu bir paket adı gerektirir.");
            return 1;
        }
        const char *package_name = argv[2];
        c_ampersand_result result = c_ampersand_pm_uninstall_package(package_name);
        if (result.code != C_AMPERSAND_OK) {
            c_ampersand_println("Paket kaldırma hatası: %s (%d)", c_ampersand_result_to_string(result), result.code);
            return 1;
        }
    } else if (strcmp(command, "list") == 0) {
        c_ampersand_result result = c_ampersand_pm_list_installed_packages();
        if (result.code != C_AMPERSAND_OK) {
            c_ampersand_println("Kurulu paketleri listeleme hatası: %s (%d)", c_ampersand_result_to_string(result), result.code);
            return 1;
        }
    } else if (strcmp(command, "update") == 0) {
        c_ampersand_result result = c_ampersand_pm_repository_update_all();
        if (result.code != C_AMPERSAND_OK) {
            c_ampersand_println("Depo güncelleme hatası: %s (%d)", c_ampersand_result_to_string(result), result.code);
            return 1;
        }
    } else if (strcmp(command, "repo") == 0) {
        if (argc < 3) {
            c_ampersand_println("Hata: 'repo' komutu bir alt komut gerektirir ('add', 'remove', 'update').");
            return 1;
        }
        const char *repo_command = argv[2];
        if (strcmp(repo_command, "add") == 0) {
            if (argc < 4) {
                c_ampersand_println("Hata: 'repo add' komutu bir depo URL'i gerektirir.");
                return 1;
            }
            const char *repo_url = argv[3];
            c_ampersand_result result = c_ampersand_pm_repository_add(repo_url);
            if (result.code != C_AMPERSAND_OK) {
                c_ampersand_println("Depo ekleme hatası: %s (%d)", c_ampersand_result_to_string(result), result.code);
                return 1;
            }
        } else if (strcmp(repo_command, "remove") == 0) {
            if (argc < 4) {
                c_ampersand_println("Hata: 'repo remove' komutu bir depo URL'i gerektirir.");
                return 1;
            }
            const char *repo_url = argv[3];
            c_ampersand_result result = c_ampersand_pm_repository_remove(repo_url);
            if (result.code != C_AMPERSAND_OK) {
                c_ampersand_println("Depo kaldırma hatası: %s (%d)", c_ampersand_result_to_string(result), result.code);
                return 1;
            }
        } else if (strcmp(repo_command, "update") == 0) {
            c_ampersand_result result = c_ampersand_pm_repository_update_all();
            if (result.code != C_AMPERSAND_OK) {
                c_ampersand_println("Depoları güncelleme hatası: %s (%d)", c_ampersand_result_to_string(result), result.code);
                return 1;
            }
        } else {
            c_ampersand_println("Hata: Geçersiz 'repo' alt komutu: %s", repo_command);
            return 1;
        }
    } else if (strcmp(command, "info") == 0) {
        c_ampersand_println("Bilgi komutu henüz implemente edilmedi.");
    } else if (strcmp(command, "help") == 0) {
        print_usage();
    } else {
        c_ampersand_println("Hata: Geçersiz komut: %s", command);
        print_usage();
        return 1;
    }

    return 0;
}
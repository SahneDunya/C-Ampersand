# Derleyici
CC = clang

# Derleyici bayrakları
CFLAGS = -Wall -Wextra -O2 -std=c11

# Musl 1.2.5 kurulum dizini (doğru yolu ayarlayın)
MUSL_PATH = /opt/musl-1.2.5

# Conan kurulum dizini (eğer özel bir dizindeyse)
CONAN_PATH = /home/user/.conan2 # Örnek yol

# Conan profili (gerekirse ayarlayın)
CONAN_PROFILE = default

# Çıktı dosyasının adı
EXECUTABLE = ampersand_vm

# Kaynak dosyalar (tüm .c dosyalarını bulur)
SOURCES = $(wildcard *.c)

# Nesne dosyaları (kaynak dosyalarından türetilir)
OBJECTS = $(SOURCES:.c=.o)

# Başlangıç hedefi
all: conan_dependencies $(EXECUTABLE)

# Nesne dosyalarını oluşturma kuralı
%.o: %.c
	$(CC) $(CFLAGS) -I$(MUSL_PATH)/include -c $< -o $@

# Çalıştırılabilir dosyayı oluşturma kuralı
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ -L$(MUSL_PATH)/lib -Wl,-rpath=$(MUSL_PATH)/lib -static -lc -lm

# Conan bağımlılıklarını yükleme kuralı
conan_dependencies:
	@echo "Conan bağımlılıkları yükleniyor..."
	conan profile show $(CONAN_PROFILE)
	$(CONAN_PATH)/bin/conan install . --profile=$(CONAN_PROFILE) --build=missing

# Temizleme kuralı
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Çalıştırma kuralı (isteğe bağlı)
run: all
	./$(EXECUTABLE)

.PHONY: all clean run conan_dependencies
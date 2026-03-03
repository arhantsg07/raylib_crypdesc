# CryptoDesk – Makefile (MSYS2 MinGW-w64 + Raylib + libcurl)

CC      = gcc
CFLAGS  = -Wall -O2 -Wno-format \
          -IC:/raylib/include \
          -Iinclude \
          -Ivendor

LDFLAGS = -LC:/raylib/lib \
          -lraylib -lcurl \
          -lopengl32 -lgdi32 -lwinmm -lws2_32

# ── Sources by component ────────────────────────────────
CORE_SRC    = src/core/main.c src/core/api.c
SCREEN_SRC  = src/screens/home.c src/screens/prices.c \
              src/screens/trends.c src/screens/news.c
UI_SRC      = src/ui/ui.c
VENDOR_SRC  = vendor/cJSON.c

SRCS   = $(CORE_SRC) $(SCREEN_SRC) $(UI_SRC) $(VENDOR_SRC)
TARGET = build/crypdesc.exe

# ── Build ────────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(SRCS)
	@if not exist build mkdir build
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	@if exist build rmdir /s /q build

.PHONY: all clean

# CryptoDesk 🪙

A modern cryptocurrency dashboard built with **C** and **Raylib**.

![Platform](https://img.shields.io/badge/Platform-Windows-blue)
![Language](https://img.shields.io/badge/Language-C-green)

## Features

- **Live Prices** — Top 20 cryptocurrencies with price, 24h/7d % change, market cap, and mini bar charts
- **Trends** — Interactive candlestick (OHLCV) chart with 7D / 30D / 90D time ranges
- **Latest News** — Real-time crypto news feed from CryptoCompare
- **Auto-refresh** — Prices update every 30s, news every 60s
- **Modern dark UI** — Smooth animations, gradient backgrounds, hover effects

## APIs Used

| API | Purpose | Auth |
|-----|---------|------|
| [CoinPaprika](https://api.coinpaprika.com/) | Prices & OHLCV data | None (free) |
| [CryptoCompare](https://min-api.cryptocompare.com/) | News feed | None (free) |

## Prerequisites

- **MSYS2 MinGW-w64** toolchain (`gcc`, `make`)
- **Raylib** installed at `C:\raylib\`
- **libcurl** via MSYS2: `pacman -S mingw-w64-x86_64-curl`

## Build & Run

```bash
cd crypdesc
mingw32-make
build\crypdesc.exe
```

## Controls

| Key | Action |
|-----|--------|
| **Click** | Navigate menus, select coins |
| **Scroll** | Scroll tables and news feed |
| **ESC** | Go back to home screen |
| **Window X** | Close application |

## Project Structure

```
crypdesc/
├── include/                 Headers (public interfaces)
│   ├── api.h                  API data types & fetch declarations
│   ├── screens.h              Screen enum & lifecycle functions
│   ├── theme.h                Color palette & layout constants
│   └── ui.h                   UI widget declarations
│
├── src/
│   ├── core/                Application core
│   │   ├── main.c             Entry point & screen state machine
│   │   └── api.c              HTTP (libcurl) + JSON parsing
│   │
│   ├── screens/             Individual screen implementations
│   │   ├── home.c             Animated main menu
│   │   ├── prices.c           Price table with mini bar charts
│   │   ├── trends.c           Candlestick chart + coin selector
│   │   └── news.c             Scrollable news card feed
│   │
│   └── ui/                  Reusable drawing widgets
│       └── ui.c               Cards, buttons, charts, spinner
│
├── vendor/                  Third-party vendored code
│   ├── cJSON.c
│   └── cJSON.h
│
├── build/                   Compiled output (generated)
├── Makefile
└── README.md
```

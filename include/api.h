#ifndef API_H
#define API_H

#include <stdbool.h>

#define MAX_PRICES 20
#define MAX_OHLCV 90
#define MAX_NEWS 20

// ── Data types ──────────────────────────────────────────
typedef struct {
  char id[64];
  char name[64];
  char symbol[16];
  int rank;
  double price;
  double volume_24h;
  double market_cap;
  double percent_change_24h;
  double percent_change_7d;
  double percent_change_30d;
} CryptoPrice;

typedef struct {
  char date[32];
  double open;
  double high;
  double low;
  double close;
  double volume;
} OHLCVData;

typedef struct {
  char title[256];
  char source[64];
  char url[512];
  char body[1024];
  long published_on; // unix timestamp
} NewsItem;

// ── Global data stores (written by bg threads, read by main thread) ─
extern CryptoPrice gPrices[MAX_PRICES];
extern int gPriceCount;
extern bool gPricesLoading;
extern bool gPricesLoaded;
extern bool gPricesError;

extern OHLCVData gOHLCV[MAX_OHLCV];
extern int gOHLCVCount;
extern bool gOHLCVLoading;
extern bool gOHLCVLoaded;
extern bool gOHLCVError;
extern char gSelectedCoinId[64];
extern char gSelectedCoinName[64];
extern int gOHLCVDays;

extern NewsItem gNews[MAX_NEWS];
extern int gNewsCount;
extern bool gNewsLoading;
extern bool gNewsLoaded;
extern bool gNewsError;

// ── Functions ───────────────────────────────────────────
void ApiInit(void);
void ApiCleanup(void);

void FetchPricesAsync(void);
void FetchOHLCVAsync(const char *coinId, const char *coinName, int days);
void FetchNewsAsync(void);

#endif // API_H

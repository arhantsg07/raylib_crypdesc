/* api.c – HTTP fetching (libcurl) + JSON parsing (cJSON)
 * All network calls run on background threads so the UI stays 60 fps. */

#include "api.h"
#include "cJSON.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


/* ── Global data stores ──────────────────────────────── */
CryptoPrice gPrices[MAX_PRICES];
int gPriceCount = 0;
bool gPricesLoading = false;
bool gPricesLoaded = false;
bool gPricesError = false;

OHLCVData gOHLCV[MAX_OHLCV];
int gOHLCVCount = 0;
bool gOHLCVLoading = false;
bool gOHLCVLoaded = false;
bool gOHLCVError = false;
char gSelectedCoinId[64] = "";
char gSelectedCoinName[64] = "";
int gOHLCVDays = 30;

NewsItem gNews[MAX_NEWS];
int gNewsCount = 0;
bool gNewsLoading = false;
bool gNewsLoaded = false;
bool gNewsError = false;

/* ── Curl write callback ─────────────────────────────── */
typedef struct {
  char *data;
  size_t size;
} Buffer;

static size_t WriteCB(void *ptr, size_t size, size_t nmemb, void *ud) {
  size_t total = size * nmemb;
  Buffer *buf = (Buffer *)ud;
  char *tmp = realloc(buf->data, buf->size + total + 1);
  if (!tmp)
    return 0;
  buf->data = tmp;
  memcpy(buf->data + buf->size, ptr, total);
  buf->size += total;
  buf->data[buf->size] = '\0';
  return total;
}

static char *HttpGet(const char *url) {
  CURL *curl = curl_easy_init();
  if (!curl)
    return NULL;

  Buffer buf = {.data = malloc(1), .size = 0};
  buf.data[0] = '\0';

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCB);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "CryptoDesk/1.0");

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    free(buf.data);
    return NULL;
  }
  return buf.data;
}

/* ── Prices thread ───────────────────────────────────── */
static DWORD WINAPI PricesThread(LPVOID p) {
  (void)p;
  char *json = HttpGet("https://api.coinpaprika.com/v1/tickers?limit=20");
  if (!json) {
    gPricesError = true;
    gPricesLoading = false;
    return 1;
  }

  cJSON *root = cJSON_Parse(json);
  free(json);
  if (!root) {
    gPricesError = true;
    gPricesLoading = false;
    return 1;
  }

  int n = cJSON_GetArraySize(root);
  gPriceCount = 0;
  for (int i = 0; i < n && gPriceCount < MAX_PRICES; i++) {
    cJSON *it = cJSON_GetArrayItem(root, i);
    CryptoPrice *cp = &gPrices[gPriceCount];
    memset(cp, 0, sizeof(*cp));

    cJSON *v;
    if ((v = cJSON_GetObjectItem(it, "id")))
      strncpy(cp->id, v->valuestring, 63);
    if ((v = cJSON_GetObjectItem(it, "name")))
      strncpy(cp->name, v->valuestring, 63);
    if ((v = cJSON_GetObjectItem(it, "symbol")))
      strncpy(cp->symbol, v->valuestring, 15);
    if ((v = cJSON_GetObjectItem(it, "rank")))
      cp->rank = v->valueint;

    cJSON *q = cJSON_GetObjectItem(it, "quotes");
    cJSON *usd = q ? cJSON_GetObjectItem(q, "USD") : NULL;
    if (usd) {
      if ((v = cJSON_GetObjectItem(usd, "price")))
        cp->price = v->valuedouble;
      if ((v = cJSON_GetObjectItem(usd, "volume_24h")))
        cp->volume_24h = v->valuedouble;
      if ((v = cJSON_GetObjectItem(usd, "market_cap")))
        cp->market_cap = v->valuedouble;
      if ((v = cJSON_GetObjectItem(usd, "percent_change_24h")))
        cp->percent_change_24h = v->valuedouble;
      if ((v = cJSON_GetObjectItem(usd, "percent_change_7d")))
        cp->percent_change_7d = v->valuedouble;
      if ((v = cJSON_GetObjectItem(usd, "percent_change_30d")))
        cp->percent_change_30d = v->valuedouble;
    }
    gPriceCount++;
  }
  cJSON_Delete(root);
  gPricesLoaded = true;
  gPricesLoading = false;
  return 0;
}

/* ── OHLCV thread ────────────────────────────────────── */
static DWORD WINAPI OHLCVThread(LPVOID p) {
  (void)p;
  /* Build date range */
  time_t now = time(NULL);
  time_t start = now - (time_t)gOHLCVDays * 86400;
  struct tm ts, te;
  ts = *gmtime(&start);
  te = *gmtime(&now);

  char sStr[16], eStr[16];
  strftime(sStr, sizeof(sStr), "%Y-%m-%d", &ts);
  strftime(eStr, sizeof(eStr), "%Y-%m-%d", &te);

  char url[512];
  snprintf(url, sizeof(url),
           "https://api.coinpaprika.com/v1/coins/%s/ohlcv/"
           "historical?start=%s&end=%s",
           gSelectedCoinId, sStr, eStr);

  char *json = HttpGet(url);
  if (!json) {
    gOHLCVError = true;
    gOHLCVLoading = false;
    return 1;
  }

  cJSON *root = cJSON_Parse(json);
  free(json);
  if (!root) {
    gOHLCVError = true;
    gOHLCVLoading = false;
    return 1;
  }

  int n = cJSON_GetArraySize(root);
  gOHLCVCount = 0;
  for (int i = 0; i < n && gOHLCVCount < MAX_OHLCV; i++) {
    cJSON *it = cJSON_GetArrayItem(root, i);
    OHLCVData *d = &gOHLCV[gOHLCVCount];
    memset(d, 0, sizeof(*d));

    cJSON *v;
    if ((v = cJSON_GetObjectItem(it, "time_open")))
      strncpy(d->date, v->valuestring, 31);
    if ((v = cJSON_GetObjectItem(it, "open")))
      d->open = v->valuedouble;
    if ((v = cJSON_GetObjectItem(it, "high")))
      d->high = v->valuedouble;
    if ((v = cJSON_GetObjectItem(it, "low")))
      d->low = v->valuedouble;
    if ((v = cJSON_GetObjectItem(it, "close")))
      d->close = v->valuedouble;
    if ((v = cJSON_GetObjectItem(it, "volume")))
      d->volume = v->valuedouble;
    gOHLCVCount++;
  }
  cJSON_Delete(root);
  gOHLCVLoaded = true;
  gOHLCVLoading = false;
  return 0;
}

/* ── News thread ─────────────────────────────────────── */
static DWORD WINAPI NewsThread(LPVOID p) {
  (void)p;
  char *json =
      HttpGet("https://min-api.cryptocompare.com/data/v2/news/?lang=EN");
  if (!json) {
    gNewsError = true;
    gNewsLoading = false;
    return 1;
  }

  cJSON *root = cJSON_Parse(json);
  free(json);
  if (!root) {
    gNewsError = true;
    gNewsLoading = false;
    return 1;
  }

  cJSON *data = cJSON_GetObjectItem(root, "Data");
  if (!data) {
    cJSON_Delete(root);
    gNewsError = true;
    gNewsLoading = false;
    return 1;
  }

  int n = cJSON_GetArraySize(data);
  gNewsCount = 0;
  for (int i = 0; i < n && gNewsCount < MAX_NEWS; i++) {
    cJSON *it = cJSON_GetArrayItem(data, i);
    NewsItem *ni = &gNews[gNewsCount];
    memset(ni, 0, sizeof(*ni));

    cJSON *v;
    if ((v = cJSON_GetObjectItem(it, "title")))
      strncpy(ni->title, v->valuestring, 255);
    if ((v = cJSON_GetObjectItem(it, "source")))
      strncpy(ni->source, v->valuestring, 63);
    if ((v = cJSON_GetObjectItem(it, "url")))
      strncpy(ni->url, v->valuestring, 511);
    if ((v = cJSON_GetObjectItem(it, "body")))
      strncpy(ni->body, v->valuestring, 1023);
    if ((v = cJSON_GetObjectItem(it, "published_on")))
      ni->published_on = (long)v->valuedouble;
    gNewsCount++;
  }
  cJSON_Delete(root);
  gNewsLoaded = true;
  gNewsLoading = false;
  return 0;
}

/* ── Public API ──────────────────────────────────────── */
void ApiInit(void) { curl_global_init(CURL_GLOBAL_DEFAULT); }
void ApiCleanup(void) { curl_global_cleanup(); }

void FetchPricesAsync(void) {
  if (gPricesLoading)
    return;
  gPricesLoading = true;
  gPricesError = false;
  CreateThread(NULL, 0, PricesThread, NULL, 0, NULL);
}

void FetchOHLCVAsync(const char *coinId, const char *coinName, int days) {
  if (gOHLCVLoading)
    return;
  strncpy(gSelectedCoinId, coinId, 63);
  strncpy(gSelectedCoinName, coinName, 63);
  gOHLCVDays = days;
  gOHLCVLoading = true;
  gOHLCVLoaded = false;
  gOHLCVError = false;
  CreateThread(NULL, 0, OHLCVThread, NULL, 0, NULL);
}

void FetchNewsAsync(void) {
  if (gNewsLoading)
    return;
  gNewsLoading = true;
  gNewsError = false;
  CreateThread(NULL, 0, NewsThread, NULL, 0, NULL);
}

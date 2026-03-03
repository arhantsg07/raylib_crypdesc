#ifndef SCREENS_H
#define SCREENS_H

// ── Screen identifiers ──────────────────────────────────
typedef enum {
  SCREEN_HOME = 0,
  SCREEN_PRICES,
  SCREEN_TRENDS,
  SCREEN_NEWS,
} GameScreen;

// Shared state used by main.c to drive transitions
extern GameScreen currentScreen;
extern GameScreen nextScreen;
extern float screenFade; // 0‑1 fade-in alpha

// ── Home ────────────────────────────────────────────────
void HomeInit(void);
void HomeUpdate(void);
void HomeDraw(void);
void HomeUnload(void);

// ── Prices ──────────────────────────────────────────────
void PricesInit(void);
void PricesUpdate(void);
void PricesDraw(void);
void PricesUnload(void);

// ── Trends ──────────────────────────────────────────────
void TrendsInit(void);
void TrendsUpdate(void);
void TrendsDraw(void);
void TrendsUnload(void);

// ── News ────────────────────────────────────────────────
void NewsInit(void);
void NewsUpdate(void);
void NewsDraw(void);
void NewsUnload(void);

#endif // SCREENS_H

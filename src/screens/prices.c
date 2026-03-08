/* prices.c – Cryptocurrency price table with mini bar charts */

#include "api.h"
#include "raylib.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"

#include <stdio.h>
#include <string.h>

/* ── Helper macros for DrawTextEx / MeasureTextEx ──────── */
#define DRAW_TEXT(text, x, y, size, color)                                     \
  DrawTextEx(gAppFont, (text), (Vector2){(float)(x), (float)(y)},              \
             (float)(size), FONT_SPACING, (color))

#define MEASURE_TEXT(text, size)                                               \
  MeasureTextEx(gAppFont, (text), (float)(size), FONT_SPACING).x

/* ── Local state ─────────────────────────────────────── */
static float scrollY = 0;
static float refreshTimer = 0;
#define REFRESH_INTERVAL 30.0f

void PricesInit(void) {
  scrollY = 0;
  refreshTimer = 0;
  if (!gPricesLoaded && !gPricesLoading)
    FetchPricesAsync();
}
void PricesUnload(void) {}

void PricesUpdate(void) {
  /* ESC = go back */
  if (IsKeyPressed(KEY_ESCAPE)) {
    nextScreen = SCREEN_HOME;
    return;
  }

  /* scroll */
  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    scrollY -= wheel * SCROLL_SPEED;
    float maxScroll = (float)(gPriceCount * TABLE_ROW_H) -
                      (WINDOW_H - HEADER_H - TABLE_HEADER_H - PAD_MD);
    if (maxScroll < 0)
      maxScroll = 0;
    if (scrollY < 0)
      scrollY = 0;
    if (scrollY > maxScroll)
      scrollY = maxScroll;
  }

  /* auto-refresh */
  refreshTimer += GetFrameTime();
  if (refreshTimer >= REFRESH_INTERVAL && !gPricesLoading) {
    refreshTimer = 0;
    FetchPricesAsync();
  }
}

void PricesDraw(void) {
  DrawGradientBg();

  /* Header */
  if (DrawHeaderBar("PRICES")) {
    nextScreen = SCREEN_HOME;
    return;
  }

  /* Refresh indicator */
  if (gPricesLoaded && !gPricesLoading) {
    int rem = (int)(REFRESH_INTERVAL - refreshTimer);
    if (rem < 0)
      rem = 0;
    char tbuf[32];
    snprintf(tbuf, sizeof(tbuf), "Refresh in %ds", rem);
    float rw = MEASURE_TEXT(tbuf, FONT_TINY);
    DRAW_TEXT(tbuf, WINDOW_W - rw - PAD_MD, (HEADER_H - FONT_TINY) / 2.0f,
              FONT_TINY, TEXT_MUTED);
  }

  float y0 = HEADER_H + PAD_SM;

  /* ── Loading / Error ─────────────────────────────── */
  if (gPricesLoading && !gPricesLoaded) {
    DrawLoadingSpinner((Vector2){WINDOW_W / 2.0f, WINDOW_H / 2.0f}, 30);
    return;
  }
  if (gPricesError && !gPricesLoaded) {
    DrawErrorCard((Rectangle){WINDOW_W / 2 - 200, WINDOW_H / 2 - 50, 400, 100},
                  "Failed to fetch prices. Check your internet.");
    return;
  }

  /* ── Column layout ───────────────────────────────── */
  /* #(40) Name(160) Symbol(80) Price(120) 24h%(80) 7d%(80) MCap(120) Bar(rest)
   */
  float cols[] = {40, 160, 80, 120, 80, 80, 120};
  const char *hdrs[] = {"#",    "Name", "Symbol",    "Price",
                        "24h%", "7d%",  "Market Cap"};
  int ncols = 7;

  float cx = PAD_LG;
  float tableW = WINDOW_W - PAD_LG * 2;

  /* Bar column takes remaining width */
  float usedW = 0;
  for (int i = 0; i < ncols; i++)
    usedW += cols[i];
  float barW = tableW - usedW;
  if (barW < 60)
    barW = 60;

  /* ── Header row ──────────────────────────────────── */
  DrawRectangle(0, (int)y0, WINDOW_W, TABLE_HEADER_H, BG_TERTIARY);
  float hx = cx;
  for (int i = 0; i < ncols; i++) {
    DRAW_TEXT(hdrs[i], hx, y0 + (TABLE_HEADER_H - FONT_SMALL) / 2.0f,
              FONT_SMALL, TEXT_SECONDARY);
    hx += cols[i];
  }
  DRAW_TEXT("Volume Bar", hx, y0 + (TABLE_HEADER_H - FONT_SMALL) / 2.0f,
            FONT_SMALL, TEXT_SECONDARY);

  y0 += TABLE_HEADER_H;

  /* ── Scissor for rows ────────────────────────────── */
  float tableH = WINDOW_H - y0 - PAD_SM;
  BeginScissorMode(0, (int)y0, WINDOW_W, (int)tableH);

  /* find max market cap for bar scaling */
  double maxMcap = 1;
  for (int i = 0; i < gPriceCount; i++)
    if (gPrices[i].market_cap > maxMcap)
      maxMcap = gPrices[i].market_cap;

  for (int i = 0; i < gPriceCount; i++) {
    float ry = y0 + i * TABLE_ROW_H - scrollY;
    if (ry + TABLE_ROW_H < y0 || ry > WINDOW_H)
      continue; /* cull */

    CryptoPrice *p = &gPrices[i];

    /* Alternating row bg */
    if (i % 2 == 0)
      DrawRectangle(0, (int)ry, WINDOW_W, TABLE_ROW_H,
                    (Color){22, 27, 34, 120});

    /* hover highlight */
    Vector2 mouse = GetMousePosition();
    Rectangle rowRect = {0, ry, (float)WINDOW_W, TABLE_ROW_H};
    if (CheckCollisionPointRec(mouse, rowRect))
      DrawRectangle(0, (int)ry, WINDOW_W, TABLE_ROW_H,
                    (Color){88, 166, 255, 20});

    float ty = ry + (TABLE_ROW_H - FONT_BODY) / 2.0f;
    float rx = cx;

    /* Rank */
    char rankBuf[8];
    snprintf(rankBuf, sizeof(rankBuf), "%d", p->rank);
    DRAW_TEXT(rankBuf, rx, ty, FONT_BODY, TEXT_SECONDARY);
    rx += cols[0];

    /* Name */
    DRAW_TEXT(p->name, rx, ty, FONT_BODY, TEXT_PRIMARY);
    rx += cols[1];

    /* Symbol */
    DRAW_TEXT(p->symbol, rx, ty, FONT_BODY, ACCENT_BLUE);
    rx += cols[2];

    /* Price */
    DRAW_TEXT(FormatPrice(p->price), rx, ty, FONT_BODY, TEXT_PRIMARY);
    rx += cols[3];

    /* 24h% */
    DRAW_TEXT(FormatPercent(p->percent_change_24h), rx, ty, FONT_BODY,
              PctColor(p->percent_change_24h));
    rx += cols[4];

    /* 7d% */
    DRAW_TEXT(FormatPercent(p->percent_change_7d), rx, ty, FONT_BODY,
              PctColor(p->percent_change_7d));
    rx += cols[5];

    /* Market Cap */
    DRAW_TEXT(FormatLargeNumber(p->market_cap), rx, ty, FONT_BODY,
              TEXT_SECONDARY);
    rx += cols[6];

    /* Mini bar */
    DrawMiniBar((Rectangle){rx, ry + 10, barW - PAD_MD, TABLE_ROW_H - 20},
                p->market_cap, maxMcap, PctColor(p->percent_change_24h));
  }

  EndScissorMode();

  /* ── Scrollbar ───────────────────────────────────── */
  float contentH = (float)(gPriceCount * TABLE_ROW_H);
  if (contentH > tableH) {
    float ratio = tableH / contentH;
    float barH = tableH * ratio;
    if (barH < 30)
      barH = 30;
    float maxS = contentH - tableH;
    float barY = y0 + (scrollY / maxS) * (tableH - barH);
    DrawRectangleRounded(
        (Rectangle){WINDOW_W - SCROLLBAR_W - 2, barY, SCROLLBAR_W, barH}, 0.5f,
        4, (Color){88, 166, 255, 80});
  }
}

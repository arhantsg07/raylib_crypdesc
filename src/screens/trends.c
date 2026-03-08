/* trends.c – Coin selector + candlestick OHLCV chart */

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
static int selectedIdx = -1; /* index into gPrices[] */
static int activeDays = 30;
static float listScroll = 0;

void TrendsInit(void) {
  selectedIdx = -1;
  activeDays = 30;
  listScroll = 0;
  /* Make sure we have prices for the coin list */
  if (!gPricesLoaded && !gPricesLoading)
    FetchPricesAsync();
}
void TrendsUnload(void) {}

void TrendsUpdate(void) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    nextScreen = SCREEN_HOME;
    return;
  }

  /* Scroll the coin list on the left */
  Vector2 mouse = GetMousePosition();
  if (mouse.x < 260) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      listScroll -= wheel * SCROLL_SPEED;
      float maxS =
          (float)(gPriceCount * 44) - (WINDOW_H - HEADER_H - PAD_MD * 2 - 80);
      if (maxS < 0)
        maxS = 0;
      if (listScroll < 0)
        listScroll = 0;
      if (listScroll > maxS)
        listScroll = maxS;
    }
  }
}

void TrendsDraw(void) {
  DrawGradientBg();

  if (DrawHeaderBar("TRENDS")) {
    nextScreen = SCREEN_HOME;
    return;
  }

  float y0 = HEADER_H + PAD_SM;

  /* ── Left panel: coin selector ───────────────────── */
  float panelW = 240;
  Rectangle panelR = {PAD_MD, y0, panelW, WINDOW_H - y0 - PAD_MD};

  DrawCard(panelR, BG_SECONDARY, BORDER_DEFAULT);

  /* Panel title */
  DRAW_TEXT("Select a coin", panelR.x + PAD_SM, panelR.y + PAD_SM, FONT_SMALL,
            TEXT_SECONDARY);

  if (gPricesLoading && !gPricesLoaded) {
    DrawLoadingSpinner(
        (Vector2){panelR.x + panelW / 2, panelR.y + panelR.height / 2}, 20);
  } else if (gPriceCount > 0) {
    float listY0 = panelR.y + PAD_SM + FONT_SMALL + PAD_SM;
    float listH = panelR.height - PAD_SM - FONT_SMALL - PAD_SM * 2;

    BeginScissorMode((int)panelR.x, (int)listY0, (int)panelW, (int)listH);
    for (int i = 0; i < gPriceCount; i++) {
      float iy = listY0 + i * 44 - listScroll;
      if (iy + 44 < listY0 || iy > panelR.y + panelR.height)
        continue;

      Rectangle itemR = {panelR.x + PAD_XS, iy, panelW - PAD_XS * 2, 40};
      Vector2 mouse = GetMousePosition();
      bool hov = CheckCollisionPointRec(mouse, itemR);
      bool sel = (i == selectedIdx);

      Color bg = sel   ? (Color){88, 166, 255, 40}
                 : hov ? (Color){88, 166, 255, 15}
                       : (Color){0, 0, 0, 0};
      DrawRectangleRounded(itemR, 0.2f, 4, bg);

      if (sel)
        DrawRectangleRoundedLinesEx(itemR, 0.2f, 4, 1, ACCENT_BLUE);

      /* Rank + name */
      char label[80];
      snprintf(label, sizeof(label), "%d. %s (%s)", gPrices[i].rank,
               gPrices[i].name, gPrices[i].symbol);
      DRAW_TEXT(label, itemR.x + PAD_SM, itemR.y + (40 - FONT_SMALL) / 2.0f,
                FONT_SMALL, sel ? ACCENT_BLUE : TEXT_PRIMARY);

      if (hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !sel) {
        selectedIdx = i;
        FetchOHLCVAsync(gPrices[i].id, gPrices[i].name, activeDays);
      }
    }
    EndScissorMode();
  }

  /* ── Right panel: chart area ─────────────────────── */
  float chartX = panelW + PAD_MD * 2 + PAD_SM;
  float chartW = WINDOW_W - chartX - PAD_MD;
  Rectangle chartPanel = {chartX, y0, chartW, WINDOW_H - y0 - PAD_MD};
  DrawCard(chartPanel, BG_SECONDARY, BORDER_DEFAULT);

  if (selectedIdx < 0) {
    /* No coin selected */
    const char *msg = "Select a coin from the left panel";
    float mw = MEASURE_TEXT(msg, FONT_BODY);
    DRAW_TEXT(msg, chartPanel.x + chartPanel.width / 2 - mw / 2,
              chartPanel.y + chartPanel.height / 2 - FONT_BODY / 2.0f,
              FONT_BODY, TEXT_MUTED);
    return;
  }

  /* ── Time range pills ────────────────────────────── */
  float px = chartPanel.x + PAD_MD;
  float py = chartPanel.y + PAD_SM;

  /* Coin name */
  DRAW_TEXT(gSelectedCoinName, px, py, FONT_SUBHEADING, TEXT_PRIMARY);
  py += FONT_SUBHEADING + PAD_SM;

  /* Price display */
  if (selectedIdx >= 0 && selectedIdx < gPriceCount) {
    DRAW_TEXT(FormatPrice(gPrices[selectedIdx].price), px, py, FONT_HEADING,
              TEXT_PRIMARY);

    const char *pctStr = FormatPercent(gPrices[selectedIdx].percent_change_24h);
    float priceW =
        MEASURE_TEXT(FormatPrice(gPrices[selectedIdx].price), FONT_HEADING);
    DRAW_TEXT(pctStr, px + priceW + PAD_MD, py + 6, FONT_BODY,
              PctColor(gPrices[selectedIdx].percent_change_24h));
  }
  py += FONT_HEADING + PAD_MD;

  /* Time range buttons */
  typedef struct {
    const char *label;
    int days;
  } TR;
  TR ranges[] = {{"7D", 7}, {"30D", 30}, {"90D", 90}};
  float pillW = 50, pillH = 28, pillGap = 8;

  for (int i = 0; i < 3; i++) {
    Rectangle pr = {px + i * (pillW + pillGap), py, pillW, pillH};
    if (DrawPillButton(pr, ranges[i].label, activeDays == ranges[i].days)) {
      if (activeDays != ranges[i].days) {
        activeDays = ranges[i].days;
        FetchOHLCVAsync(gPrices[selectedIdx].id, gPrices[selectedIdx].name,
                        activeDays);
      }
    }
  }
  py += pillH + PAD_MD;

  /* ── Candlestick chart ───────────────────────────── */
  float chartAreaH = chartPanel.y + chartPanel.height - py - PAD_MD;
  Rectangle cr = {chartPanel.x + PAD_SM, py, chartPanel.width - PAD_SM * 2,
                  chartAreaH};

  if (gOHLCVLoading) {
    DrawLoadingSpinner((Vector2){cr.x + cr.width / 2, cr.y + cr.height / 2},
                       25);
  } else if (gOHLCVError) {
    DrawErrorCard((Rectangle){cr.x + cr.width / 2 - 180,
                              cr.y + cr.height / 2 - 40, 360, 80},
                  "Failed to load chart data.");
  } else if (gOHLCVLoaded && gOHLCVCount > 0) {
    DrawCandlestickChart(cr, gOHLCV, gOHLCVCount);
  }
}

/* ui.c – Reusable drawing helpers for CryptoDesk */

#include "ui.h"
#include "theme.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

/* ── Helper macros for DrawTextEx / MeasureTextEx ──────── */
#define DRAW_TEXT(text, x, y, size, color)                                     \
  DrawTextEx(gAppFont, (text), (Vector2){(float)(x), (float)(y)},              \
             (float)(size), FONT_SPACING, (color))

#define MEASURE_TEXT(text, size)                                               \
  MeasureTextEx(gAppFont, (text), (float)(size), FONT_SPACING).x

/* ══════════════════════════════════════════════════════
   Background
   ══════════════════════════════════════════════════════ */
void DrawGradientBg(void) {
  DrawRectangleGradientV(0, 0, WINDOW_W, WINDOW_H, BG_PRIMARY, BG_SECONDARY);
}

/* ══════════════════════════════════════════════════════
   Cards & containers
   ══════════════════════════════════════════════════════ */
void DrawCard(Rectangle r, Color bg, Color border) {
  DrawRectangleRounded(r, CARD_ROUNDNESS, 8, bg);
  DrawRectangleRoundedLinesEx(r, CARD_ROUNDNESS, 8, CARD_BORDER_W, border);
}

bool DrawCardButton(Rectangle r, const char *label, const char *icon,
                    Color accent) {
  Vector2 mouse = GetMousePosition();
  bool hovered = CheckCollisionPointRec(mouse, r);

  Color bg = hovered ? CARD_BG_HOVER : CARD_BG;
  Color bdr = hovered ? accent : BORDER_DEFAULT;

  /* subtle glow behind card on hover */
  if (hovered) {
    Color glow = accent;
    glow.a = 25;
    Rectangle gr = {r.x - 4, r.y - 4, r.width + 8, r.height + 8};
    DrawRectangleRounded(gr, CARD_ROUNDNESS, 8, glow);
  }

  DrawCard(r, bg, bdr);

  /* icon character */
  if (icon) {
    float iw = MEASURE_TEXT(icon, FONT_HEADING + 8);
    DRAW_TEXT(icon, r.x + r.width / 2 - iw / 2, r.y + r.height * 0.25f,
              FONT_HEADING + 8, accent);
  }

  /* label */
  float tw = MEASURE_TEXT(label, FONT_BODY);
  DRAW_TEXT(label, r.x + r.width / 2 - tw / 2, r.y + r.height * 0.65f,
            FONT_BODY, TEXT_PRIMARY);

  return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/* ══════════════════════════════════════════════════════
   Header bar  (returns true when Back ← is clicked)
   ══════════════════════════════════════════════════════ */
bool DrawHeaderBar(const char *title) {
  /* Background strip */
  DrawRectangle(0, 0, WINDOW_W, HEADER_H, BG_TERTIARY);
  DrawRectangle(0, HEADER_H - 1, WINDOW_W, 1, BORDER_DEFAULT);

  /* Back button */
  bool backClicked = false;
  Rectangle btn = {PAD_MD, (HEADER_H - 30) / 2.0f, 70, 30};
  Vector2 mouse = GetMousePosition();
  bool hov = CheckCollisionPointRec(mouse, btn);

  Color col = hov ? ACCENT_BLUE : TEXT_SECONDARY;
  DRAW_TEXT("<  Back", btn.x, btn.y + 6, FONT_SMALL, col);
  if (hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    backClicked = true;

  /* Title */
  float tw = MEASURE_TEXT(title, FONT_HEADING);
  DRAW_TEXT(title, WINDOW_W / 2.0f - tw / 2, (HEADER_H - FONT_HEADING) / 2.0f,
            FONT_HEADING, TEXT_PRIMARY);

  return backClicked;
}

/* ══════════════════════════════════════════════════════
   Pill / toggle buttons
   ══════════════════════════════════════════════════════ */
bool DrawPillButton(Rectangle r, const char *text, bool active) {
  Vector2 mouse = GetMousePosition();
  bool hov = CheckCollisionPointRec(mouse, r);

  Color bg = active ? ACCENT_BLUE : (hov ? BG_TERTIARY : BG_SECONDARY);
  Color fg = active ? BG_PRIMARY : TEXT_SECONDARY;

  DrawRectangleRounded(r, 0.5f, 8, bg);
  if (!active)
    DrawRectangleRoundedLinesEx(r, 0.5f, 8, 1, BORDER_DEFAULT);

  float tw = MEASURE_TEXT(text, FONT_SMALL);
  DRAW_TEXT(text, r.x + r.width / 2 - tw / 2,
            r.y + r.height / 2 - FONT_SMALL / 2.0f, FONT_SMALL, fg);

  return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool DrawTextButton(float x, float y, const char *text, int size, Color col) {
  Vector2 mouse = GetMousePosition();
  float tw = MEASURE_TEXT(text, size);
  Rectangle r = {x, y, tw, (float)size};
  bool hov = CheckCollisionPointRec(mouse, r);

  Color dc = hov ? ACCENT_BLUE : col;
  DRAW_TEXT(text, x, y, size, dc);

  return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/* ══════════════════════════════════════════════════════
   Mini bar chart  (single horizontal bar in a cell)
   ══════════════════════════════════════════════════════ */
void DrawMiniBar(Rectangle r, double value, double maxVal, Color col) {
  if (maxVal <= 0)
    return;
  float pct = (float)(value / maxVal);
  if (pct < 0)
    pct = 0;
  if (pct > 1)
    pct = 1;

  /* track */
  Color track = BORDER_DEFAULT;
  DrawRectangleRounded(
      (Rectangle){r.x, r.y + r.height * 0.3f, r.width, r.height * 0.4f}, 0.5f,
      4, track);

  /* fill */
  float fw = r.width * pct;
  if (fw < 4)
    fw = 4;
  DrawRectangleRounded(
      (Rectangle){r.x, r.y + r.height * 0.3f, fw, r.height * 0.4f}, 0.5f, 4,
      col);
}

/* ══════════════════════════════════════════════════════
   Candlestick chart
   ══════════════════════════════════════════════════════ */
void DrawCandlestickChart(Rectangle r, OHLCVData *data, int count) {
  if (count < 2)
    return;

  /* Find price range */
  double minP = data[0].low, maxP = data[0].high;
  for (int i = 1; i < count; i++) {
    if (data[i].low < minP)
      minP = data[i].low;
    if (data[i].high > maxP)
      maxP = data[i].high;
  }
  double range = maxP - minP;
  if (range < 0.0001)
    range = 1.0;

  /* Padding for labels */
  float chartL = r.x + 80;
  float chartR = r.x + r.width - PAD_MD;
  float chartT = r.y + PAD_MD;
  float chartB = r.y + r.height - 40;
  float chartW = chartR - chartL;
  float chartH = chartB - chartT;

  /* Grid lines + Y labels */
  int gridLines = 5;
  for (int i = 0; i <= gridLines; i++) {
    float yy = chartT + chartH * i / gridLines;
    DrawLine((int)chartL, (int)yy, (int)chartR, (int)yy,
             (Color){48, 54, 61, 100});

    double val = maxP - range * i / gridLines;
    DRAW_TEXT(FormatPrice(val), r.x + 4, yy - 6, FONT_TINY, TEXT_MUTED);
  }

  /* Candles */
  float candleW = chartW / count;
  float bodyW = candleW * 0.6f;
  if (bodyW < 3)
    bodyW = 3;

  for (int i = 0; i < count; i++) {
    float cx = chartL + candleW * i + candleW / 2;

    float yHigh = chartT + (float)((maxP - data[i].high) / range) * chartH;
    float yLow = chartT + (float)((maxP - data[i].low) / range) * chartH;
    float yOpen = chartT + (float)((maxP - data[i].open) / range) * chartH;
    float yClose = chartT + (float)((maxP - data[i].close) / range) * chartH;

    bool bullish = data[i].close >= data[i].open;
    Color col = bullish ? POSITIVE_GREEN : NEGATIVE_RED;

    /* Wick */
    DrawLineEx((Vector2){cx, yHigh}, (Vector2){cx, yLow}, 1.5f, col);

    /* Body */
    float bodyTop = bullish ? yClose : yOpen;
    float bodyBot = bullish ? yOpen : yClose;
    float bodyH = bodyBot - bodyTop;
    if (bodyH < 1)
      bodyH = 1;

    DrawRectangleRounded((Rectangle){cx - bodyW / 2, bodyTop, bodyW, bodyH},
                         0.15f, 4, col);
  }

  /* X-axis date labels (every few candles) */
  int step = count / 6;
  if (step < 1)
    step = 1;
  for (int i = 0; i < count; i += step) {
    float cx = chartL + candleW * i + candleW / 2;
    /* date is like "2024-01-15T..." – show just MM-DD */
    char lbl[8] = "";
    if (strlen(data[i].date) >= 10) {
      memcpy(lbl, data[i].date + 5, 5);
      lbl[5] = '\0';
    }
    float tw = MEASURE_TEXT(lbl, FONT_TINY);
    DRAW_TEXT(lbl, cx - tw / 2.0f, chartB + 8, FONT_TINY, TEXT_MUTED);
  }
}

/* ══════════════════════════════════════════════════════
   Loading spinner
   ══════════════════════════════════════════════════════ */
void DrawLoadingSpinner(Vector2 center, float radius) {
  float t = (float)GetTime();

  for (int i = 0; i < 12; i++) {
    float angle = (float)i * 30.0f + t * 360.0f;
    float rad = angle * DEG2RAD;
    float x = center.x + cosf(rad) * radius;
    float y = center.y + sinf(rad) * radius;

    float alpha = (float)((i + 1)) / 12.0f;
    Color dot = ACCENT_BLUE;
    dot.a = (unsigned char)(alpha * 255);

    DrawCircleV((Vector2){x, y}, 3.5f, dot);
  }

  /* "Loading…" text */
  const char *lt = "Loading...";
  float tw = MEASURE_TEXT(lt, FONT_SMALL);
  DRAW_TEXT(lt, center.x - tw / 2.0f, center.y + radius + 16, FONT_SMALL,
            TEXT_SECONDARY);
}

/* ══════════════════════════════════════════════════════
   Error card
   ══════════════════════════════════════════════════════ */
void DrawErrorCard(Rectangle r, const char *msg) {
  DrawCard(r, BG_TERTIARY, NEGATIVE_RED);

  const char *hdr = "! Error";
  float hw = MEASURE_TEXT(hdr, FONT_BODY);
  DRAW_TEXT(hdr, r.x + r.width / 2 - hw / 2, r.y + PAD_MD, FONT_BODY,
            NEGATIVE_RED);

  float mw = MEASURE_TEXT(msg, FONT_SMALL);
  DRAW_TEXT(msg, r.x + r.width / 2 - mw / 2, r.y + PAD_MD + FONT_BODY + PAD_SM,
            FONT_SMALL, TEXT_SECONDARY);
}

/* ══════════════════════════════════════════════════════
   Formatting helpers (return static buffers – NOT thread safe,
   but only called from main / draw thread so that's fine)
   ══════════════════════════════════════════════════════ */
static char _priceBuf[8][32];
static int _priceIdx = 0;

const char *FormatPrice(double price) {
  char *buf = _priceBuf[_priceIdx++ & 7];
  if (price >= 1000.0)
    snprintf(buf, 32, "$%,.2f", price);
  else if (price >= 1.0)
    snprintf(buf, 32, "$%.2f", price);
  else
    snprintf(buf, 32, "$%.4f", price);
  return buf;
}

static char _largeBuf[8][32];
static int _largeIdx = 0;

const char *FormatLargeNumber(double num) {
  char *buf = _largeBuf[_largeIdx++ & 7];
  if (num >= 1e12)
    snprintf(buf, 32, "$%.1fT", num / 1e12);
  else if (num >= 1e9)
    snprintf(buf, 32, "$%.1fB", num / 1e9);
  else if (num >= 1e6)
    snprintf(buf, 32, "$%.1fM", num / 1e6);
  else
    snprintf(buf, 32, "$%.0f", num);
  return buf;
}

static char _pctBuf[8][16];
static int _pctIdx = 0;

const char *FormatPercent(double pct) {
  char *buf = _pctBuf[_pctIdx++ & 7];
  snprintf(buf, 16, "%+.2f%%", pct);
  return buf;
}

Color PctColor(double pct) {
  return pct >= 0.0 ? POSITIVE_GREEN : NEGATIVE_RED;
}

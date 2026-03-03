/* home.c – Main menu: Prices · Trends · News */

#include "raylib.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"

#include <math.h>

/* ── Helper macros for DrawTextEx / MeasureTextEx ──────── */
#define DRAW_TEXT(text, x, y, size, color)                                     \
  DrawTextEx(gAppFont, (text), (Vector2){(float)(x), (float)(y)},              \
             (float)(size), FONT_SPACING, (color))

#define MEASURE_TEXT(text, size)                                               \
  MeasureTextEx(gAppFont, (text), (float)(size), FONT_SPACING).x

/* ── Local state ─────────────────────────────────────── */
static float timeAcc = 0.0f;

void HomeInit(void) { timeAcc = 0.0f; }
void HomeUnload(void) {}

void HomeUpdate(void) { timeAcc += GetFrameTime(); }

void HomeDraw(void) {
  DrawGradientBg();

  /* ── Animated glow circle behind title ────────── */
  float pulse = 0.5f + 0.5f * sinf(timeAcc * 1.8f);
  Color glow = ACCENT_BLUE;
  glow.a = (unsigned char)(30 * pulse);
  DrawCircle(WINDOW_W / 2, 170, 120 + pulse * 20, glow);

  glow = ACCENT_PURPLE;
  glow.a = (unsigned char)(20 * pulse);
  DrawCircle(WINDOW_W / 2 + 40, 160, 80 + pulse * 15, glow);

  /* ── Bitcoin symbol (pulsing) ─────────────────── */
  float bScale = 1.0f + 0.05f * sinf(timeAcc * 2.5f);
  int bSize = (int)(FONT_TITLE * 1.6f * bScale);
  const char *btcSym = "B";
  float bw = MEASURE_TEXT(btcSym, bSize);

  /* circle behind B */
  float cy = 165 + sinf(timeAcc * 1.2f) * 6;
  DrawCircleV((Vector2){WINDOW_W / 2.0f, cy}, 46 * bScale,
              (Color){255, 183, 77, (unsigned char)(60 * pulse)});
  DrawCircleV((Vector2){WINDOW_W / 2.0f, cy}, 38 * bScale, BG_TERTIARY);
  DrawCircleLinesV((Vector2){WINDOW_W / 2.0f, cy}, 38 * bScale, ACCENT_GOLD);
  DRAW_TEXT(btcSym, WINDOW_W / 2.0f - bw / 2, cy - bSize / 2.0f, bSize,
            ACCENT_GOLD);

  /* ── Title ────────────────────────────────────── */
  const char *title = "CRYPTO DESK";
  float tw = MEASURE_TEXT(title, FONT_TITLE);
  DRAW_TEXT(title, WINDOW_W / 2.0f - tw / 2, 240, FONT_TITLE, TEXT_PRIMARY);

  /* subtitle */
  const char *sub = "Real-time cryptocurrency dashboard";
  float sw = MEASURE_TEXT(sub, FONT_SMALL);
  DRAW_TEXT(sub, WINDOW_W / 2.0f - sw / 2, 296, FONT_SMALL, TEXT_SECONDARY);

  /* ── Three menu cards ─────────────────────────── */
  float cardW = 220, cardH = 160, gap = 40;
  float totalW = cardW * 3 + gap * 2;
  float startX = (WINDOW_W - totalW) / 2.0f;
  float cardY = 370;

  /* Card 1 – Prices */
  if (DrawCardButton((Rectangle){startX, cardY, cardW, cardH}, "PRICES", "$",
                     ACCENT_BLUE))
    nextScreen = SCREEN_PRICES;

  /* Card 2 – Trends */
  if (DrawCardButton((Rectangle){startX + cardW + gap, cardY, cardW, cardH},
                     "TRENDS", "~", ACCENT_PURPLE))
    nextScreen = SCREEN_TRENDS;

  /* Card 3 – News */
  if (DrawCardButton(
          (Rectangle){startX + 2 * (cardW + gap), cardY, cardW, cardH},
          "LATEST NEWS", "#", ACCENT_CYAN))
    nextScreen = SCREEN_NEWS;

  /* ── Footer ───────────────────────────────────── */
  const char *foot = "Powered by CoinPaprika & CryptoCompare";
  float fw = MEASURE_TEXT(foot, FONT_TINY);
  DRAW_TEXT(foot, WINDOW_W / 2.0f - fw / 2, WINDOW_H - 36, FONT_TINY,
            TEXT_MUTED);

  const char *ver = "v1.0";
  float vw = MEASURE_TEXT(ver, FONT_TINY);
  DRAW_TEXT(ver, WINDOW_W / 2.0f - vw / 2, WINDOW_H - 20, FONT_TINY,
            TEXT_MUTED);
}

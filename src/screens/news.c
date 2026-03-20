/* news.c – Scrollable cryptocurrency news card feed */

#include "api.h"
#include "raylib.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ── Helper macros for DrawTextEx / MeasureTextEx ──────── */
#define DRAW_TEXT(text, x, y, size, color)                                     \
  DrawTextEx(gAppFont, (text), (Vector2){(float)(x), (float)(y)},              \
             (float)(size), FONT_SPACING, (color))

#define MEASURE_TEXT(text, size)                                               \
  MeasureTextEx(gAppFont, (text), (float)(size), FONT_SPACING).x

/* ── Local state ─────────────────────────────────────── */
static float scrollY = 0;
static float refreshTimer = 0;
#define NEWS_REFRESH 60.0f
#define CARD_H 130
#define CARD_GAP 12

void NewsInit(void) {
  scrollY = 0;
  refreshTimer = 0;
  if (!gNewsLoaded && !gNewsLoading)
    FetchNewsAsync();
}
void NewsUnload(void) {}

void NewsUpdate(void) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    nextScreen = SCREEN_HOME;
    return;
  }

  /* scroll */
  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    scrollY -= wheel * SCROLL_SPEED;
    float maxS = (float)(gNewsCount * (CARD_H + CARD_GAP)) -
                 (WINDOW_H - HEADER_H - PAD_MD * 2);
    if (maxS < 0)
      maxS = 0;
    if (scrollY < 0)
      scrollY = 0;
    if (scrollY > maxS)
      scrollY = maxS;
  }

  /* auto-refresh */
  refreshTimer += GetFrameTime();
  if (refreshTimer >= NEWS_REFRESH && !gNewsLoading) {
    refreshTimer = 0;
    FetchNewsAsync();
  }
}

/* Helper: relative time string */
static const char *TimeAgo(long ts) {
  static char buf[32];
  long now = (long)time(NULL);
  long diff = now - ts;
  if (diff < 60)
    snprintf(buf, sizeof(buf), "just now");
  else if (diff < 3600)
    snprintf(buf, sizeof(buf), "%ldm ago", diff / 60);
  else if (diff < 86400)
    snprintf(buf, sizeof(buf), "%ldh ago", diff / 3600);
  else
    snprintf(buf, sizeof(buf), "%ldd ago", diff / 86400);
  return buf;
}

/* Helper: truncate text to fit width */
static void DrawTextTruncated(const char *text, int x, int y, int fontSize,
                              Color color, int maxWidth) {
  if (MEASURE_TEXT(text, fontSize) <= maxWidth) {
    DRAW_TEXT(text, x, y, fontSize, color);
    return;
  }
  /* binary-ish search for how many chars fit */
  int len = (int)strlen(text);
  char buf[512];
  for (int n = len; n > 0; n--) {
    if (n > 508)
      continue;
    memcpy(buf, text, n);
    buf[n] = '\0';
    strcat(buf, "...");
    if (MEASURE_TEXT(buf, fontSize) <= maxWidth) {
      DRAW_TEXT(buf, x, y, fontSize, color);
      return;
    }
  }
  DRAW_TEXT("...", x, y, fontSize, color);
}

void NewsDraw(void) {
  DrawGradientBg();

  if (DrawHeaderBar("LATEST NEWS")) {
    nextScreen = SCREEN_HOME;
    return;
  }

  /* Refresh timer */
  if (gNewsLoaded && !gNewsLoading) {
    int rem = (int)(NEWS_REFRESH - refreshTimer);
    if (rem < 0)
      rem = 0;
    char tbuf[32];
    snprintf(tbuf, sizeof(tbuf), "Refresh in %ds", rem);
    float rw = MEASURE_TEXT(tbuf, FONT_TINY);
    DRAW_TEXT(tbuf, WINDOW_W - rw - PAD_MD, (HEADER_H - FONT_TINY) / 2.0f,
              FONT_TINY, TEXT_MUTED);
  }

  float y0 = HEADER_H + PAD_MD;

  /* Loading / Error */
  if (gNewsLoading && !gNewsLoaded) {
    DrawLoadingSpinner((Vector2){WINDOW_W / 2.0f, WINDOW_H / 2.0f}, 30);
    return;
  }
  if (gNewsError && !gNewsLoaded) {
    DrawErrorCard((Rectangle){WINDOW_W / 2 - 200, WINDOW_H / 2 - 50, 400, 100},
                  "Failed to fetch news. Check your internet.");
    return;
  }

  /* ── Scrollable card list ────────────────────────── */
  float listH = WINDOW_H - y0 - PAD_SM;
  BeginScissorMode(0, (int)y0, WINDOW_W, (int)listH);

  float cardW = WINDOW_W - PAD_LG * 2;

  for (int i = 0; i < gNewsCount; i++) {
    float cy = y0 + i * (CARD_H + CARD_GAP) - scrollY;
    if (cy + CARD_H < y0 || cy > WINDOW_H)
      continue;

    NewsItem *n = &gNews[i];

    Rectangle cr = {(float)PAD_LG, cy, cardW, CARD_H};

    /* Hover detect */
    Vector2 mouse = GetMousePosition();
    bool hov = CheckCollisionPointRec(mouse, cr);

    Color bg = hov ? CARD_BG_HOVER : CARD_BG;
    Color bdr = hov ? ACCENT_CYAN : BORDER_DEFAULT;
    DrawCard(cr, bg, bdr);

    float tx = cr.x + PAD_MD;
    float ty = cr.y + PAD_SM;
    float contentW = cardW - PAD_MD * 2;

    /* Source + time */
    char meta[128];
    snprintf(meta, sizeof(meta), "%s  |  %s", n->source,
             TimeAgo(n->published_on));
    DRAW_TEXT(meta, tx, ty, FONT_TINY, ACCENT_CYAN);
    ty += FONT_TINY + PAD_XS;

    /* Title */
    DrawTextTruncated(n->title, (int)tx, (int)ty, FONT_BODY, TEXT_PRIMARY,
                      (int)contentW);
    ty += FONT_BODY + PAD_XS;

    /* Body snippet – up to 2 lines */
    DrawTextTruncated(n->body, (int)tx, (int)ty, FONT_SMALL, TEXT_SECONDARY,
                      (int)contentW);
    ty += FONT_SMALL + 2;
    if ((int)strlen(n->body) > 80) {
      DrawTextTruncated(n->body + 80, (int)tx, (int)ty, FONT_SMALL,
                        TEXT_SECONDARY, (int)contentW);
    }
  }

  EndScissorMode();

  /* ── Scrollbar ───────────────────────────────────── */
  float contentH = (float)(gNewsCount * (CARD_H + CARD_GAP));
  if (contentH > listH) {
    float ratio = listH / contentH;
    float barH = listH * ratio;
    if (barH < 30)
      barH = 30;
    float maxS = contentH - listH;
    float barY = y0 + (scrollY / maxS) * (listH - barH);
    DrawRectangleRounded(
        (Rectangle){WINDOW_W - SCROLLBAR_W - 2, barY, SCROLLBAR_W, barH}, 0.5f,
        4, (Color){56, 211, 215, 80});
  }
}

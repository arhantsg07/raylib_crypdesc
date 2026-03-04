/* main.c – CryptoDesk entry point & screen state machine */

#include "api.h"
#include "raylib.h"
#include "screens.h"
#include "theme.h"

/* Shared screen state */
GameScreen currentScreen = SCREEN_HOME;
GameScreen nextScreen = SCREEN_HOME;
float screenFade = 0.0f;

/* Global custom font */
Font gAppFont = {0};

/* Forward: per-screen lifecycle tables */
typedef void (*ScreenFn)(void);
static ScreenFn initFns[] = {HomeInit, PricesInit, TrendsInit, NewsInit};
static ScreenFn updateFns[] = {HomeUpdate, PricesUpdate, TrendsUpdate,
                               NewsUpdate};
static ScreenFn drawFns[] = {HomeDraw, PricesDraw, TrendsDraw, NewsDraw};
static ScreenFn unloadFns[] = {HomeUnload, PricesUnload, TrendsUnload,
                               NewsUnload};

int main(void) {
  /* ── Window ──────────────────────────────────────── */
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(WINDOW_W, WINDOW_H, "CryptoDesk");
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_NULL); /* disable ESC-to-close; we use ESC for back */

  /* Load custom font */
  gAppFont = LoadFontEx("assets/IBMPlexSans-Regular.ttf", FONT_TITLE, 0, 0);
  SetTextureFilter(gAppFont.texture, TEXTURE_FILTER_BILINEAR);

  ApiInit();
  initFns[currentScreen]();
  screenFade = 0.0f;

  /* ── Main loop ───────────────────────────────────── */
  while (!WindowShouldClose()) {
    /* fade-in animation */
    if (screenFade < 1.0f) {
      screenFade += GetFrameTime() * 4.0f;
      if (screenFade > 1.0f)
        screenFade = 1.0f;
    }

    /* screen transition */
    if (nextScreen != currentScreen) {
      unloadFns[currentScreen]();
      currentScreen = nextScreen;
      initFns[currentScreen]();
      screenFade = 0.0f;
    }

    updateFns[currentScreen]();

    BeginDrawing();
    drawFns[currentScreen]();

    /* fade overlay */
    if (screenFade < 1.0f) {
      unsigned char a = (unsigned char)((1.0f - screenFade) * 255);
      DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){13, 17, 23, a});
    }
    EndDrawing();
  }

  /* ── Cleanup ─────────────────────────────────────── */
  unloadFns[currentScreen]();
  ApiCleanup();
  UnloadFont(gAppFont);
  CloseWindow();
  return 0;
}

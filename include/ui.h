#ifndef UI_H
#define UI_H

#include "api.h"
#include "raylib.h"


// ── Background ──────────────────────────────────────────
void DrawGradientBg(void);

// ── Cards & containers ──────────────────────────────────
void DrawCard(Rectangle r, Color bg, Color border);
bool DrawCardButton(Rectangle r, const char *label, const char *icon,
                    Color accent);

// ── Header bar ──────────────────────────────────────────
bool DrawHeaderBar(const char *title); // returns true if back clicked

// ── Buttons ─────────────────────────────────────────────
bool DrawPillButton(Rectangle r, const char *text, bool active);
bool DrawTextButton(float x, float y, const char *text, int size, Color col);

// ── Charts ──────────────────────────────────────────────
void DrawMiniBar(Rectangle r, double value, double maxVal, Color col);
void DrawCandlestickChart(Rectangle r, OHLCVData *data, int count);

// ── Feedback ────────────────────────────────────────────
void DrawLoadingSpinner(Vector2 center, float radius);
void DrawErrorCard(Rectangle r, const char *msg);

// ── Helpers ─────────────────────────────────────────────
const char *FormatPrice(double price);
const char *FormatLargeNumber(double num);
const char *FormatPercent(double pct);
Color PctColor(double pct);

#endif // UI_H

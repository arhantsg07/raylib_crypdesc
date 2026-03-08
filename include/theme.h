#ifndef THEME_H
#define THEME_H

#include "raylib.h"

// ── Window ──────────────────────────────────────────────
#define WINDOW_W 1280
#define WINDOW_H 720
#define TARGET_FPS 60

// ── Background ──────────────────────────────────────────
#define BG_PRIMARY (Color){13, 17, 23, 255}   // #0D1117
#define BG_SECONDARY (Color){22, 27, 34, 255} // #161B22
#define BG_TERTIARY (Color){33, 38, 45, 255}  // #21262D

// ── Borders ─────────────────────────────────────────────
#define BORDER_DEFAULT (Color){48, 54, 61, 255} // #30363D
#define BORDER_HOVER (Color){88, 166, 255, 100} // blue glow

// ── Accents ─────────────────────────────────────────────
#define ACCENT_BLUE (Color){88, 166, 255, 255}   // #58A6FF
#define ACCENT_PURPLE (Color){124, 58, 237, 255} // #7C3AED
#define ACCENT_CYAN (Color){56, 211, 215, 255}   // #38D3D7
#define ACCENT_GOLD (Color){255, 183, 77, 255}   // #FFB74D

// ── Semantic ────────────────────────────────────────────
#define POSITIVE_GREEN (Color){63, 185, 80, 255} // #3FB950
#define NEGATIVE_RED (Color){248, 81, 73, 255}   // #F85149

// ── Text ────────────────────────────────────────────────
#define TEXT_PRIMARY (Color){240, 246, 252, 255}   // #F0F6FC
#define TEXT_SECONDARY (Color){139, 148, 158, 255} // #8B949E
#define TEXT_MUTED (Color){72, 79, 88, 255}        // #484F58

// ── Card ────────────────────────────────────────────────
#define CARD_BG BG_SECONDARY
#define CARD_BG_HOVER (Color){30, 37, 48, 255}
#define CARD_ROUNDNESS 0.06f
#define CARD_BORDER_W 1.5f

// ── Typography ──────────────────────────────────────────
#define FONT_TITLE 50
#define FONT_HEADING 32
#define FONT_SUBHEADING 26
#define FONT_BODY 24
#define FONT_SMALL 18
#define FONT_TINY 16
#define FONT_SPACING 1.0f

// ── Global custom font ─────────────────────────────────
extern Font gAppFont;

// ── Layout ──────────────────────────────────────────────
#define PAD_XL 32
#define PAD_LG 24
#define PAD_MD 16
#define PAD_SM 10
#define PAD_XS 6

// ── Header bar ──────────────────────────────────────────
#define HEADER_H 60

// ── Table (Prices) ──────────────────────────────────────
#define TABLE_ROW_H 48
#define TABLE_HEADER_H 40

// ── Scrollbar ───────────────────────────────────────────
#define SCROLLBAR_W 6
#define SCROLL_SPEED 40.0f

#endif // THEME_H

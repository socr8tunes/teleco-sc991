/*
 * Calculator application: key handling, screens and rendering. Portable (no I/O):
 * the platform feeds keys and time and presents the framebuffer.
 */
#ifndef TELECO_UI_APP_H
#define TELECO_UI_APP_H

#include <stdint.h>
#include "calc/eval.h"
#include "calc/format.h"
#include "editor/editor.h"
#include "renderer/lcd.h"
#include "ui/keys.h"

/* menu ids */
enum menu_id {
    MN_MAIN = 0, MN_HYP, MN_CONST, MN_CONV, MN_STAT, MN_MODE,
    MN_CALC, MN_NUM, MN_PROB, MN_CPLX, MN_MATRIX, MN_VECTOR, MN_STATVARS, MN_DIST, MN_EQN,
    MN_MEM, MN_ANGLE, MN_FORMAT, MN_DIGITS, MN_BASE, MN_STATTYPE, MN_COUNT
};

#define HIST_TOKS 480
#define LOGO_MS   1400u
#define AUTO_OFF_MS (10u * 60u * 1000u)

enum screen_id {
    SCR_LOGO = 0, SCR_CALC, SCR_MENU, SCR_STATEDIT, SCR_STATRES, SCR_MATVIEW, SCR_CONFIRM, SCR_OFF, SCR_GRID
};

typedef struct hist {
    tok_t buf[HIST_TOKS];      /* records: tokens then one length marker (t = len) */
    uint16_t used;
} hist_t;

#define MENU_DEPTH 4

typedef struct app {
    ctx_t    ctx;
    editor_t ed;
    hist_t   hist;

    uint8_t  screen;
    uint8_t  shift, alpha;
    uint8_t  post_eval;       /* editor shows a finished calculation           */
    uint8_t  have_res;
    uint8_t  err;             /* calc_err_t of the last evaluation              */
    uint8_t  exact;           /* 1 = natural (fractions/surds), 0 = decimal      */
    uint8_t  eng_view;
    uint8_t  dms_view;
    val_t    res;
    uint8_t  view_idx, view_cnt;   /* paging through multi-valued results       */
    uint8_t  is_pol, is_rec;
    int16_t  hist_pos;        /* -1 = not browsing                              */
    uint8_t  insert_hint;

    /* menus */
    uint8_t  menu_id[MENU_DEPTH];
    uint8_t  menu_sel[MENU_DEPTH];
    uint8_t  menu_sp;
    uint8_t  menu_prev_screen;

    /* statistics editor */
    uint8_t  st_row, st_col, st_top;
    char     st_buf[24];
    uint8_t  st_len, st_editing;
    uint8_t  sr_page;
    /* matrix viewer */
    uint8_t  mv_r, mv_c;
    /* coefficient grid (linear systems); numbers only, never saved */
    dec_t    gr_val[20];      /* row-major, 5 columns per row */
    char     gr_buf[24];
    uint8_t  gr_n, gr_row, gr_col, gr_len, gr_editing, gr_msg;
    /* confirm dialog */
    uint8_t  confirm_action;

    uint8_t  contrast;
    uint8_t  battery_low;
    uint8_t  beep;            /* set on rejected key; platform may clear/use     */
    uint32_t now_ms, last_key_ms, logo_ms;
    uint8_t  blink;
} app_t;

void app_init(app_t *a);
void app_tick(app_t *a, uint32_t now_ms);
void app_key(app_t *a, int key);
void app_render(const app_t *a, lcd_t *l);

/* screens implemented in other files */
void ui_menu_open(app_t *a, int id);
void ui_menu_key(app_t *a, int key);
void ui_menu_render(const app_t *a, lcd_t *l);
void ui_stat_key(app_t *a, int key);
void ui_stat_render(const app_t *a, lcd_t *l);
void ui_statres_key(app_t *a, int key);
void ui_statres_render(const app_t *a, lcd_t *l);
void ui_matview_key(app_t *a, int key);
void ui_matview_render(const app_t *a, lcd_t *l);
void ui_grid_open(app_t *a, int n);
void ui_grid_key(app_t *a, int key);
void ui_grid_render(const app_t *a, lcd_t *l);
void ui_confirm_key(app_t *a, int key);
void ui_confirm_render(const app_t *a, lcd_t *l);
void ui_title(lcd_t *l, const char *left, const char *right);   /* inverted title bar */
void ui_logo_render(const app_t *a, lcd_t *l);

/* helpers shared between ui files */
int  ui_res_tokens(const fmt_res_t *r, tok_t *out, int cap);
void ui_u8lcd(const char *in, char *out, size_t cap);   /* UTF-8 text -> LCD symbol codes */
int  ui_real_tokens(const ctx_t *c, const num_t *n, int exact, tok_t *out, int cap);
void ui_insert(app_t *a, tok_t k);
int  ui_set_expr_text(app_t *a, const char *text);   /* load text into the editor (tests, web simulator); 0 = ok */
void ui_insert_menu_tok(app_t *a, uint8_t t, uint8_t aux);   /* as if typed from a key */
void ui_begin_input(app_t *a);          /* leave result view before inserting */
void ui_enter_calc(app_t *a);
void ui_ac(app_t *a);
void ui_reset_memory(app_t *a);
int  ui_hist_count(const hist_t *h);

#endif

/* Key-name scripting and text extraction for the PC simulator and UI tests. */
#ifndef TELECO_PC_SIM_KEYS_H
#define TELECO_PC_SIM_KEYS_H

#include <stddef.h>
#include <stdio.h>
#include "ui/app.h"

/* Key id for a name ("SHIFT", "FRAC", "7", "+", "EXE" ...), or -1. */
int sim_key_from_name(const char *name);
/* Name of a key id. */
const char *sim_key_name(int key);
/*
 * Run a whitespace separated key script. A word made of digits and '.' is typed
 * key by key; "wait:ms" advances the clock. Returns the number of keys pressed
 * or -1 with *bad pointing at the unknown word.
 */
int sim_run_keys(app_t *a, const char *script, const char **bad);
/* The result currently shown, as ASCII text ("" if none; the error name for errors). */
size_t sim_result_text(const app_t *a, char *buf, size_t cap);
/* The expression in the editor as ASCII text. */
size_t sim_expr_text(const app_t *a, char *buf, size_t cap);
/* Frame as text: two pixel rows per character using ' ', '.', ':' , '#'. */
void sim_frame_ascii(const lcd_t *l, FILE *out);
void sim_frame_pbm(const lcd_t *l, FILE *out);

#endif

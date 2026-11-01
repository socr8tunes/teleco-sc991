/* Shell / HAL / persistence tests on the PC HAL with a fake clock. */
#include <stdio.h>
#include <string.h>
#include "ui/shell.h"
#include "ui/persist.h"
#include "hal_pc.h"
#include "hal/storage.h"
#include "sim_keys.h"

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static shell_t S;

static void tap(int key)
{
    uint8_t k = (uint8_t)key;
    hal_pc_set_keys(&k, 1);
    hal_pc_advance(20);
    shell_step(&S);
    hal_pc_set_keys(&k, 0);
    hal_pc_advance(20);
    shell_step(&S);
}

static void taps(const char *script)
{
    char w[32];
    while (*script) {
        size_t n = 0;
        while (*script == ' ') script++;
        while (script[n] && script[n] != ' ') { w[n] = script[n]; n++; }
        w[n] = 0;
        script += n;
        if (!n) break;
        if (n > 1 && w[0] >= '0' && w[0] <= '9') {
            size_t i;
            for (i = 0; i < n; i++) { char d[2] = {w[i], 0}; tap(sim_key_from_name(d)); }
        } else {
            tap(sim_key_from_name(w));
        }
    }
}

static void test_boot_and_calc(void)
{
    char r[64];
    hal_pc_storage_wipe();
    hal_pc_reset();
    CHECK(shell_init(&S) == 0, "no state to restore");
    CHECK(S.app.screen == SCR_LOGO, "starts on the logo");
    shell_step(&S);
    CHECK(hal_pc_present_count() >= 1, "logo presented");
    hal_pc_advance(1500);
    shell_step(&S);
    CHECK(S.app.screen == SCR_CALC, "logo timed out, straight to the calculation screen (screen %d)", S.app.screen);
    taps("2 + 3 EXE");
    sim_result_text(&S.app, r, sizeof r);
    CHECK(strcmp(r, "5") == 0, "2+3 = %s", r);
    CHECK(hal_pc_sleep_count() > 0, "idle loop sleeps");
}

static void test_rollover(void)
{
    uint8_t two[2];
    char r[64];
    hal_pc_storage_wipe(); hal_pc_reset();
    shell_init(&S);
    hal_pc_advance(1500); shell_step(&S);
    two[0] = (uint8_t)sim_key_from_name("7"); two[1] = (uint8_t)sim_key_from_name("8");
    hal_pc_set_keys(two, 2);
    hal_pc_advance(20); shell_step(&S); hal_pc_advance(20); shell_step(&S);   /* held: each press counts once */
    hal_pc_set_keys(two, 0);
    shell_step(&S);
    taps("EXE");
    sim_result_text(&S.app, r, sizeof r);
    CHECK(strcmp(r, "78") == 0 || strcmp(r, "87") == 0, "two keys at once typed once each: %s", r);
}

static void test_persistence(void)
{
    char r[64];
    hal_pc_storage_wipe(); hal_pc_reset();
    shell_init(&S);
    hal_pc_advance(1500); shell_step(&S);
    taps("7 STO ALPHA NEG EXE AC 1 + 1 EXE");
    taps("SHIFT AC");                           /* power off: state saved */
    CHECK(S.app.screen == SCR_OFF && !hal_pc_display_on(), "switched off");
    CHECK(hal_pc_storage_size() > 0, "state saved");
    {
        shell_t *n = &S;
        memset(n, 0, sizeof *n);
        hal_pc_reset();
        CHECK(shell_init(n) == 1, "state restored after power loss");
        CHECK(n->app.ctx.var[VAR_A].re.v.m != 0, "variable A kept");
        CHECK(ui_hist_count(&n->app.hist) == 2, "history kept (%d)", ui_hist_count(&n->app.hist));
        hal_pc_advance(1500); shell_step(n);
        taps("ALPHA NEG + 1 EXE");
        sim_result_text(&n->app, r, sizeof r);
        CHECK(strcmp(r, "8") == 0, "A + 1 = %s", r);
    }
}

static void test_corruption_and_failed_save(void)
{
    app_t *a = &S.app;
    hal_pc_storage_wipe(); hal_pc_reset();
    shell_init(&S);
    hal_pc_advance(1500); shell_step(&S);
    taps("5 STO ALPHA DMS EXE");
    CHECK(persist_save(a) == 0, "save");
    hal_pc_storage_corrupt(500);
    {
        app_t b;
        app_init(&b);
        CHECK(persist_load(&b) != 0, "CRC detects a corrupted blob");
        CHECK(b.ctx.var[VAR_B].re.v.m == 0, "app untouched after a failed load");
    }
    /* a power loss during a save keeps the previous good blob */
    hal_pc_storage_wipe();
    CHECK(persist_save(a) == 0, "save good state");
    taps("9 STO ALPHA DMS EXE");
    hal_pc_storage_fail_next_commit();
    CHECK(persist_save(a) != 0, "interrupted save reports failure");
    {
        app_t b;
        app_init(&b);
        CHECK(persist_load(&b) == 0, "previous blob still valid");
        CHECK(b.ctx.var[VAR_B].re.v.m != 0, "previous value loaded");
    }
}

/* A blob written by a different firmware version (state layout changed) must be discarded, and the
 * calculator must start with clean defaults instead of misreading the old data. */
static void test_old_version_discarded(void)
{
    app_t *a = &S.app;
    hal_pc_storage_wipe(); hal_pc_reset();
    shell_init(&S);
    hal_pc_advance(1500); shell_step(&S);
    taps("5 STO ALPHA DMS EXE");
    CHECK(persist_save(a) == 0, "save current-version blob");
    hal_pc_storage_corrupt(4);            /* the version field follows the 4-byte magic */
    {
        app_t b;
        app_init(&b);
        CHECK(persist_load(&b) != 0, "blob with a different version is rejected");
        CHECK(b.ctx.var[VAR_B].re.v.m == 0, "rejected blob leaves defaults");
    }
    hal_pc_reset();
    shell_init(&S);
    hal_pc_advance(1500); shell_step(&S);
    CHECK(S.app.ctx.var[VAR_B].re.v.m == 0, "shell boots with clean defaults after discarding old state");
}

static void test_auto_off_and_battery(void)
{
    hal_pc_storage_wipe(); hal_pc_reset();
    shell_init(&S);
    hal_pc_advance(1500); shell_step(&S);
    taps("1");
    hal_pc_advance(AUTO_OFF_MS + 1000);
    shell_step(&S);
    CHECK(S.app.screen == SCR_OFF && !hal_pc_display_on(), "auto power-off after %u ms idle", (unsigned)AUTO_OFF_MS);
    tap(K_AC);
    CHECK(S.app.screen == SCR_CALC && hal_pc_display_on(), "AC wakes the calculator");
    hal_pc_set_battery_low(1);
    shell_step(&S);
    CHECK(S.app.battery_low == 1, "battery flag reaches the UI");
}

int main(void)
{
    test_boot_and_calc();
    test_rollover();
    test_persistence();
    test_corruption_and_failed_save();
    test_old_version_discarded();
    test_auto_off_and_battery();
    printf("%d checks, %d failures\n", checks, fails);
    return fails != 0;
}

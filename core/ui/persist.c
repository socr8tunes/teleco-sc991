#include "ui/persist.h"
#include "hal/storage.h"
#include "parser/token_ids.h"
#include <string.h>

#define MAGIC   0x39435354u   /* "TSC9" */
#define VERSION 7u     /* 7: replay history stores frozen wire IDs (parser/token_ids.h) instead of enum values */

typedef struct hdr {
    uint32_t magic, version;
    uint32_t ctx_size, hist_size;
    uint8_t  exact, contrast, pad0, pad1;
} hdr_t;

static uint32_t crc32_update(uint32_t crc, const void *data, size_t len)
{
    const uint8_t *p = data;
    size_t i;
    crc = ~crc;
    for (i = 0; i < len; i++) {
        int b;
        crc ^= p[i];
        for (b = 0; b < 8; b++) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

/*
 * Replay history = records of tokens, each followed by a marker entry whose t is the record length. Translate the token
 * entries (not the markers) between enum values and frozen wire IDs in place; the caller restores / validates.
 */
static int hist_translate(hist_t *h, int to_wire)
{
    int p = h->used;
    while (p > 0) {
        int n = h->buf[p - 1].t, i;
        if (n <= 0 || n + 1 > p) return -1;
        for (i = p - 1 - n; i < p - 1; i++) {
            uint8_t v = h->buf[i].t;
            if (to_wire) {
                if (v >= T_LAST) return -1;
                h->buf[i].t = TOK_WIRE[v];
            } else {
                int k, found = -1;
                for (k = 0; k < T_LAST; k++) if (TOK_WIRE[k] == v) { found = k; break; }
                if (found < 0 || (found == 0 && v != 0)) return -1;
                h->buf[i].t = (uint8_t)found;
            }
        }
        p -= n + 1;
    }
    return 0;
}

int persist_save(app_t *a)
{
    hdr_t h;
    uint32_t crc = 0;
    memset(&h, 0, sizeof h);
    h.magic = MAGIC; h.version = VERSION;
    h.ctx_size = (uint32_t)sizeof a->ctx; h.hist_size = (uint32_t)sizeof a->hist;
    h.exact = a->exact; h.contrast = a->contrast;
    if (hist_translate(&a->hist, 1)) { hist_translate(&a->hist, 0); return -1; }
    crc = crc32_update(crc, &h, sizeof h);
    crc = crc32_update(crc, &a->ctx, sizeof a->ctx);
    crc = crc32_update(crc, &a->hist, sizeof a->hist);
    {
        int rc = 0;
        if (hal_storage_begin(sizeof h + sizeof a->ctx + sizeof a->hist + sizeof crc)) rc = -1;
        else if (hal_storage_write(&h, sizeof h) || hal_storage_write(&a->ctx, sizeof a->ctx) ||
                 hal_storage_write(&a->hist, sizeof a->hist) || hal_storage_write(&crc, sizeof crc)) rc = -1;
        else rc = hal_storage_commit();
        hist_translate(&a->hist, 0);                     /* back to enum values for the running session */
        return rc;
    }
}

/*
 * Validate first (header, then CRC over a streamed second pass) before touching the app:
 * the blob is read twice rather than buffered.
 */
int persist_load(app_t *a)
{
    hdr_t h;
    size_t len = 0;
    uint32_t crc = 0, stored = 0;
    uint8_t chunk[256];
    size_t left;

    if (hal_storage_open(&len)) return -1;
    if (len != sizeof h + sizeof a->ctx + sizeof a->hist + sizeof crc) return -1;
    if (hal_storage_read(&h, sizeof h)) return -1;
    if (h.magic != MAGIC || h.version != VERSION || h.ctx_size != sizeof a->ctx || h.hist_size != sizeof a->hist) return -1;
    crc = crc32_update(0, &h, sizeof h);
    left = sizeof a->ctx + sizeof a->hist;
    while (left) {
        size_t n = left < sizeof chunk ? left : sizeof chunk;
        if (hal_storage_read(chunk, n)) return -1;
        crc = crc32_update(crc, chunk, n);
        left -= n;
    }
    if (hal_storage_read(&stored, sizeof stored) || stored != crc) return -1;

    /* valid: second pass into the live state */
    if (hal_storage_open(&len) || hal_storage_read(&h, sizeof h)) return -1;
    if (hal_storage_read(&a->ctx, sizeof a->ctx) || hal_storage_read(&a->hist, sizeof a->hist)) return -1;
    if (hist_translate(&a->hist, 0)) { a->hist.used = 0; }      /* unknown wire ID: drop the history, keep the settings */
    a->exact = h.exact;
    a->contrast = h.contrast;
    a->post_eval = 0; a->have_res = 0; a->err = 0; a->hist_pos = -1;
    ed_clear(&a->ed);
    return 0;
}

/*
 * Teleco SC991 command-line calculator (PC test harness).
 *
 *   teleco_cli "2+3*4"            evaluate arguments
 *   echo "sin(30)" | teleco_cli    evaluate lines from stdin
 *
 * Settings lines:  :deg :rad :grad  :norm1 :norm2  :fix N  :sci N  :eng
 *                  :decimal :exact  :improper :mixed  :cmplx on|off  :raw  :reset
 * Output is ASCII unless TELECO_UTF8=1.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc_script.h"

int main(int argc, char **argv)
{
    static script_t sc;
    char line[512], out[512];
    int i;
    const char *u = getenv("TELECO_UTF8");
    script_init(&sc);
    if (u && u[0] == '1') sc.ascii = 0;
    if (argc > 1) {
        for (i = 1; i < argc; i++)
            if (script_line(&sc, argv[i], out, sizeof out)) puts(out);
        return 0;
    }
    while (fgets(line, sizeof line, stdin)) {
        size_t n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = 0;
        if (script_line(&sc, line, out, sizeof out)) puts(out);
    }
    return 0;
}

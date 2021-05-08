// Copyright (c) 2021 Pathfinders.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// * All advertising materials mentioning features or use of this software must display the following acknowledgement: This product includes software developed by Pathfinders and its contributors.
// * Neither the name of Pathfinders nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define ANSI_HEADER_SIZE 25
#define NO_ANSI_HEADER_SIZE 10

#define REMAINING_OFFSET (_countof(g_instance.write_buf) - g_instance.write_ptr)

// Format:
// 00:00:00 INFO     | @ someFunction in someFile.c       | Some message
// 00:00:00 CRITICAL | @ anotherFunction in anotherFile.c | Another message

static const char *LVL_STRS[] = {
    "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
};

static const char *LVL_COLORS[] = {
    "\x1b[32", "\x1b[36", "\x1b[33", "\x1b[31", "\x1b[101"
};

struct rm_log_instance {
    FILE *file;

    time_t raw_time;
    struct tm local_time;

    char write_buf[2048];
    size_t write_ptr;

    int enable_color;
} g_instance;

/* enables ANSI color codes in the windows command prompt */
void enable_terminal_color(void) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        g_instance.enable_color = 0;

        RML_ERR("GetStdHandle failed: %lu\n", GetLastError());
        RML_WRN("ANSI colors are not supported in this terminal\n");
        return;
    }

    unsigned long mode = 0;
    if (!GetConsoleMode(console_handle, &mode)) {
        g_instance.enable_color = 0;

        RML_ERR("GetConsoleMode failed: %lu\n", GetLastError());
        RML_WRN("ANSI colors are not supported in this terminal\n");
        return;
    }

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(console_handle, mode)) {
        g_instance.enable_color = 0;

        RML_ERR("SetConsoleMode failed: %lu\n", GetLastError());
        RML_ERR("ANSI colors are not supported in this terminal\n");
    }
}

/*
 * initializes the logger.
 * pass null to the path parameter to disable file logging
 */
int rm_init_log(const char *path, int enable_terminal_colors) {
    g_instance.enable_color = enable_terminal_colors;
    g_instance.raw_time = time(0);
    errno_t error = localtime_s(&g_instance.local_time, &g_instance.raw_time);
    if (error != 0) {
        return 1;
    }

    if (path != 0) {
        error = fopen_s(&g_instance.file, path, "a+");
        if (error != 0) {
            return 1;
        }
    } else {
        g_instance.file = 0;
    }

    enable_terminal_color();
    return 0;
}

/*
 * frees all log-related resources
 */
void rm_free_log(void) {
    if (g_instance.file != 0) fclose(g_instance.file);
}

/* writes the level to the buffer with optional ANSI colors */
void write_lvl(enum rm_log_lvl lvl, int ansi_enabled) {
    if (ansi_enabled) {
        g_instance.write_ptr += _snprintf_s(
                g_instance.write_buf, _countof(g_instance.write_buf), _countof(g_instance.write_buf),
                "\x1b[0m\x1b[%s;1m%-8s\x1b[0m ", LVL_COLORS[lvl], LVL_STRS[lvl]);
    } else {
        g_instance.write_ptr += _snprintf_s(
                g_instance.write_buf, 12, 12,
                "%-10s ", LVL_STRS[lvl]);
    }
}

/* writes the log timestamp to the buffer */
void write_time(void) {
    if (difftime(time(0), g_instance.raw_time) > 0) {
        g_instance.raw_time = time(0);
        errno_t error = localtime_s(&g_instance.local_time, &g_instance.raw_time);
        if (error != 0) {
            fprintf(stderr, "localtime_s call failed (%i)\n", error);
        }
    }

    g_instance.write_ptr += strftime(
            g_instance.write_buf + g_instance.write_ptr,
            _countof(g_instance.write_buf) - g_instance.write_ptr, "%X | ", &g_instance.local_time);
}

/* writes the location where the log function was called */
void write_src(const char* function, int line) {
    /* write function:line into separate string to pad both specifiers at once */
    static char src_buf[50];
    _snprintf_s(
        src_buf, 50, 50, "%s:%d", function, line
    );

    g_instance.write_ptr += _snprintf_s(
        g_instance.write_buf + g_instance.write_ptr, REMAINING_OFFSET, REMAINING_OFFSET,
        "@ %-20s | ", src_buf
    );
}

/* writes the actual message to the buffer */
void write_msg(const char *fmt, va_list args) {
    g_instance.write_ptr += vsnprintf_s(
            g_instance.write_buf + g_instance.write_ptr, _countof(g_instance.write_buf) - g_instance.write_ptr,
            _countof(g_instance.write_buf) - g_instance.write_ptr, fmt, args);
}

/* replaces the level in the message
 * with a version without ANSI colors to print to a file
 */
void reformat_for_file(enum rm_log_lvl lvl) {
    size_t ptr_loc = g_instance.write_ptr;
    size_t headerSize = ANSI_HEADER_SIZE;

    size_t size_without_lvl = ptr_loc - headerSize;

    if (lvl == RM_LVL_CRT) headerSize++;

    g_instance.write_ptr = 0;
    write_lvl(lvl, 0);

    memmove(g_instance.write_buf - 1 + g_instance.write_ptr, g_instance.write_buf + headerSize, size_without_lvl);
    g_instance.write_ptr = size_without_lvl + NO_ANSI_HEADER_SIZE;
}

/* logs a message with extra info to the console and optionally a log file */
void rm_log(
    enum rm_log_lvl lvl, const char *function,
    int line, const char *fmt, ...
) {
    g_instance.write_ptr = 0;

    write_lvl(lvl, g_instance.enable_color);
    write_time();
    write_src(function, line);

    va_list args;
    va_start(args, fmt);
    write_msg(fmt, args);
    va_end(args);

    if (lvl >= RM_LVL_ERR) {
        fwrite(g_instance.write_buf, g_instance.write_ptr, 1, stderr);
    } else {
        fwrite(g_instance.write_buf, g_instance.write_ptr, 1, stdout);
    }

    if (g_instance.file != 0) {
        if (g_instance.enable_color) reformat_for_file(lvl);
        fwrite(g_instance.write_buf, g_instance.write_ptr, 1, g_instance.file);
    }
}

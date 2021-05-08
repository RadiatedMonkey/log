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

#ifndef RADIATEDMONKEY_LOG_H
#define RADIATEDMONKEY_LOG_H

enum rm_log_lvl {
    RM_LVL_DBG,
    RM_LVL_INF,
    RM_LVL_WRN,
    RM_LVL_ERR,
    RM_LVL_CRT
};

#if !defined(NDEBUG) && !defined(RML_DEBUG_OVERRIDE)
#define RML_DBG(...) rm_log(RM_LVL_DBG, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define RML_DBG(...)
#endif

#define RML_INF(...) rm_log(RM_LVL_INF, __FUNCTION__, __LINE__, __VA_ARGS__)
#define RML_WRN(...) rm_log(RM_LVL_WRN, __FUNCTION__, __LINE__, __VA_ARGS__)
#define RML_ERR(...) rm_log(RM_LVL_ERR, __FUNCTION__, __LINE__, __VA_ARGS__)
#define RML_CRT(...) rm_log(RM_LVL_CRT, __FUNCTION__, __LINE__, __VA_ARGS__)

void rm_log(enum rm_log_lvl lvl, const char* function, int line, const char* fmt, ...);

int rm_init_log(const char* path, int enable_terminal_colors);
void rm_free_log(void);

#endif// RADIATEDMONKEY_LOG_H

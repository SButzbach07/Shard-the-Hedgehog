/*
 * Open Surge Engine
 * logfile.c - logfile module
 * Copyright (C) 2008-2022  Alexandre Martins <alemartf@gmail.com>
 * http://opensurge2d.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdarg.h>
#include "logfile.h"
#include "global.h"
#include "assetfs.h"
#include "util.h"


/* private stuff ;) */
static const char* LOGFILE_PATH = "logfile.txt"; /* default log file */
static FILE* logfile = NULL;


/*
 * logfile_init()
 * Initializes the logfile module.
 */
void logfile_init()
{
    const char* fullpath = assetfs_create_cache_file(LOGFILE_PATH);
    if(NULL != (logfile = fopen_utf8(fullpath, "w"))) {
        logfile_message("%s version %s", GAME_TITLE, GAME_VERSION_STRING);
    }
    else {
        logfile_message("%s version %s", GAME_TITLE, GAME_VERSION_STRING);
        logfile_message("logfile_init(): couldn't open \"%s\" for writing.", LOGFILE_PATH);
    }
}


/*
 * logfile_message()
 * Prints a message to the logfile (printf format)
 */
void logfile_message(const char* fmt, ...)
{
    /*FILE* fp = logfile ? logfile : stdout;*/
    FILE* fp = logfile;

    if(fp != NULL) {
        va_list args;

        va_start(args, fmt);
        vfprintf(fp, fmt, args);
        va_end(args);

        fputc('\n', fp);
        fflush(fp);
    }
}



/* 
 * logfile_release()
 * Releases the logfile module
 */
void logfile_release()
{
    logfile_message("tchau!");
    
    if(logfile != NULL)
        fclose(logfile);

    logfile = NULL;
}


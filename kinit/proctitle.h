/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2007 The ProFTPD Project team           //krazy:exclude=copyright
 * Copyright (c) 2007 Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PROCTITLE_H
#define PROCTITLE_H

/**
 * Initialises the program data variables to allow the
 * changing of the process title.  This _must_ be called
 * before proctitle_set, and must only be called once.
 *
 * @param argc argc, as passed to main()
 * @param argv argv, as passed to main()
 * @param envp envp, as passed to main()
 */
void proctitle_init(int argc, char *argv[], char *envp[]);

/**
 * Change the process title.  It accepts a variable number
 * of arguments (a va_list) in the manner of the printf
 * family of functions.  See the documentation for
 * printf for a description of the format string.
 */
void proctitle_set(const char *fmt, ...)
#ifdef __GNUC__
    __attribute__ (( format ( printf, 1, 2 ) ) )
#endif
;

#endif /* PROCTITLE_H */

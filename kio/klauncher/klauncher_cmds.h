/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KLAUNCHER_CMDS_H_
#define _KLAUNCHER_CMDS_H_

typedef struct
{
   long cmd;
   long arg_length;
} klauncher_header;

/* Launcher commands: */

#define LAUNCHER_EXEC	1
/*
 * LAUNCHER_EXEC
 *
 * Start a new process.
 *
 * long argc: number of arguments
 * char *args: arguments, argument 0 is the program to start.
 * int avoid_loops : ( added, read only if present ) avoid using the first
 *    path in $PATH where this process binary is found in order to avoid
 *    infinite loop by binary->kdeinit_wrapper link in $PATH
 */


#define LAUNCHER_SETENV	2
/*
 * LAUNCHER_SETENV
 *
 * Change environment of future processes launched via kdeinit.
 *
 * char *env_name;
 * char *env_value;
 */

#define LAUNCHER_DIED 3
/*
 * LAUNCHER_DIED
 *
 * Notification A child of kdeinit died.
 *
 * long pid;
 * long exit_code;
 */

#define LAUNCHER_OK 4
/*
 * LAUNCHER_OK
 *
 * Notification Last process launched ok.
 *
 * long pid;
 */

#define LAUNCHER_ERROR 5
/*
 * LAUNCHER_ERROR
 *
 * Notification Last process could not be launched.
 *
 * char *error msg (utf8)
 */

#define LAUNCHER_SHELL	6
/*
 * LAUNCHER_SHELL
 *
 * Start a new process and adjust enviroment.
 * Starts app-startup notification.
 *
 * long argc: number of arguments
 * char *args: arguments, argument 0 is the program to start.
 * char *cwd: Working directory.
 * long envc: number of environment vars
 * char *envs: environment strings.
 * int avoid_loops : ( added, read only if present ) avoid using the first
 *    path in $PATH where this process binary is found in order to avoid
 *    infinite loop by binary->kdeinit_wrapper link in $PATH
 */

#define LAUNCHER_TERMINATE_KDE 7

/*
 * LAUNCHER_TERMINATE_KDEINIT
 *
 * Suicide is painless
 */
#define LAUNCHER_TERMINATE_KDEINIT	8

#define LAUNCHER_DEBUG_WAIT		9
/*
 * LAUNCHER_DEBUG_WAIT
 *
 * Next process started will do a sleep(1000000)
 * before calling main()/kdemain()
 *
 * (Used for debugging io-slaves)
 */

#define LAUNCHER_EXT_EXEC	10
/*
 * LAUNCHER_EXT_EXEC
 *
 * Start a new process.
 * Starts app-startup notification.
 *
 * long argc: number of arguments
 * char *args: arguments, argument 0 is the program to start.
 * int avoid_loops : ( added, read only if present ) avoid using the first
 *    path in $PATH where this process binary is found in order to avoid
 *    infinite loop by binary->kdeinit_wrapper link in $PATH
 */


#define LAUNCHER_KWRAPPER	11
/*
 * LAUNCHER_KWRAPPER
 *
 * Start a new process, adjust enviroment, pass signals and output.
 * Starts app-startup notification.
 *
 * long argc: number of arguments
 * char *args: arguments, argument 0 is the program to start.
 * char *cwd: Working directory.
 * long envc: number of environment vars
 * char *envs: environment strings.
 * char *tty: tty to redirect stdout/stderr to.
 * int avoid_loops : ( added, read only if present ) avoid using the first
 *    path in $PATH where this process binary is found in order to avoid
 *    infinite loop by binary->kdeinit_wrapper link in $PATH
 */

#endif

/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

#include <kdelibs_export.h>

//??#if !defined(_INC_TIME)

#undef INCL_WINSOCK_API_PROTOTYPES
#define INCL_WINSOCK_API_PROTOTYPES 1 //for ntohl(), etc.
#include <winsock2.h> //timeval

struct itimerval {
	struct timeval it_interval; /**< reset value*/
	struct timeval it_value;    /**< current value*/
};

#include <../include/time.h>

#ifdef __cplusplus
extern "C" {
#endif

KDEWIN32_EXPORT int gettimeofday(struct timeval *__p, struct timezone *__z);
KDEWIN32_EXPORT int settimeofday(const struct timeval *, const struct timezone *); 

#ifdef __cplusplus
}
#endif

#endif //_SYS_TIME_H_

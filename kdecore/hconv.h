/*
   Copyright (c) 2003 Hans Petter Bieker <bieker@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KCALENDARSYSTEM_HCONV_H
#define KCALENDARSYSTEM_HCONV_H

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct {
  int day;
  int mon;
  int year;
} SDATE;

extern SDATE *julianToGregorian(double julian);
extern SDATE *gregorianToHijri(int,int,int);
extern SDATE *hijriToGregorian(int,int,int);
extern double gregorianToJulian(int,int,int);
extern int hijriDaysInMonth(int m, int y);

#ifdef __cplusplus
  }
#endif

#endif

/* This file is part of the KDE libraries
   Copyright (C) 2007 Christian Ehrlicher <ch.ehrlicher@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KKERNEL_WIN_H
#define KKERNEL_WIN_H

#include <qglobal.h>

#ifdef Q_OS_WIN

#include <QString>
/**
 * @short Windows-specific functions needed in kdecore
 * @author Christian Ehrlicher <ch.ehrlicher@gmx.de>
 *
 * This file prvies some windows specific functions which
 * Qt do not provide access to.
*/

/**
 * Returns the prefix because is can't be hardcoded on windows
 *
 * @return base path for all global kde directories
 */
QString getKde4Prefix();

#endif  // Q_OS_WIN
#endif  // KKERNEL_WIN_H

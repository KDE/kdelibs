/* This file is part of the KDE libraries
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>

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

#ifndef KMACROEXPANDERTEST_H
#define KMACROEXPANDERTEST_H

#include <qobject.h>

class KMacroExpanderTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void expandMacros();
	void expandMacrosShellQuote();
	void expandMacrosSubClass();
};

#endif // KMACROEXPANDERTEST_H

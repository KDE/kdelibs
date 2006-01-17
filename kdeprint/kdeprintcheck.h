/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KDEPRINTCHECK_H
#define KDEPRINTCHECK_H

#include <qstring.h>
#include <qstringlist.h>
#include <kconfig.h>
#include <kurl.h>

class KDEPRINT_EXPORT KdeprintChecker
{
public:
	static bool check(KConfig *conf, const QString& group = QString());
	static bool check(const QStringList& uris);

private:
	static bool checkURL(const KUrl&);
	static bool checkConfig(const KUrl&);
	static bool checkExec(const KUrl&);
	static bool checkService(const KUrl&);
};

#endif

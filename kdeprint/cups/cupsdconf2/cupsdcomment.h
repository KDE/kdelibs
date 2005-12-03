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

#ifndef CUPSDCOMMENT_H
#define	CUPSDCOMMENT_H

#include <qstringlist.h>
#include <q3dict.h>

class QFile;

class Comment
{
public:
        bool load(QFile* f);
        QString toolTip();
        QString comment();
	QString key();
private:
        QString comment_;
        QString example_;
	QString key_;
};

class CupsdComment
{
public:
	QString operator[] (const QString& key);
        QString comment(const QString& key);
        QString toolTip(const QString& key);

private:
	bool loadComments();

private:
	Q3Dict<Comment> comments_;
};

#endif

/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "cupsdcomment.h"

#include <qfile.h>
#include <qregexp.h>
#include <klocale.h>
#include <kstandarddirs.h>

QString Comment::comment()
{
        QString str = comment_;
        str.replace(QRegExp("<[^>]*>"), "");
        str += ("#\n" + example_);
        return str;
}

QString Comment::toolTip()
{
        QString str = comment_;
        str.replace(QRegExp("^#[\\s]*"), "").replace(QRegExp("\n#[\\s]*"), "\n");
        return i18n("Do not translate the keyword between brackets (e.g. ServerName, ServerAdmin, etc.)", str.utf8());
}

QString Comment::key()
{
	return key_;
}

bool Comment::load(QFile *f)
{
        comment_ = "";
        example_ = "";
	key_ = "";
        QString line, *current = &comment_;
        while (!f->atEnd())
        {
                f->readLine(line, 1024);
                if (line.left(2) == "$$")
                {
                        current = &example_;
                }
		else if (line.left(2) == "%%")
		{
			key_ = line.mid(2).stripWhiteSpace();
		}
                else if (line.left(2) == "@@")
                {
                        return true;
                }
                else if (line.stripWhiteSpace().isEmpty())
                {
                        ; // do nothing
                }
                else
                {
                        if (line[0] != '#') break;
                        else
                        {
                                current->append(line);
                        }
                }
        }
        return false;
}

//------------------------------------------------------------------------------------------------------------

QString CupsdComment::operator[] (const QString& key)
{
        return comment(key);
}

QString CupsdComment::comment(const QString& key)
{
        if (comments_.count() != 0 || loadComments())
	{
		Comment *comm = comments_.find(key);
		if (comm)
			return comm->comment();
	}
        return QString::null;
}

QString CupsdComment::toolTip(const QString& key)
{
        if (comments_.count() != 0 || loadComments())
	{
		Comment *comm = comments_.find(key);
		if (comm)
			return comm->toolTip();
	}
        return QString::null;
}

bool CupsdComment::loadComments()
{
        comments_.setAutoDelete(true);
        comments_.clear();
        QFile	f(locate("data", "kdeprint/cupsd.conf.template"));
	if (f.exists() && f.open(IO_ReadOnly))
	{
                Comment         *comm;
                while (!f.atEnd())
                {
                        comm = new Comment();
                        if (!comm->load(&f))
                                break;
                        else
                        {
				if (comm->key().isEmpty())
					delete comm;
				else
					comments_.insert(comm->key(), comm);
                        }
                }
	}
        return true;
}

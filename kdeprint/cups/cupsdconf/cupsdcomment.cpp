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
#include <kstddirs.h>

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
        str.replace(QRegExp("^#[\\s]*"), "").replace(QRegExp("\n#"), "\n");
        return str;
}

bool Comment::load(QFile *f)
{
        comment_ = "";
        example_ = "";
        QString line, *current = &comment_;
        while (!f->atEnd())
        {
                f->readLine(line, 1024);
                if (line.left(2) == "$$")
                {
                        current = &example_;
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

QString CupsdComment::operator[] (unsigned int index)
{
        return comment(index);
}

QString CupsdComment::comment(uint index)
{
        if (comments_.count() != 0 || loadComments())
                return comments_.at(index)->comment();
        return QString::null;
}

QString CupsdComment::toolTip(uint index)
{
        if (comments_.count() != 0 || loadComments())
                return comments_.at(index)->toolTip();
        return QString::null;
}

bool CupsdComment::loadComments()
{
        comments_.setAutoDelete(true);
        comments_.clear();
        QFile	f(locate("data", "kdeprint/cupsd.conf.template"));
        int     index(0);
	if (f.exists() && f.open(IO_ReadOnly))
	{
                Comment         *comm;
                while (index <= LOCATIONS_COMM)
                {
                        comm = new Comment();
                        if (!comm->load(&f))
                                break;
                        else
                        {
                                index++;
                                comments_.append(comm);
                        }
                }
	}
        if (index != LOCATIONS_COMM+1)
        {
                comments_.clear();
                qWarning("Problem while loading comment file %s",f.name().latin1());
                return false;
        }
        return true;
}


/*void CupsdComment::loadComments()
{
	QFile	f(locate("data", "kups/cupsd.conf.template"));
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		comm, line;
		while (!t.atEnd())
		{
			line = t.readLine();
			if (line.isEmpty() && !comm.isEmpty())
			{
				comments_.append(comm);
				comm.truncate(0);
			}
			else
			{
				comm += (line + "\n");
			}
		}
		if (!comm.isEmpty())
			comments_.append(comm);
	}
	if (comments_.count() < (LOCATIONS_COMM+1))
		debug("warning: missing comments in \"%s\"",f.name().data());
}
*/

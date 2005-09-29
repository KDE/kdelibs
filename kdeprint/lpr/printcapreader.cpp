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

#include "printcapreader.h"
#include "printcapentry.h"

#include <qfile.h>
#include <kdebug.h>

void PrintcapReader::setPrintcapFile(QFile *f)
{
    if (f->isOpen())
    {
        m_stream.setDevice(f);
        m_buffer = QString::null;
    }
}

bool PrintcapReader::nextLine(QString& line)
{
	if (m_stream.atEnd() && m_buffer.isEmpty())
		return false;
	else if (!m_buffer.isEmpty())
	{
		line = m_buffer;
		m_buffer = QString::null;
	}
	else
		line = m_stream.readLine().stripWhiteSpace();
	// strip any '\' at the end
	if (line[line.length()-1] == '\\')
		line = line.left(line.length()-1).stripWhiteSpace();
	return true;
}

void PrintcapReader::unputLine(const QString& s)
{
    m_buffer = s;
}

PrintcapEntry* PrintcapReader::nextEntry()
{
    if (!m_stream.device())
        return NULL;

    QString line, comment, name, fields, buf;
    // skip comments, keep last one
    while (1)
    {
	    if (!nextLine(line))
		    return NULL;
	    else if (line.isEmpty())
		    continue;
	    else if (line[0] == '#')
		    comment = line;
	    else
	    {
		    buf = line;
		    break;
	    }
    }

    // look for the complete entry
    while (1)
    {
	    // we found the entry if we reached the end of file or
	    // found an empty line
	    if (!nextLine(line) || line.isEmpty())
		    break;
	    // just skip comments
	    else if (line[0] == '#')
		    continue;
	    // lines starting with ':' or '|' are appended
	    else if (line[0] == ':' || line[0] == '|')
		    buf += line;
	    // otherwise it's another entry, put it back in the
	    // buffer
	    else
	    {
		    unputLine(line);
		    break;
	    }
    }

    // now parse the entry
    kdDebug() << "COMMENT: " << comment << endl;
    kdDebug() << "LINE: " << buf << endl;
    int p = buf.find(':');
    if (p == -1)
        name = buf;
    else
    {
        name = buf.left(p);
        fields = buf.right(buf.length()-p-1);
    }

    // construct the printcap entry
    if (!name.isEmpty())
    {
        PrintcapEntry   *entry = new PrintcapEntry;
        QStringList l = QStringList::split('|', name, false);
        entry->name = l[0];
        entry->comment = comment;
        // kdDebug() << "Printer: " << entry->name << endl;
        // kdDebug() << "Aliases:" << endl;
        for (uint i=1; i<l.count(); i++)
        {
            entry->aliases << l[i];
            // kdDebug() << "  " << l[i] << endl;
        }
        if (!fields.isEmpty())
        {
            // kdDebug() << "Fields:" << endl;
            // kdDebug() << "(" << fields << ")" << endl;
            l = QStringList::split(':', fields, false);
            for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
            {
                Field f;
                int p = (*it).find('=');
                if (p == -1)
                {
                    p = (*it).find('#');
                    if (p == -1)
                    {
                        f.type = Field::Boolean;
			p = (*it).find('@');
			if (p == -1)
			{
				f.name = (*it);
				f.value = "1";
			}
			else
			{
				f.name = (*it).left(p);
				f.value = "0";
			}
                    }
                    else
                    {
                        f.type = Field::Integer;
                        f.name = (*it).left(p);
                        f.value = (*it).mid(p+1);
                    }
                }
                else
                {
                    f.type = Field::String;
                    f.name = (*it).left(p);
                    f.value = (*it).mid(p+1);
                    if (f.value.startsWith("\""))
                        f.value = f.value.mid(1, f.value.length()-2);
                }
                entry->fields[f.name] = f;
            }
        }
        // kdDebug() << endl;
        return entry;
    }
    return NULL;
}

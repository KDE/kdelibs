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

void PrintcapReader::skipBlankLines()
{
    QString line;
    while (!m_stream.atEnd() && (line=m_stream.readLine().stripWhiteSpace()).isEmpty()) ;
    if (!line.isEmpty())
        unputLine(line);
}

QString PrintcapReader::nextLine()
{
    QString line;
    if (!m_buffer.isEmpty())
    {
        line = m_buffer;
        m_buffer = QString::null;
    }
    else
    {
        if (m_stream.atEnd())
            return QString::null;
        else
            line = m_stream.readLine().stripWhiteSpace();
    }
    if (line.right(1) == "\\")
    {
        line = line.left(line.length()-1).stripWhiteSpace() + nextLine();
    }
    return line;
}

void PrintcapReader::unputLine(const QString& s)
{
    m_buffer = s;
}

QString PrintcapReader::lprngLine()
{
    QString line = nextLine();
    while (1)
    {
        QString nline = nextLine();
        if (nline.isEmpty())
            break;
        QChar   c = nline[0];
        if (c == ':' || c == '|')
            line.append(nline);
        else if (c != '#')
        {
            unputLine(nline);
            break;
        }
    }
    return line;
}

PrintcapEntry* PrintcapReader::nextEntry()
{
    if (!m_stream.device())
        return NULL;

    QString line, comment, name, fields;
    // skip comments, keep last one
    skipBlankLines();
    do
    {
        line = nextLine();
        if (line.isEmpty())
            return NULL;
        if (line.startsWith("#"))
            comment = line;
        else
        {
            unputLine(line);
            break;
        }
    } while (1);
    // get the rest of the entry
    QString buf = lprngLine();
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
                        f.name = (*it);
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

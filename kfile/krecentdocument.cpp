/* -*- c++ -*-
 * Copyright (C)2000 Daniel M. Duley <mosfet@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <krecentdocument.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kurl.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <sys/types.h>
#include <utime.h>

void KRecentDocument::add(const QString &openStr, bool isUrl)
{
    qWarning("KRecentDocument::add for %s", openStr.latin1());
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup(QString::fromLatin1("RecentDocuments"));
    bool useRecent = config->readBoolEntry(QString::fromLatin1("UseRecent"), true);
    int maxEntries = config->readNumEntry(QString::fromLatin1("MaxEntries"), 10);

    config->setGroup(oldGrp);
    if(!useRecent)
        return;

    // need to change this path, not sure where
    QString path = locateLocal("data", QString::fromLatin1("RecentDocuments/"));
                 
    QString dStr;
    QFileInfo fi(openStr);
    if(!isUrl)
        dStr = path + fi.fileName();
    else{
        KURL url(openStr);
        dStr = path + url.fileName();
    }
    
    QString ddesktop = dStr + QString::fromLatin1(".desktop");

    int i;
    // check for duplicates
    if(QFile::exists(ddesktop)){
        // see if it points to the same file and application
        KSimpleConfig tmp(ddesktop);
        tmp.setDesktopGroup();
        if(tmp.readEntry(QString::fromLatin1("Exec"), QString::fromLatin1(""))
	   == QString::fromLatin1(kapp->argv()[0]) + 
	   QString::fromLatin1(" ") + openStr) 
	{
            qWarning("Touching");
            utime(ddesktop.latin1(), NULL);
            return;
        }
        // if not append a (num) to it
        for(i=2; i < maxEntries+1 && QFile::exists(dStr + QString::fromLatin1(".desktop")); ++i)
            dStr.sprintf("%s[%d]", dStr.latin1(), i);
    }
    dStr = dStr + QString::fromLatin1(".desktop");

    QDir dir(path);
    // check for max entries, delete oldest files if exceeded
    QStringList list = dir.entryList(QDir::Files, QDir::Time | QDir::Reversed);
    i = list.count();
    if(i > maxEntries-1){
        QStringList::Iterator it;
        it = list.begin();
        while(i > maxEntries-1){
            QFile::remove(dir.absPath() + QString::fromLatin1("/") + (*it));
            --i, ++it;
        }
    }

    // create the applnk
    QFile dFile(dStr);
    dFile.open(IO_ReadWrite);
    QTextStream stream(&dFile);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "URL=" << openStr << "\n";
    stream << "Exec=" << kapp->argv()[0] << " " << openStr << "\n";
    if(!isUrl)
        stream << "Name=" << fi.fileName() << "\n";
    else
        stream << "Name=" << openStr << "\n";
    stream << "Icon=document"  << "\n";
    dFile.close();
}

void KRecentDocument::clear()
{
  // we remove all we can access. There shouldn't be any more than in $HOME
  QStringList list = KGlobal::dirs()->findAllResources("data", QString::fromLatin1("RecentDocuments/*"));
  QDir dir;
  for(QStringList::Iterator it = list.begin(); it != list.end() ; ++it)
    dir.remove(*it);
}

int KRecentDocument::maximumItems()
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver sa(config, QString::fromLatin1("RecentDocuments"));
    return config->readNumEntry(QString::fromLatin1("MaxEntries"), 10);
}
    


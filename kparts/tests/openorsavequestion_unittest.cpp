/*
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kparts/browseropenorsavequestion.h>
#include <qtest_kde.h>

#include <QWidget>

using namespace KParts;

class PartTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAutoEmbed()
    {
        // This one should get the fast path, no dialog should show up.
        BrowserOpenOrSaveQuestion questionEmbedHtml(0, KUrl("http://www.example.com/"),
                                                    QString::fromLatin1("text/html"));
        QCOMPARE(questionEmbedHtml.askEmbedOrSave(), BrowserOpenOrSaveQuestion::Embed);

    }
    void testDontAskAgain()
    {
        KSharedConfig::Ptr cfg = KSharedConfig::openConfig("filetypesrc", KConfig::NoGlobals);
        cfg->group("Notification Messages").writeEntry("askSave" "text/plain", "false");
        BrowserOpenOrSaveQuestion question(0, KUrl("http://www.example.com/"),
                                           QString::fromLatin1("text/plain"));
        QCOMPARE((int)question.askOpenOrSave(), (int)BrowserOpenOrSaveQuestion::Open);
        cfg->group("Notification Messages").writeEntry("askSave" "text/plain", "true");
        QCOMPARE((int)question.askOpenOrSave(), (int)BrowserOpenOrSaveQuestion::Save);
    }
};

QTEST_KDEMAIN( PartTest, GUI )

#include "openorsavequestion_unittest.moc"

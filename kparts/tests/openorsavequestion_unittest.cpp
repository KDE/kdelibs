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

#include <kmimetypetrader.h>
#include <kparts/browseropenorsavequestion.h>
#include <qtest_kde.h>

#include <kdialog.h>
#include <kpushbutton.h>
#include <QMenu>
#include <QWidget>

using namespace KParts;

// SYNC - copied from browseropenorsavequestion.cpp
static const KDialog::ButtonCode Save = KDialog::Yes;
static const KDialog::ButtonCode OpenDefault = KDialog::User2;
static const KDialog::ButtonCode OpenWith = KDialog::User1;
static const KDialog::ButtonCode Cancel = KDialog::Cancel;

Q_DECLARE_METATYPE(KDialog*)

class OpenOrSaveTest : public QObject
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
        cfg->group("Notification Messages").deleteEntry("askSave" "text/plain");
    }

    void testAllChoices_data()
    {
        qRegisterMetaType<KDialog*>("KDialog*");
        
        QTest::addColumn<QString>("mimetype");
        QTest::addColumn<int>("button");
        QTest::addColumn<int>("expectedResult");
        QTest::addColumn<bool>("expectedService");

        // For this test, we rely on the fact that there is at least one app associated with application/zip
        // and one (or more) with text/plain.
        QVERIFY(KMimeTypeTrader::self()->query("application/zip", "Application").count() > 0);
        QVERIFY(KMimeTypeTrader::self()->query("text/plain", "Application").count() > 0);
        
        QTest::newRow("(zip) cancel") << "application/zip" << (int)Cancel << (int)BrowserOpenOrSaveQuestion::Cancel << false;
        QTest::newRow("(zip) open default app") << "application/zip" << (int)OpenDefault << (int)BrowserOpenOrSaveQuestion::Open << true;
        QTest::newRow("(zip) open with...") << "application/zip" << (int)OpenWith << (int)BrowserOpenOrSaveQuestion::Open << false;
        QTest::newRow("(zip) save") << "application/zip" << (int)Save << (int)BrowserOpenOrSaveQuestion::Save << false;

        QTest::newRow("(text) cancel") << "text/plain" << (int)Cancel << (int)BrowserOpenOrSaveQuestion::Cancel << false;
        QTest::newRow("(text) open default app") << "text/plain" << (int)OpenDefault << (int)BrowserOpenOrSaveQuestion::Open << true;
        QTest::newRow("(text) open with...") << "text/plain" << (int)OpenWith << (int)BrowserOpenOrSaveQuestion::Open << false;
        QTest::newRow("(text) save") << "text/plain" << (int)Save << (int)BrowserOpenOrSaveQuestion::Save << false;

        // For this test, we rely on the fact that there are no apps associated with application/x-zerosize
        QCOMPARE(KMimeTypeTrader::self()->query("application/x-zerosize", "Application").count(), 0);
        
        QTest::newRow("(zero) cancel") << "application/x-zerosize" << (int)Cancel << (int)BrowserOpenOrSaveQuestion::Cancel << false;
        QTest::newRow("(zero) open with...") << "application/x-zerosize" << (int)OpenDefault /*Yes, not OpenWith*/ << (int)BrowserOpenOrSaveQuestion::Open << false;
        QTest::newRow("(zero) save") << "application/x-zerosize" << (int)Save << (int)BrowserOpenOrSaveQuestion::Save << false;
    }
    
    void testAllChoices()
    {
        QFETCH(QString, mimetype);
        QFETCH(int, button);
        QFETCH(int, expectedResult);
        QFETCH(bool, expectedService);
        
        QWidget parent;
        BrowserOpenOrSaveQuestion questionEmbedZip(&parent, KUrl("http://www.example.com/"), mimetype);
        questionEmbedZip.setFeatures(BrowserOpenOrSaveQuestion::ServiceSelection);
        KDialog* theDialog = qFindChild<KDialog *>(&parent);
        QVERIFY(theDialog);
        //QMetaObject::invokeMethod(theDialog, "slotButtonClicked", Qt::QueuedConnection, Q_ARG(int, button));
        QMetaObject::invokeMethod(this, "clickButton", Qt::QueuedConnection, Q_ARG(KDialog*, theDialog),
                                  Q_ARG(int, button));
        QCOMPARE((int)questionEmbedZip.askOpenOrSave(), expectedResult);
        QCOMPARE(!questionEmbedZip.selectedService().isNull(), expectedService);
    }
    
protected Q_SLOTS: // our own slots, not tests
    void clickButton(KDialog* dialog, int buttonId)
    {
        QPushButton* button = dialog->button(KDialog::ButtonCode(buttonId));
        Q_ASSERT(button);
        Q_ASSERT(!button->isHidden());
        if (button->menu()) {
            Q_ASSERT(buttonId == OpenWith); // only this one has a menu
            button->menu()->actions().last()->trigger();
        } else {
            // Can't do that, it's protected: dialog->slotButtonClicked(buttonId);
            QMetaObject::invokeMethod(dialog, "slotButtonClicked", Q_ARG(int, buttonId));
        }
    }
};

QTEST_KDEMAIN( OpenOrSaveTest, GUI )

#include "openorsavequestion_unittest.moc"

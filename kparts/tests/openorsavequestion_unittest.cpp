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
#include <qtest_widgets.h>

#include <kconfiggroup.h>
#include <QDebug>

#include <QDialog>
#include <QMenu>
#include <QPushButton>
#include <QWidget>

using namespace KParts;

// SYNC - keep this in sync with browseropenorsavequestion.cpp
static const QString Save = "saveButton";
static const QString OpenDefault = "openDefaultButton";
static const QString OpenWith = "openWithButton";
static const QString Cancel = "cancelButton";

Q_DECLARE_METATYPE(QDialog*)

class OpenOrSaveTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAutoEmbed()
    {
        // This one should get the fast path, no dialog should show up.
        BrowserOpenOrSaveQuestion questionEmbedHtml(0, QUrl("http://www.example.com/"),
                                                    QString::fromLatin1("text/html"));
        QCOMPARE(questionEmbedHtml.askEmbedOrSave(), BrowserOpenOrSaveQuestion::Embed);

    }
    void testDontAskAgain()
    {
        KSharedConfig::Ptr cfg = KSharedConfig::openConfig("filetypesrc", KConfig::NoGlobals);
        cfg->group("Notification Messages").writeEntry("askSave" "text/plain", "false");
        BrowserOpenOrSaveQuestion question(0, QUrl("http://www.example.com/"),
                                           QString::fromLatin1("text/plain"));
        QCOMPARE((int)question.askOpenOrSave(), (int)BrowserOpenOrSaveQuestion::Open);
        cfg->group("Notification Messages").writeEntry("askSave" "text/plain", "true");
        QCOMPARE((int)question.askOpenOrSave(), (int)BrowserOpenOrSaveQuestion::Save);
        cfg->group("Notification Messages").deleteEntry("askSave" "text/plain");
    }

    void testAllChoices_data()
    {
        qRegisterMetaType<QDialog*>("QDialog*");

        QTest::addColumn<QString>("mimetype");
        QTest::addColumn<QString>("buttonName");
        QTest::addColumn<int>("expectedResult");
        QTest::addColumn<bool>("expectedService");

        // For this test, we rely on the fact that:
        // 1. there is at least one app associated with application/zip,
        // 2. there is at least one app associated with text/plain, and
        // 3. there are no apps associated with application/x-zerosize.

        if(KMimeTypeTrader::self()->query("application/zip", "Application").count() > 0) {
            QTest::newRow("(zip) cancel") << "application/zip" << Cancel << (int)BrowserOpenOrSaveQuestion::Cancel << false;
            QTest::newRow("(zip) open default app") << "application/zip" << OpenDefault << (int)BrowserOpenOrSaveQuestion::Open << true;
            QTest::newRow("(zip) open with...") << "application/zip" << OpenWith << (int)BrowserOpenOrSaveQuestion::Open << false;
            QTest::newRow("(zip) save") << "application/zip" << Save << (int)BrowserOpenOrSaveQuestion::Save << false;
        }
        else {
            qWarning() << "This test relies on the fact that there is at least one app associated with appliation/zip.";
        }

        if(KMimeTypeTrader::self()->query("text/plain", "Application").count() > 0) {
            QTest::newRow("(text) cancel") << "text/plain" << Cancel << (int)BrowserOpenOrSaveQuestion::Cancel << false;
            QTest::newRow("(text) open default app") << "text/plain" << OpenDefault << (int)BrowserOpenOrSaveQuestion::Open << true;
            QTest::newRow("(text) open with...") << "text/plain" << OpenWith << (int)BrowserOpenOrSaveQuestion::Open << false;
            QTest::newRow("(text) save") << "text/plain" << Save << (int)BrowserOpenOrSaveQuestion::Save << false;
        }
        else {
            qWarning() << "This test relies on the fact that there is at least one app associated with text/plain.";
        }

        if(KMimeTypeTrader::self()->query("application/x-zerosize", "Application").count() == 0) {
            QTest::newRow("(zero) cancel") << "application/x-zerosize" << Cancel << (int)BrowserOpenOrSaveQuestion::Cancel << false;
            QTest::newRow("(zero) open with...") << "application/x-zerosize" << OpenDefault /*Yes, not OpenWith*/ << (int)BrowserOpenOrSaveQuestion::Open << false;
            QTest::newRow("(zero) save") << "application/x-zerosize" << Save << (int)BrowserOpenOrSaveQuestion::Save << false;
        }
        else {
            qWarning() << "This test relies on the fact that there are no apps associated with application/x-zerosize.";
        }


    }

    void testAllChoices()
    {
        QFETCH(QString, mimetype);
        QFETCH(QString, buttonName);
        QFETCH(int, expectedResult);
        QFETCH(bool, expectedService);

        QWidget parent;
        BrowserOpenOrSaveQuestion questionEmbedZip(&parent, QUrl("http://www.example.com/"), mimetype);
        questionEmbedZip.setFeatures(BrowserOpenOrSaveQuestion::ServiceSelection);
        QDialog* theDialog = parent.findChild<QDialog *>();
        QVERIFY(theDialog);
        //QMetaObject::invokeMethod(theDialog, "slotButtonClicked", Qt::QueuedConnection, Q_ARG(int, button));
        QMetaObject::invokeMethod(this, "clickButton", Qt::QueuedConnection, Q_ARG(QDialog*, theDialog),
                                  Q_ARG(QString, buttonName));
        QCOMPARE((int)questionEmbedZip.askOpenOrSave(), expectedResult);
        QCOMPARE((bool)questionEmbedZip.selectedService(), expectedService);
    }

protected Q_SLOTS: // our own slots, not tests
    void clickButton(QDialog* dialog, const QString &buttonName)
    {
        QPushButton *button = dialog->findChild<QPushButton *>(buttonName);
        Q_ASSERT(button);
        Q_ASSERT(!button->isHidden());
        if (button->menu()) {
            Q_ASSERT(buttonName == OpenWith); // only this one has a menu
            button->menu()->actions().last()->trigger();
        } else {
            button->click();
        }
    }
};

QTEST_MAIN(OpenOrSaveTest)

#include "openorsavequestion_unittest.moc"

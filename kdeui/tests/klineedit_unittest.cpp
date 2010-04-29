/* This file is part of the KDE libraries

    Copyright (c) 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QClipboard>
#include <qtest_kde.h>
#include <qtestevent.h>
#include <klineedit.h>

class KLineEdit_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPassword()
    {
        KLineEdit w;
        w.setPasswordMode(true);
        QTest::keyClick(&w, Qt::Key_1);
        QTest::keyClick(&w, Qt::Key_2);
        QTest::keyClick(&w, Qt::Key_3);
        QCOMPARE(w.text(), QString("123"));
    }

    void testReturnPressed()
    {
        KLineEdit w;
        w.setText("Hello world");
        QSignalSpy qReturnPressedSpy(&w, SIGNAL(returnPressed()));
        QSignalSpy kReturnPressedSpy(&w, SIGNAL(returnPressed(QString)));
        QTest::keyClick(&w, Qt::Key_Return);
        QCOMPARE(qReturnPressedSpy.count(), 1);
        QCOMPARE(kReturnPressedSpy.count(), 1);
        QCOMPARE(kReturnPressedSpy[0][0].toString(), QString("Hello world"));
    }

    void testTextEditedSignals()
    {
        KLineEdit w;

        // setText emits textChanged and userTextChanged, but not textEdited
        QSignalSpy textChangedSpy(&w, SIGNAL(textChanged(QString)));
        QSignalSpy textEditedSpy(&w, SIGNAL(textEdited(QString)));
        QSignalSpy userTextChangedSpy(&w, SIGNAL(userTextChanged(QString)));
        w.setText("Hello worl");
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 0);
        userTextChangedSpy.clear();
        textChangedSpy.clear();
        textEditedSpy.clear();

        // typing emits all three signals
        QTest::keyClick(&w, Qt::Key_D);
        QCOMPARE(w.text(), QString::fromLatin1("Hello world"));
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 1);
        QCOMPARE(textEditedSpy[0][0].toString(), w.text());
        w.setText("K");
        userTextChangedSpy.clear();
        textChangedSpy.clear();
        textEditedSpy.clear();

        // the suggestion from auto completion emits textChanged but not userTextChanged nor textEdited
        w.setCompletionMode(KGlobalSettings::CompletionAuto);
        KCompletion completion;
	completion.setSoundsEnabled(false);
        QStringList items;
        items << "KDE is cool" << "KDE is really cool";
        completion.setItems(items);
        w.setCompletionObject(&completion);

        w.doCompletion(w.text());
        QCOMPARE(w.text(), items.at(0));

        QCOMPARE(userTextChangedSpy.count(), 0);
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 0);
        userTextChangedSpy.clear();
        textChangedSpy.clear();
        textEditedSpy.clear();

        // accepting the completion suggestion emits userTextChanged
        // TODO: should emit textChanged and textEdited too?
        QTest::keyClick(&w, Qt::Key_Right);
        QCOMPARE(w.text(), items.at(0));

        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textChangedSpy.count(), 0);
        QCOMPARE(textEditedSpy.count(), 0);
    }

    void testPaste()
    {
        const QString origText = QApplication::clipboard()->text();
        const QString pastedText = "Test paste from klineedit_unittest";
        QApplication::clipboard()->setText(pastedText);
        KLineEdit w;
        w.setText("Hello world");
        w.selectAll();
        QTest::keyClick(&w, Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(w.text(), pastedText);
        QApplication::clipboard()->setText(origText);
    }

};

QTEST_KDEMAIN(KLineEdit_UnitTest, GUI)

#include "klineedit_unittest.moc"

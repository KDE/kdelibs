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

#include <kdebug.h>
#include <QClipboard>
#include <qtest_kde.h>
#include <qtestevent.h>
#include <klineedit.h>
#include <kcompletionbox.h>

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
        QVERIFY(!w.isModified());

        // setText emits textChanged and userTextChanged, but not textEdited
        QSignalSpy textChangedSpy(&w, SIGNAL(textChanged(QString)));
        QSignalSpy textEditedSpy(&w, SIGNAL(textEdited(QString)));
#ifndef KDE_NO_DEPRECATED
        QSignalSpy userTextChangedSpy(&w, SIGNAL(userTextChanged(QString)));
#endif
        w.setText("Hello worl");
#ifndef KDE_NO_DEPRECATED
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 0);
        QVERIFY(!w.isModified());
#ifndef KDE_NO_DEPRECATED
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        // typing emits all three signals
        QTest::keyClick(&w, Qt::Key_D);
        QCOMPARE(w.text(), QString::fromLatin1("Hello world"));
#ifndef KDE_NO_DEPRECATED
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 1);
        QCOMPARE(textEditedSpy[0][0].toString(), w.text());
        QVERIFY(w.isModified());

        w.setText("K"); // prepare for next test
#ifndef KDE_NO_DEPRECATED
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();
        QVERIFY(!w.isModified());

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

#ifndef KDE_NO_DEPRECATED
        QCOMPARE(userTextChangedSpy.count(), 0);
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 0);
        QVERIFY(!w.isModified());
#ifndef KDE_NO_DEPRECATED
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        // accepting the completion suggestion now emits all three signals too
        QTest::keyClick(&w, Qt::Key_Right);
        QCOMPARE(w.text(), items.at(0));

#ifndef KDE_NO_DEPRECATED
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 1);
        QCOMPARE(textEditedSpy[0][0].toString(), w.text());
        QVERIFY(w.isModified());
#ifndef KDE_NO_DEPRECATED
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        // Now with popup completion
        w.setCompletionMode(KGlobalSettings::CompletionPopup);
        w.setText("KDE");
        QVERIFY(!w.isModified());
#ifndef KDE_NO_DEPRECATED
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();
        w.doCompletion(w.text()); // popup appears
        QCOMPARE(w.text(), QString::fromLatin1("KDE"));
#ifndef KDE_NO_DEPRECATED
        QCOMPARE(textChangedSpy.count() + userTextChangedSpy.count() + textEditedSpy.count(), 0);
#else
        QCOMPARE(textChangedSpy.count() + textEditedSpy.count(), 0);
#endif
        w.completionBox()->down(); // select 1st item
        QCOMPARE(w.text(), items.at(0));
        QVERIFY(w.isModified());
        w.completionBox()->down(); // select 2nd item
        QCOMPARE(w.text(), items.at(1));

        // Selecting an item in the popup completion changes the lineedit text
        // and emits textChanged and userTextChanged, but not textEdited.
#ifndef KDE_NO_DEPRECATED
        QCOMPARE(userTextChangedSpy.count(), 2);
#endif
        QCOMPARE(textChangedSpy.count(), 2);
        QCOMPARE(textEditedSpy.count(), 0);
#ifndef KDE_NO_DEPRECATED
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        QTest::keyClick(&w, Qt::Key_Enter); // activate
        QVERIFY(!w.completionBox()->isVisible());
        QCOMPARE(w.text(), items.at(1));
        QVERIFY(w.isModified());
        // Nothing else happens, the text was already set in the lineedit
        QCOMPARE(textChangedSpy.count(), 0);
        QCOMPARE(textEditedSpy.count(), 0);
#ifndef KDE_NO_DEPRECATED
        QCOMPARE(userTextChangedSpy.count(), 0);
#endif

        // Now when using the mouse in the popup completion
        w.setText("KDE");
        w.doCompletion(w.text()); // popup appears
        QCOMPARE(w.text(), QString::fromLatin1("KDE"));
        // Selecting an item in the popup completion changes the lineedit text and emits all 3 signals
        const QRect rect = w.completionBox()->visualRect(w.completionBox()->model()->index(1, 0));
        QSignalSpy activatedSpy(w.completionBox(), SIGNAL(activated(QString)));
        QTest::mouseClick(w.completionBox()->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());
        QCOMPARE(activatedSpy.count(), 1);
        QCOMPARE(w.text(), items.at(1));
        QVERIFY(w.isModified());
    }

    void testCompletionBox()
    {
        KLineEdit w;
        w.setText("/");
        w.setCompletionMode(KGlobalSettings::CompletionPopup);
        KCompletion completion;
        completion.setSoundsEnabled(false);
        w.setCompletionObject(&completion);
        QStringList items;
        items << "/home/" << "/hold/" << "/hole/";
        completion.setItems(items);
        QTest::keyClick(&w, 'h');
        QCOMPARE(w.text(), QString::fromLatin1("/h"));
        QCOMPARE(w.completionBox()->currentRow(), -1);
        QCOMPARE(w.completionBox()->items(), items);
        QTest::keyClick(&w, 'o');
        QCOMPARE(w.text(), QString::fromLatin1("/ho"));
        QCOMPARE(w.completionBox()->currentRow(), -1);
        w.completionBox()->down(); // select 1st item
        QCOMPARE(w.text(), items.at(0));
        w.completionBox()->down(); // select 2nd item
        QCOMPARE(w.text(), items.at(1));
        w.completionBox()->up();   // select 1st item again
        QCOMPARE(w.text(), items.at(0));
        w.completionBox()->up();   // select last item
        QCOMPARE(w.text(), items.at(2));
        w.completionBox()->down(); // select 1st item again
        QCOMPARE(w.text(), items.at(0));

        QStringList newItems;
        newItems << "/home/kde";
        completion.setItems(newItems);
        QTest::keyClick(&w, 'k');
        QCOMPARE(w.text(), QString("/home/k"));
        //QCOMPARE(w.completionBox()->currentRow(), -1); // #247552
        w.completionBox()->down(); // select the item
        QCOMPARE(w.completionBox()->items(), newItems);
        QCOMPARE(w.text(), newItems.at(0));
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

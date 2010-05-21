/* This file is part of the KDE libraries

    Copyright (c) 2010 Romain Perier <mrpouet@gentoo.org>

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
#include <QCheckBox>
#include <qtest_kde.h>
#include <kdialog.h>
#include <kpushbutton.h>
#include <QWeakPointer>

Q_DECLARE_METATYPE(KDialog::ButtonCode)

class KDialog_UnitTest : public QObject
{
    Q_OBJECT
public:
    KDialog_UnitTest()
    {
        qRegisterMetaType<KDialog::ButtonCode>("KDialog::ButtonCode");
    }

private: // helper methods
    void checkOtherButtonsAreNotDefault(KDialog & dialog, KDialog::ButtonCode id)
    {
        KDialog::ButtonCode codes[] = { KDialog::Ok, KDialog::Apply, KDialog::Cancel,
            KDialog::No, KDialog::Yes };
        for (int i = 0; i < 5; i++)
	{
            if (codes[i] == id)
	        continue;
            QVERIFY(!dialog.button(codes[i])->isDefault());
	}
    }

    void checkSetDefaultButton(KDialog & dialog, KDialog::ButtonCode id)
    {
        dialog.setDefaultButton(id);
        QCOMPARE(dialog.defaultButton(), id);
        QVERIFY(dialog.button(id)->isDefault());
        checkOtherButtonsAreNotDefault(dialog, id);
    }

private Q_SLOTS:
    // Test if buttons are correctly stored
    // in the KDialog, then try to get back them.
    // (only common buttons are tested)
    void testDefaultButton()
    {
        KDialog dialog;
        dialog.setButtons(KDialog::Ok | KDialog::Apply
            | KDialog::Cancel | KDialog::No | KDialog::Yes);
        checkSetDefaultButton(dialog, KDialog::Ok);
        checkSetDefaultButton(dialog, KDialog::Apply);
        checkSetDefaultButton(dialog, KDialog::Cancel);
        checkSetDefaultButton(dialog, KDialog::No);
        dialog.setDefaultButton(KDialog::NoDefault);
        checkOtherButtonsAreNotDefault(dialog, KDialog::NoDefault);
        checkSetDefaultButton(dialog, KDialog::Yes);
        dialog.setDefaultButton(KDialog::None); // #148969
        checkOtherButtonsAreNotDefault(dialog, KDialog::None);
    }

    // Test what happens with giving focus to widgets before the window is shown
    // This is mostly Qt experimentation, unrelated to KDialog's own code
    void testFocus()
    {
        KDialog dialog;
        dialog.setButtons(KDialog::Ok | KDialog::Cancel);
        QCheckBox* checkBox = new QCheckBox("Hello world !", &dialog);
        QPushButton* okButton = dialog.button(KDialog::Ok);
        okButton->setFocus();
        QVERIFY(!okButton->hasFocus()); // confusing, heh?
        QCOMPARE(dialog.focusWidget(), static_cast<QWidget*>(okButton));
        checkBox->setFocus();
        QVERIFY(!checkBox->hasFocus()); // confusing, heh?
        QCOMPARE(dialog.focusWidget(), static_cast<QWidget*>(checkBox));
    }

    // Ensure that only the defaultButton() receives the keyEvent
    // (it should get the focus)
    void testKeyPressEvents()
    {
        KDialog dialog;
        QSignalSpy qCancelClickedSpy(&dialog, SIGNAL(cancelClicked()));
        QSignalSpy qOkClickedSpy(&dialog, SIGNAL(okClicked()));
        dialog.setButtons(KDialog::Ok | KDialog::Cancel);
        dialog.setDefaultButton(KDialog::Cancel);
        dialog.show();
        // Necessary after show(), otherwise dialog.focusWidget() returns NULL
        QApplication::setActiveWindow(&dialog);
        QVERIFY(dialog.focusWidget());
        // Graphically always the focused widget receives the keyEvent
        // (otherwises it does not make sense)
        QTest::keyClick(dialog.focusWidget(), Qt::Key_Return);
        QCOMPARE(qCancelClickedSpy.count(), 1);
        QCOMPARE(qOkClickedSpy.count(), 0);
    }

    void testCheckBoxKeepsFocus()
    {
        KDialog dialog;
        QCheckBox checkBox("Hello world !", &dialog);
        QSignalSpy qCancelClickedSpy(&dialog, SIGNAL(cancelClicked()));
        dialog.setButtons(KDialog::Ok | KDialog::Cancel);
        checkBox.setFocus();
        dialog.setMainWidget(&checkBox);
        dialog.setDefaultButton(KDialog::Cancel);
        dialog.show();
        QApplication::setActiveWindow(&dialog);
        QVERIFY(checkBox.hasFocus());
        QVERIFY(!dialog.button(KDialog::Cancel)->hasFocus());
        QCOMPARE(dialog.focusWidget(), static_cast<QWidget*>(&checkBox));
        QTest::keyClick(dialog.focusWidget(), Qt::Key_Return);
        QCOMPARE(qCancelClickedSpy.count(), 1);
    }

    // Test if buttons labels are correctly handled
    // assuming that testButtonDefault() passed
    void testButtonText()
    {
        KDialog dialog;
        KDialog::ButtonCode id = KDialog::Ok;
        QString text = "it's okay !";
        dialog.setButtons(id);
        dialog.setButtonText(id, text);
        QCOMPARE(dialog.buttonText(id), text);
        QCOMPARE(dialog.button(id)->text(), text);
    }

    void testButtonToolTip()
    {
        KDialog dialog;
        KDialog::ButtonCode id = KDialog::Ok;
        QString tooltip = "This is okay button";
        dialog.setButtons(id);
        dialog.setButtonToolTip(id, tooltip);
        QCOMPARE(dialog.buttonToolTip(id), tooltip);
        QCOMPARE(dialog.button(id)->toolTip(), tooltip);
    }

    void testButtonWhatsThis()
    {
        KDialog dialog;
        KDialog::ButtonCode id = KDialog::Ok;
        QString whatsthis = "A simple button to press okay";
        dialog.setButtons(id);
        dialog.setButtonWhatsThis(id, whatsthis);
        QCOMPARE(dialog.buttonWhatsThis(id), whatsthis);
        QCOMPARE(dialog.button(id)->whatsThis(), whatsthis);
    }

    void testDeleteOnClose_data()
    {
        QTest::addColumn<KDialog::ButtonCode>("button");
        QTest::addColumn<int>("emitAccepted");
        QTest::addColumn<int>("emitRejected");

        QTest::newRow("Ok") << KDialog::Ok << 1 << 0;
        QTest::newRow("Cancel") << KDialog::Cancel << 0 << 1;
        QTest::newRow("Close") << KDialog::Close << 0 << 0;
    }

    void testDeleteOnClose()
    {
        QFETCH(KDialog::ButtonCode, button);
        QFETCH(int, emitAccepted);
        QFETCH(int, emitRejected);

        KDialog* dialog = new KDialog;
        QWeakPointer<KDialog> dialogPointer(dialog);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setButtons(KDialog::Ok | button);
        QSignalSpy qAcceptedSpy(dialog, SIGNAL(accepted()));
        QSignalSpy qRejectedSpy(dialog, SIGNAL(rejected()));
        dialog->show(); // KDialog::closeEvent tests for isHidden
        dialog->button(button)->click();
        QCOMPARE(qAcceptedSpy.count(), emitAccepted);
        QCOMPARE(qRejectedSpy.count(), emitRejected);
        QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
        QVERIFY(dialogPointer.isNull()); // deletion happened
    }

    // Closing the dialog using the window manager button
    void testCloseDialogWithDeleteOnClose_data()
    {
        QTest::addColumn<KDialog::ButtonCode>("button");
        QTest::addColumn<int>("emitRejected");
        QTest::addColumn<QString>("signal");

        QTest::newRow("Cancel") << KDialog::Cancel << 1 << SIGNAL(cancelClicked());
        QTest::newRow("Close") << KDialog::Close << 0 << SIGNAL(closeClicked());
    }

    void testCloseDialogWithDeleteOnClose()
    {
        QFETCH(KDialog::ButtonCode, button);
        QFETCH(int, emitRejected);
        QFETCH(QString, signal);

        KDialog* dialog = new KDialog;
        QWeakPointer<KDialog> dialogPointer(dialog);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setButtons(KDialog::Ok | button);
        QSignalSpy qCancelOrCloseClickedSpy(dialog, signal.toLatin1().constData());
        QSignalSpy qRejectedSpy(dialog, SIGNAL(rejected()));
        dialog->show(); // KDialog::closeEvent tests for isHidden
        dialog->close();
        if (qRejectedSpy.isEmpty() && emitRejected)
            QVERIFY(QTest::kWaitForSignal(dialog, SIGNAL(rejected()), 5000));
        if (qCancelOrCloseClickedSpy.isEmpty())
            QVERIFY(QTest::kWaitForSignal(dialog, signal.toLatin1().constData(), 5000));
        QCOMPARE(qRejectedSpy.count(), emitRejected); // and then rejected is emitted as well
        QCOMPARE(qCancelOrCloseClickedSpy.count(), 1); // KDialog emulated cancel or close being clicked
        qApp->sendPostedEvents(); // DeferredDelete
        QVERIFY(dialogPointer.isNull()); // deletion happened
    }
};

QTEST_KDEMAIN(KDialog_UnitTest, GUI)
#include "kdialog_unittest.moc"

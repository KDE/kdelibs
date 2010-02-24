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

#include <qtest_kde.h>
#include <kdialog.h>
#include <kpushbutton.h>

class KDialog_UnitTest : public QObject
{
    Q_OBJECT
private:
    void testNoDefaultButton(KDialog & dialog, KDialog::ButtonCode id)
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

    void testButtonCode(KDialog & dialog, KDialog::ButtonCode id)
    {
        dialog.setDefaultButton(id);
        QCOMPARE(dialog.defaultButton(), id);
        QVERIFY(dialog.button(id)->isDefault());
        testNoDefaultButton(dialog, id);
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
        testButtonCode(dialog, KDialog::Ok);
        testButtonCode(dialog, KDialog::Apply);
        testButtonCode(dialog, KDialog::Cancel);
        testButtonCode(dialog, KDialog::No);
        testButtonCode(dialog, KDialog::Yes);
        dialog.setDefaultButton(KDialog::NoDefault);
        testNoDefaultButton(dialog, KDialog::NoDefault);
    }
    // Test if buttons labels are correctly handled
    // assuming that the previous test passed
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
};

QTEST_KDEMAIN(KDialog_UnitTest, GUI)
#include "kdialog_unittest.moc"

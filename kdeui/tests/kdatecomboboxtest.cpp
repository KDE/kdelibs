/*
    Copyright 2011 John Layt <john@layt.net>

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

#include "kdatecomboboxtest.h"

#include <QtCore/QDate>

#include "qtest_kde.h"
#include "kdebug.h"
#include "kdatecombobox.h"
#include "kcalendarsystem.h"
#include "klineedit.h"

QTEST_KDEMAIN(KDateComboBoxTest, GUI)

#include "kdatecomboboxtest.moc"

void KDateComboBoxTest::testDefaults()
{
    m_combo = new KDateComboBox(0);
    QCOMPARE(m_combo->date(), QDate::currentDate());
    QCOMPARE(m_combo->minimumDate(), KGlobal::locale()->calendar()->earliestValidDate());
    QCOMPARE(m_combo->maximumDate(), KGlobal::locale()->calendar()->latestValidDate());
    QCOMPARE(m_combo->isValid(), true);
    QCOMPARE(m_combo->isNull(), false);
    QCOMPARE(m_combo->options(), KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords);
    QCOMPARE(m_combo->displayFormat(), KLocale::ShortDate);
    delete m_combo;
}

void KDateComboBoxTest::testValidNull()
{
    m_combo = new KDateComboBox(0);
    QCOMPARE(m_combo->isValid(), true);
    QCOMPARE(m_combo->isNull(), false);
    m_combo->setDate(QDate());
    QCOMPARE(m_combo->isValid(), false);
    QCOMPARE(m_combo->isNull(), true);
    m_combo->setDate(QDate(2000, 1, 1));
    m_combo->lineEdit()->setText("invalid");
    QCOMPARE(m_combo->isValid(), false);
    QCOMPARE(m_combo->isNull(), false);
    delete m_combo;
}

void KDateComboBoxTest::testDateRange()
{
    m_combo = new KDateComboBox(0);
    m_combo->setDate(QDate(2000, 1, 1));
    QCOMPARE(m_combo->minimumDate(), KGlobal::locale()->calendar()->earliestValidDate());
    QCOMPARE(m_combo->maximumDate(), KGlobal::locale()->calendar()->latestValidDate());
    QCOMPARE(m_combo->isValid(), true);

    m_combo->setDateRange(QDate(2001, 1, 1), QDate(2002, 1, 1));
    QCOMPARE(m_combo->minimumDate(), QDate(2001, 1, 1));
    QCOMPARE(m_combo->maximumDate(), QDate(2002, 1, 1));
    QCOMPARE(m_combo->isValid(), false);

    m_combo->setDate(QDate(2003, 1, 1));
    QCOMPARE(m_combo->isValid(), false);
    m_combo->setDate(QDate(2001, 6, 1));
    QCOMPARE(m_combo->isValid(), true);
    m_combo->setDate(QDate(2001, 1, 1));
    QCOMPARE(m_combo->isValid(), true);
    m_combo->setDate(QDate(2002, 1, 1));
    QCOMPARE(m_combo->isValid(), true);
    m_combo->setDate(QDate(2000, 12, 31));
    QCOMPARE(m_combo->isValid(), false);
    m_combo->setDate(QDate(2002, 1, 2));
    QCOMPARE(m_combo->isValid(), false);

    m_combo->setDateRange(QDate(1995, 1, 1), QDate(1990, 1, 1));
    QCOMPARE(m_combo->minimumDate(), QDate(2001, 1, 1));
    QCOMPARE(m_combo->maximumDate(), QDate(2002, 1, 1));

    m_combo->setMinimumDate(QDate(2000, 1, 1));
    QCOMPARE(m_combo->minimumDate(), QDate(2000, 1, 1));
    QCOMPARE(m_combo->maximumDate(), QDate(2002, 1, 1));

    m_combo->setMaximumDate(QDate(2003, 1, 1));
    QCOMPARE(m_combo->minimumDate(), QDate(2000, 1, 1));
    QCOMPARE(m_combo->maximumDate(), QDate(2003, 1, 1));

    delete m_combo;
}

void KDateComboBoxTest::testDateList()
{
    m_combo = new KDateComboBox(0);
    QMap<QDate, QString> map;

    // Test default map
    QCOMPARE(m_combo->dateMap(), map);

    // Test basic map
    map.clear();
    map.insert(QDate(2000, 1, 1), QLatin1String("New Years Day"));
    map.insert(QDate(2000, 1, 2), QString());
    map.insert(QDate(2000, 1, 3), QLatin1String("separator"));
    map.insert(QDate(), QLatin1String("No Date"));
    m_combo->setDateMap(map);
    QCOMPARE(m_combo->dateMap(), map);

    delete m_combo;
}

void KDateComboBoxTest::testOptions()
{
    m_combo = new KDateComboBox(0);
    KDateComboBox::Options options = KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords;
    QCOMPARE(m_combo->options(), options);
    options = KDateComboBox::EditDate | KDateComboBox::WarnOnInvalid;
    m_combo->setOptions(options);
    QCOMPARE(m_combo->options(), options);
    delete m_combo;
}

void KDateComboBoxTest::testDisplayFormat()
{
    m_combo = new KDateComboBox(0);
    KLocale::DateFormat format = KLocale::ShortDate;
    QCOMPARE(m_combo->displayFormat(), format);
    format = KLocale::IsoDate;
    m_combo->setDisplayFormat(format);
    QCOMPARE(m_combo->displayFormat(), format);
    delete m_combo;
}

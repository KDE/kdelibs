/*  This file is part of the KDE libraries
 *  Copyright 2013 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdeplatformtheme_config.h"
#include "kstyle.h"

#include <QTest>
#include <QDir>
#include <QFile>
#include <QToolBar>
#include <QToolButton>
#include <QApplication>
#include <QStandardPaths>

#include <QDebug>

static void prepareEnvironment()
{
    QStandardPaths::setTestModeEnabled(true);

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    configPath.append("/kdeglobals");

    QFile::remove(configPath);
    QFile::copy(CONFIGFILE, configPath);
}

Q_COREAPP_STARTUP_FUNCTION(prepareEnvironment);

class KStyle_UnitTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void initTestCase()
        {
            qApp->setStyle(new KStyle);
        }

        void testToolButtonStyleHint()
        {
            QToolBar *toolbar = new QToolBar();
            QToolButton *btn = new QToolButton(toolbar);

            QCOMPARE(qApp->style()->styleHint(QStyle::SH_ToolButtonStyle, 0, btn), (int)  Qt::ToolButtonTextOnly);

            toolbar->setProperty("otherToolbar", true);
            QCOMPARE(qApp->style()->styleHint(QStyle::SH_ToolButtonStyle, 0, btn), (int) Qt::ToolButtonTextUnderIcon);
        }
};

QTEST_MAIN(KStyle_UnitTest)

#include "kstyle_unittest.moc"
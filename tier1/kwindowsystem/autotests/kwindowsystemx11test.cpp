/*
 *   Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "nettesthelper.h"
#include "kwindowsystem.h"

#include <qtest_widgets.h>
#include <QSignalSpy>
#include <QWidget>
Q_DECLARE_METATYPE(WId)

class KWindowSystemX11Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testWindowAdded();
    void testWindowRemoved();
};

void KWindowSystemX11Test::testWindowAdded()
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(windowAdded(WId)));
    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();
    QTest::qWaitForWindowExposed(widget.data());
    QVERIFY(spy.count() > 0);
    bool hasWId = false;
    for (auto it = spy.constBegin(); it != spy.constEnd(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }
        QCOMPARE((*it).count(), 1);
        hasWId = (*it).at(0).toULongLong() == widget->winId();
        if (hasWId) {
            break;
        }
    }
    QVERIFY(hasWId);
    QVERIFY(KWindowSystem::hasWId(widget->winId()));
}

void KWindowSystemX11Test::testWindowRemoved()
{
    qRegisterMetaType<WId>("WId");
    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();
    QTest::qWaitForWindowExposed(widget.data());
    QVERIFY(KWindowSystem::hasWId(widget->winId()));

    QSignalSpy spy(KWindowSystem::self(), SIGNAL(windowRemoved(WId)));
    widget->hide();
    spy.wait(1000);
    QCOMPARE(spy.first().at(0).toULongLong(), widget->winId());
    QVERIFY(!KWindowSystem::hasWId(widget->winId()));
}

QTEST_MAIN(KWindowSystemX11Test)

#include "kwindowsystemx11test.moc"

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

#include <kwindoweffects.h>
#include <qtest_widgets.h>
#include <QX11Info>
#include <xcb/xcb.h>

Q_DECLARE_METATYPE(KWindowEffects::SlideFromLocation)

class KWindowEffectsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testSlideWindow_data();
    void testSlideWindow();
    void testSlideWindowRemove();
    void testSlideWindowWidget_data();
    void testSlideWindowWidget();
    void testSlideWindowWidgetRemove();

private:
    int32_t locationToValue(KWindowEffects::SlideFromLocation location) const;
    void performSlideWindowTest(xcb_window_t window, int offset, KWindowEffects::SlideFromLocation location) const;
    void performSlideWindowRemoveTest(xcb_window_t window);
    xcb_atom_t m_slide;
    QScopedPointer<QWindow> m_window;
    QScopedPointer<QWidget> m_widget;
};

void KWindowEffectsTest::initTestCase()
{
    m_window.reset(new QWindow());
    QVERIFY(m_window->winId() != XCB_WINDOW_NONE);
    m_widget.reset(new QWidget());
    m_widget->show();
    QVERIFY(m_widget->effectiveWinId() != XCB_WINDOW_NONE);
    xcb_connection_t *c = QX11Info::connection();
    const QByteArray slide = QByteArrayLiteral("_KDE_SLIDE");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, slide.length(), slide.constData());

    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, NULL));
    QVERIFY(!atom.isNull());
    m_slide = atom->atom;
}

void KWindowEffectsTest::testSlideWindow_data()
{
    QTest::addColumn<int>("offset");
    QTest::addColumn<KWindowEffects::SlideFromLocation>("location");

    QTest::newRow("Left")   << 10 << KWindowEffects::LeftEdge;
    QTest::newRow("Right")  << 20 << KWindowEffects::RightEdge;
    QTest::newRow("Top")    <<  0 << KWindowEffects::TopEdge;
    QTest::newRow("Bottom") << -1 << KWindowEffects::BottomEdge;
}

void KWindowEffectsTest::testSlideWindow()
{
    QFETCH(int, offset);
    QFETCH(KWindowEffects::SlideFromLocation, location);

    KWindowEffects::slideWindow(m_window->winId(), location, offset);
    performSlideWindowTest(m_window->winId(), offset, location);
}

void KWindowEffectsTest::testSlideWindowRemove()
{
    xcb_window_t window = m_window->winId();
    // first install the atom
    KWindowEffects::slideWindow(window, KWindowEffects::TopEdge, 0);
    performSlideWindowTest(window, 0, KWindowEffects::TopEdge);

    // now delete it
    KWindowEffects::slideWindow(window, KWindowEffects::NoEdge, 0);
    performSlideWindowRemoveTest(window);
}

void KWindowEffectsTest::testSlideWindowWidget_data()
{
    QTest::addColumn<KWindowEffects::SlideFromLocation>("location");

    QTest::newRow("Left")   << KWindowEffects::LeftEdge;
    QTest::newRow("Right")  << KWindowEffects::RightEdge;
    QTest::newRow("Top")    << KWindowEffects::TopEdge;
    QTest::newRow("Bottom") << KWindowEffects::BottomEdge;
}

void KWindowEffectsTest::testSlideWindowWidget()
{
    QFETCH(KWindowEffects::SlideFromLocation, location);

    KWindowEffects::slideWindow(m_widget.data(), location);
    performSlideWindowTest(m_widget->effectiveWinId(), -1, location);
}

void KWindowEffectsTest::testSlideWindowWidgetRemove()
{
    xcb_window_t window = m_widget->effectiveWinId();
    // first install the atom
    KWindowEffects::slideWindow(m_widget.data(), KWindowEffects::TopEdge);
    performSlideWindowTest(window, -1, KWindowEffects::TopEdge);

    // now delete it
    KWindowEffects::slideWindow(m_widget.data(), KWindowEffects::NoEdge);
    performSlideWindowRemoveTest(window);
}

void KWindowEffectsTest::performSlideWindowTest(xcb_window_t window, int offset, KWindowEffects::SlideFromLocation location) const
{
    xcb_connection_t *c = QX11Info::connection();
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(c, false, window, m_slide, m_slide, 0, 100);
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> reply(xcb_get_property_reply(c, cookie, NULL));
    QVERIFY(!reply.isNull());
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(2));
    QCOMPARE(reply->type, m_slide);
    int32_t *data = static_cast<int32_t *>(xcb_get_property_value(reply.data()));
    QCOMPARE(data[0], offset);
    QCOMPARE(data[1], locationToValue(location));
}

void KWindowEffectsTest::performSlideWindowRemoveTest(xcb_window_t window)
{
    xcb_connection_t *c = QX11Info::connection();
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(c, false, window, m_slide, m_slide, 0, 100);
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> reply(xcb_get_property_reply(c, cookie, NULL));
    QVERIFY(!reply.isNull());
    QCOMPARE(reply->type, xcb_atom_t(XCB_ATOM_NONE));
}

int32_t KWindowEffectsTest::locationToValue(KWindowEffects::SlideFromLocation location) const
{
    switch (location) {
    case KWindowEffects::LeftEdge:
        return 0;
    case KWindowEffects::TopEdge:
        return 1;
    case KWindowEffects::RightEdge:
        return 2;
    case KWindowEffects::BottomEdge:
        return 3;
    default:
        return -1;
    }
}

QTEST_MAIN(KWindowEffectsTest)

#include "kwindoweffectstest.moc"

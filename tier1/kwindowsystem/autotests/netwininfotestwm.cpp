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
#include <netwm.h>
#include <qtest_widgets.h>
#include <QProcess>

class Property : public QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter>
{
public:
    Property(xcb_get_property_reply_t *p = 0) : QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter>(p) {}
};

static const int s_propertiesCount = 2;
static const unsigned long s_properties[s_propertiesCount] = {
        ~0ul,
        ~0ul
};

#define INFO NETWinInfo info(m_connection, m_testWindow, m_rootWindow, s_properties, s_propertiesCount, NET::WindowManager);

#define ATOM(name) \
    KXUtils::Atom atom(connection(), QByteArrayLiteral(#name));

#define UTF8 KXUtils::Atom utf8String(connection(), QByteArrayLiteral("UTF8_STRING"));

#define GETPROP(type, length, formatSize) \
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, m_testWindow, \
                                                                  atom, type, 0, length); \
    Property reply(xcb_get_property_reply(connection(), cookie, Q_NULLPTR)); \
    QVERIFY(!reply.isNull()); \
    QCOMPARE(reply->format, uint8_t(formatSize)); \
    QCOMPARE(reply->value_len, uint32_t(length));

#define VERIFYDELETED(t) \
    xcb_get_property_cookie_t cookieDeleted = xcb_get_property_unchecked(connection(), false, m_testWindow, \
                                                                         atom, t, 0, 1); \
    Property replyDeleted(xcb_get_property_reply(connection(), cookieDeleted, Q_NULLPTR)); \
    QVERIFY(!replyDeleted.isNull()); \
    QVERIFY(replyDeleted->type == XCB_ATOM_NONE);

class NetWinInfoTestWM : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testState_data();
    void testState();
    void testVisibleName();
    void testVisibleIconName();
    void testDesktop_data();
    void testDesktop();
    void testOpacity_data();
    void testOpacity();
    void testAllowedActions_data();
    void testAllowedActions();
    void testFrameExtents();
    void testFrameExtentsKDE();
    void testFrameOverlap();

private:
    bool hasAtomFlag(const xcb_atom_t *atoms, int atomsLenght, const QByteArray &actionName);
    void testStrut(xcb_atom_t atom, NETStrut (NETWinInfo:: *getter)(void)const, void (NETWinInfo:: *setter)(NETStrut), NET::Property property, NET::Property2 property2 = NET::Property2(0));
    void waitForPropertyChange(NETWinInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2 = NET::Property2(0));
    xcb_connection_t *connection() {
        return m_connection;
    }
    xcb_connection_t *m_connection;
    QScopedPointer<QProcess> m_xvfb;
    xcb_window_t m_rootWindow;
    xcb_window_t m_testWindow;
};

void NetWinInfoTestWM::initTestCase()
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
    // first reset just to be sure
    m_connection = Q_NULLPTR;
    m_rootWindow = XCB_WINDOW_NONE;
    m_testWindow = XCB_WINDOW_NONE;
    // start Xvfb
    m_xvfb.reset(new QProcess);
    // randomize the display id in [1, 98]
    // 0 is not used because it conflicts with "normal" X server
    // 99 is not used because it's used by KDE's CI infrastructure
    const QString id = QStringLiteral(":") + QString::number((qrand() % 98) + 1);
    m_xvfb->start(QStringLiteral("Xvfb"), QStringList() << id);
    QVERIFY(m_xvfb->waitForStarted());
    QCOMPARE(m_xvfb->state(), QProcess::Running);
    // give it some time before we open the X Display
    QTest::qWait(100);
    // create X connection
    int screen = 0;
    m_connection = xcb_connect(qPrintable(id), &screen);
    QVERIFY(m_connection);
    m_rootWindow = KXUtils::rootWindow(m_connection, screen);
}

void NetWinInfoTestWM::cleanupTestCase()
{
    // close connection
    xcb_disconnect(connection());
    // kill Xvfb
    m_xvfb->terminate();
    m_xvfb->waitForFinished();
}

void NetWinInfoTestWM::init()
{
    // create test window
    m_testWindow = xcb_generate_id(m_connection);
    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_create_window(m_connection, XCB_COPY_FROM_PARENT, m_testWindow,
                      m_rootWindow,
                      0, 0, 100, 100, 0, XCB_COPY_FROM_PARENT,
                      XCB_COPY_FROM_PARENT, XCB_CW_EVENT_MASK, values);
    // and map it
    xcb_map_window(m_connection, m_testWindow);
}

void NetWinInfoTestWM::cleanup()
{
    // destroy test window
    xcb_unmap_window(m_connection, m_testWindow);
    xcb_destroy_window(m_connection, m_testWindow);
    m_testWindow = XCB_WINDOW_NONE;
}

void NetWinInfoTestWM::waitForPropertyChange(NETWinInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2)
{
    while (true) {
        KXUtils::ScopedCPointer<xcb_generic_event_t> event(xcb_wait_for_event(connection()));
        if (event.isNull()) {
            break;
        }
        if ((event->response_type & ~0x80) != XCB_PROPERTY_NOTIFY) {
            continue;
        }
        xcb_property_notify_event_t *pe = reinterpret_cast<xcb_property_notify_event_t*>(event.data());
        if (pe->window != m_testWindow) {
            continue;
        }
        if (pe->atom != atom) {
            continue;
        }
        unsigned long dirty[ 2 ];
        info->event(event.data(), dirty, 2);
        if (prop != 0) {
            QVERIFY(dirty[0] & prop);
        }
        if (prop2 != 0) {
            QVERIFY(dirty[1] & prop2);
        }
        if (!prop) {
            QCOMPARE(dirty[0], 0ul);
        }
        if (!prop2) {
            QCOMPARE(dirty[1], 0ul);
        }
        break;
    }
}

bool NetWinInfoTestWM::hasAtomFlag(const xcb_atom_t *atoms, int atomsLength, const QByteArray &actionName)
{
    KXUtils::Atom atom(connection(), actionName);
    if (atom == XCB_ATOM_NONE) {
        qDebug() << "get atom failed";
        return false;
    }
    for (int i = 0; i < atomsLength; ++i) {
        if (atoms[i] == atom) {
            return true;
        }
    }
    return false;
}

Q_DECLARE_FLAGS(Actions, NET::Action)
Q_DECLARE_METATYPE(Actions)

void NetWinInfoTestWM::testAllowedActions_data()
{
    QTest::addColumn<Actions>("actions");
    QTest::addColumn<QVector<QByteArray> >("names");

    const QByteArray move       = QByteArrayLiteral("_NET_WM_ACTION_MOVE");
    const QByteArray resize     = QByteArrayLiteral("_NET_WM_ACTION_RESIZE");
    const QByteArray minimize   = QByteArrayLiteral("_NET_WM_ACTION_MINIMIZE");
    const QByteArray shade      = QByteArrayLiteral("_NET_WM_ACTION_SHADE");
    const QByteArray stick      = QByteArrayLiteral("_NET_WM_ACTION_STICK");
    const QByteArray maxVert    = QByteArrayLiteral("_NET_WM_ACTION_MAXIMIZE_VERT");
    const QByteArray maxHoriz   = QByteArrayLiteral("_NET_WM_ACTION_MAXIMIZE_HORZ");
    const QByteArray fullscreen = QByteArrayLiteral("_NET_WM_ACTION_FULLSCREEN");
    const QByteArray desktop    = QByteArrayLiteral("_NET_WM_ACTION_CHANGE_DESKTOP");
    const QByteArray close      = QByteArrayLiteral("_NET_WM_ACTION_CLOSE");

    QTest::newRow("move")       << Actions(NET::ActionMove)          << (QVector<QByteArray>() << move);
    QTest::newRow("resize")     << Actions(NET::ActionResize)        << (QVector<QByteArray>() << resize);
    QTest::newRow("minimize")   << Actions(NET::ActionMinimize)      << (QVector<QByteArray>() << minimize);
    QTest::newRow("shade")      << Actions(NET::ActionShade)         << (QVector<QByteArray>() << shade);
    QTest::newRow("stick")      << Actions(NET::ActionStick)         << (QVector<QByteArray>() << stick);
    QTest::newRow("maxVert")    << Actions(NET::ActionMaxVert)       << (QVector<QByteArray>() << maxVert);
    QTest::newRow("maxHoriz")   << Actions(NET::ActionMaxHoriz)      << (QVector<QByteArray>() << maxHoriz);
    QTest::newRow("fullscreen") << Actions(NET::ActionFullScreen)    << (QVector<QByteArray>() << fullscreen);
    QTest::newRow("desktop")    << Actions(NET::ActionChangeDesktop) << (QVector<QByteArray>() << desktop);
    QTest::newRow("close")      << Actions(NET::ActionClose)         << (QVector<QByteArray>() << close);

    QTest::newRow("none") << Actions() << QVector<QByteArray>();

    QTest::newRow("all") << Actions(NET::ActionMove |
                                    NET::ActionResize |
                                    NET::ActionMinimize |
                                    NET::ActionShade |
                                    NET::ActionStick |
                                    NET::ActionMaxVert |
                                    NET::ActionMaxHoriz |
                                    NET::ActionFullScreen |
                                    NET::ActionChangeDesktop |
                                    NET::ActionClose)
                        << (QVector<QByteArray>() << move << resize << minimize << shade <<
                                                     stick << maxVert << maxHoriz <<
                                                     fullscreen << desktop << close);
}

void NetWinInfoTestWM::testAllowedActions()
{
    QVERIFY(connection());
    ATOM(_NET_WM_ALLOWED_ACTIONS)
    INFO

    QCOMPARE(info.allowedActions(), 0ul);
    QFETCH(Actions, actions);
    info.setAllowedActions(actions);
    QCOMPARE(info.allowedActions(), static_cast<unsigned long>(actions));

    // compare with the X property
    QFETCH(QVector<QByteArray>, names);
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_ATOM, names.size(), 32)
    xcb_atom_t *atoms = reinterpret_cast<xcb_atom_t*>(xcb_get_property_value(reply.data()));
    for (int i = 0; i < names.size(); ++i) {
        QVERIFY(hasAtomFlag(atoms, names.size(), names.at(i)));
    }

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2AllowedActions);
    QCOMPARE(info.allowedActions(), static_cast<unsigned long>(actions));
}

void NetWinInfoTestWM::testDesktop_data()
{
    QTest::addColumn<int>("desktop");
    QTest::addColumn<uint32_t>("propertyDesktop");

    QTest::newRow("1") << 1 << uint32_t(0);
    QTest::newRow("4") << 4 << uint32_t(3);
    QTest::newRow("on all") << int(NET::OnAllDesktops) << uint32_t(~0);
}

void NetWinInfoTestWM::testDesktop()
{
    QVERIFY(connection());
    ATOM(_NET_WM_DESKTOP)
    INFO

    QCOMPARE(info.desktop(), 0);
    QFETCH(int, desktop);
    info.setDesktop(desktop);
    QCOMPARE(info.desktop(), desktop);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QTEST(reinterpret_cast<uint32_t*>(xcb_get_property_value(reply.data()))[0], "propertyDesktop");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMDesktop);
    QCOMPARE(info.desktop(), desktop);

    // delete it
    info.setDesktop(0);
    QCOMPARE(info.desktop(), 0);
    VERIFYDELETED(XCB_ATOM_CARDINAL)

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMDesktop);
    QCOMPARE(info.desktop(), 0);
}

void NetWinInfoTestWM::testStrut(xcb_atom_t atom, NETStrut (NETWinInfo:: *getter)(void)const, void (NETWinInfo:: *setter)(NETStrut), NET::Property property, NET::Property2 property2)
{
    INFO

    NETStrut extents = (info.*getter)();
    QCOMPARE(extents.bottom, 0);
    QCOMPARE(extents.left, 0);
    QCOMPARE(extents.right, 0);
    QCOMPARE(extents.top, 0);

    NETStrut newExtents;
    newExtents.bottom = 10;
    newExtents.left   = 20;
    newExtents.right  = 30;
    newExtents.top    = 40;
    (info.*setter)(newExtents);
    extents = (info.*getter)();
    QCOMPARE(extents.bottom, newExtents.bottom);
    QCOMPARE(extents.left,   newExtents.left);
    QCOMPARE(extents.right,  newExtents.right);
    QCOMPARE(extents.top,    newExtents.top);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 4, 32)
    uint32_t *data = reinterpret_cast<uint32_t*>(xcb_get_property_value(reply.data()));
    QCOMPARE(data[0], uint32_t(newExtents.left));
    QCOMPARE(data[1], uint32_t(newExtents.right));
    QCOMPARE(data[2], uint32_t(newExtents.top));
    QCOMPARE(data[3], uint32_t(newExtents.bottom));

    // and wait for our event
    waitForPropertyChange(&info, atom, property, property2);
    extents = (info.*getter)();
    QCOMPARE(extents.bottom, newExtents.bottom);
    QCOMPARE(extents.left,   newExtents.left);
    QCOMPARE(extents.right,  newExtents.right);
    QCOMPARE(extents.top,    newExtents.top);
}

void NetWinInfoTestWM::testFrameExtents()
{
    QVERIFY(connection());
    ATOM(_NET_FRAME_EXTENTS)
    testStrut(atom, &NETWinInfo::frameExtents, &NETWinInfo::setFrameExtents, NET::WMFrameExtents);
}

void NetWinInfoTestWM::testFrameExtentsKDE()
{
    // same as testFrameExtents just with a different atom name
    QVERIFY(connection());
    ATOM(_KDE_NET_WM_FRAME_STRUT)
    testStrut(atom, &NETWinInfo::frameExtents, &NETWinInfo::setFrameExtents, NET::WMFrameExtents);
}

void NetWinInfoTestWM::testFrameOverlap()
{
    QVERIFY(connection());
    ATOM(_NET_WM_FRAME_OVERLAP)
    testStrut(atom, &NETWinInfo::frameOverlap, &NETWinInfo::setFrameOverlap, NET::Property(0), NET::WM2FrameOverlap);
}

void NetWinInfoTestWM::testOpacity_data()
{
    QTest::addColumn<uint32_t>("opacity");

    QTest::newRow("0 %")   << uint32_t(0);
    QTest::newRow("50 %")  << uint32_t(0x0000ffff);
    QTest::newRow("100 %") << uint32_t(0xffffffff);
}

void NetWinInfoTestWM::testOpacity()
{
    QVERIFY(connection());
    ATOM(_NET_WM_WINDOW_OPACITY)
    INFO

    QCOMPARE(info.opacity(), static_cast<unsigned long>(0xffffffffU));
    QFETCH(uint32_t, opacity);
    info.setOpacity(opacity);
    QCOMPARE(info.opacity(), static_cast<unsigned long>(opacity));

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QCOMPARE(reinterpret_cast<uint32_t*>(xcb_get_property_value(reply.data()))[0], opacity);

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2Opacity);
    QCOMPARE(info.opacity(), static_cast<unsigned long>(opacity));
}

Q_DECLARE_FLAGS(States, NET::State)
Q_DECLARE_METATYPE(States)
void NetWinInfoTestWM::testState_data()
{
    QTest::addColumn<States>("states");
    QTest::addColumn<QVector<QByteArray> >("names");

    const QByteArray modal            = QByteArrayLiteral("_NET_WM_STATE_MODAL");
    const QByteArray sticky           = QByteArrayLiteral("_NET_WM_STATE_STICKY");
    const QByteArray maxVert          = QByteArrayLiteral("_NET_WM_STATE_MAXIMIZED_VERT");
    const QByteArray maxHoriz         = QByteArrayLiteral("_NET_WM_STATE_MAXIMIZED_HORZ");
    const QByteArray shaded           = QByteArrayLiteral("_NET_WM_STATE_SHADED");
    const QByteArray skipTaskbar      = QByteArrayLiteral("_NET_WM_STATE_SKIP_TASKBAR");
    const QByteArray keepAbove        = QByteArrayLiteral("_NET_WM_STATE_ABOVE");
    const QByteArray staysOnTop       = QByteArrayLiteral("_NET_WM_STATE_STAYS_ON_TOP");
    const QByteArray skipPager        = QByteArrayLiteral("_NET_WM_STATE_SKIP_PAGER");
    const QByteArray hidden           = QByteArrayLiteral("_NET_WM_STATE_HIDDEN");
    const QByteArray fullScreen       = QByteArrayLiteral("_NET_WM_STATE_FULLSCREEN");
    const QByteArray keepBelow        = QByteArrayLiteral("_NET_WM_STATE_BELOW");
    const QByteArray demandsAttention = QByteArrayLiteral("_NET_WM_STATE_DEMANDS_ATTENTION");

    QTest::newRow("modal")            << States(NET::Modal)            << (QVector<QByteArray>() << modal);
    QTest::newRow("sticky")           << States(NET::Sticky)           << (QVector<QByteArray>() << sticky);
    QTest::newRow("maxVert")          << States(NET::MaxVert)          << (QVector<QByteArray>() << maxVert);
    QTest::newRow("maxHoriz")         << States(NET::MaxHoriz)         << (QVector<QByteArray>() << maxHoriz);
    QTest::newRow("shaded")           << States(NET::Shaded)           << (QVector<QByteArray>() << shaded);
    QTest::newRow("skipTaskbar")      << States(NET::SkipTaskbar)      << (QVector<QByteArray>() << skipTaskbar);
    QTest::newRow("keepAbove")        << States(NET::KeepAbove)        << (QVector<QByteArray>() << keepAbove << staysOnTop);
    QTest::newRow("staysOnTop")       << States(NET::StaysOnTop)       << (QVector<QByteArray>() << keepAbove << staysOnTop);
    QTest::newRow("skipPager")        << States(NET::SkipPager)        << (QVector<QByteArray>() << skipPager);
    QTest::newRow("hidden")           << States(NET::Hidden)           << (QVector<QByteArray>() << hidden);
    QTest::newRow("fullScreen")       << States(NET::FullScreen)       << (QVector<QByteArray>() << fullScreen);
    QTest::newRow("keepBelow")        << States(NET::KeepBelow)        << (QVector<QByteArray>() << keepBelow);
    QTest::newRow("demandsAttention") << States(NET::DemandsAttention) << (QVector<QByteArray>() << demandsAttention);

    // TODO: it's possible to be keep above and below at the same time?!?
    QTest::newRow("all") << States(NET::Modal |
                                   NET::Sticky |
                                   NET::Max |
                                   NET::Shaded |
                                   NET::SkipTaskbar |
                                   NET::SkipPager |
                                   NET::KeepAbove |
                                   NET::KeepBelow |
                                   NET::Hidden |
                                   NET::FullScreen |
                                   NET::DemandsAttention)
                          << (QVector<QByteArray>() << modal << sticky << maxVert << maxHoriz
                                                    << shaded << skipTaskbar << keepAbove
                                                    << skipPager << hidden << fullScreen
                                                    << keepBelow << demandsAttention << staysOnTop);
}

void NetWinInfoTestWM::testState()
{
    QVERIFY(connection());
    ATOM(_NET_WM_STATE)
    INFO

    QCOMPARE(info.state(), 0ul);
    QFETCH(States, states);
    info.setState(states, 0);
    QCOMPARE(info.state(), static_cast<unsigned long>(states));

    // compare with the X property
    QFETCH(QVector<QByteArray>, names);
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_ATOM, names.size(), 32)
    xcb_atom_t *atoms = reinterpret_cast<xcb_atom_t*>(xcb_get_property_value(reply.data()));
    for (int i = 0; i < names.size(); ++i) {
        QVERIFY(hasAtomFlag(atoms, names.size(), names.at(i)));
    }

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMState);
    QCOMPARE(info.state(), static_cast<unsigned long>(states));
}

void NetWinInfoTestWM::testVisibleIconName()
{
    QVERIFY(connection());
    ATOM(_NET_WM_VISIBLE_ICON_NAME)
    UTF8
    INFO

    QVERIFY(!info.visibleIconName());
    info.setVisibleIconName("foo");
    QCOMPARE(info.visibleIconName(), "foo");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    GETPROP(utf8String, 3, 8)
    QCOMPARE(reinterpret_cast<const char*>(xcb_get_property_value(reply.data())), "foo");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleIconName);
    QCOMPARE(info.visibleIconName(), "foo");

    // delete the string
    info.setVisibleIconName("");
    QCOMPARE(info.visibleIconName(), "");
    VERIFYDELETED(utf8String)
    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleIconName);
    QVERIFY(!info.visibleIconName());
}

void NetWinInfoTestWM::testVisibleName()
{
    QVERIFY(connection());
    ATOM(_NET_WM_VISIBLE_NAME)
    UTF8
    INFO

    QVERIFY(!info.visibleName());
    info.setVisibleName("foo");
    QCOMPARE(info.visibleName(), "foo");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    GETPROP(utf8String, 3, 8)
    QCOMPARE(reinterpret_cast<const char*>(xcb_get_property_value(reply.data())), "foo");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleName);
    QCOMPARE(info.visibleName(), "foo");

    // delete the string
    info.setVisibleName("");
    QCOMPARE(info.visibleName(), "");
    VERIFYDELETED(utf8String)

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleName);
    QVERIFY(!info.visibleName());
}

QTEST_MAIN(NetWinInfoTestWM)

#include "netwininfotestwm.moc"

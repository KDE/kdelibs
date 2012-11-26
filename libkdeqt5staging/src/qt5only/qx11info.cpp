//
// An implementation of QX11Info for Qt5. This code only provides the
// static methods of the QX11Info, not the methods for getting information
// about particular widgets or pixmaps.
//

#include "qx11info_x11.h"

#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>
#include <qscreen.h>
#include <qdesktopwidget.h>
#include <qwindow.h>
#include <qapplication.h>
#include <xcb/xcb.h>

#include <X11/Xlib.h>

QX11Info::QX11Info()
{
}

int QX11Info::appDpiX(int screen)
{
    if (screen == -1) {
        const QScreen *screen = QGuiApplication::primaryScreen();
        return qRound(screen->logicalDotsPerInchX());
    }

    QList<QScreen *> screens = QGuiApplication::screens();
    if (screen >= screens.size())
        return 0;

    return screens[screen]->logicalDotsPerInchX();
}

int QX11Info::appDpiY(int screen)
{
    if (screen == -1) {
        const QScreen *screen = QGuiApplication::primaryScreen();
        return qRound(screen->logicalDotsPerInchY());
    }

    QList<QScreen *> screens = QGuiApplication::screens();
    if (screen > screens.size())
        return 0;

    return screens[screen]->logicalDotsPerInchY();
}

void QX11Info::setAppDpiX(int screen, int xdpi)
{
    // ### Fixme
    Q_UNUSED(screen);
    Q_UNUSED(xdpi);
}

void QX11Info::setAppDpiY(int screen, int ydpi)
{
    // ### Fixme
    Q_UNUSED(screen);
    Q_UNUSED(ydpi);
}

unsigned long QX11Info::appRootWindow(int screen)
{
#if 0
    // This looks like it should work, but gives the wrong value.
    QDesktopWidget *desktop = QApplication::desktop();
    QWidget *screenWidget = desktop->screen(screen);
    QWindow *window = screenWidget->windowHandle();
#else
    Q_UNUSED(screen);

    QDesktopWidget *desktop = QApplication::desktop();
    QWindow *window = desktop->windowHandle();
#endif
    return window->winId();
}

int QX11Info::appScreen()
{
    QDesktopWidget *desktop = QApplication::desktop();
    return desktop->primaryScreen();
}

unsigned long QX11Info::appTime()
{
    return 0L;
}

unsigned long QX11Info::appUserTime()
{
    return 0L;
}

void QX11Info::setAppTime(unsigned long time)
{
    Q_UNUSED(time);
}

void QX11Info::setAppUserTime(unsigned long time)
{
    QPlatformNativeInterface *native = qApp->platformNativeInterface();

    QDesktopWidget *desktop = QApplication::desktop();
    QWindow *window = desktop->windowHandle();

    xcb_timestamp_t timestamp = uint32_t(time);
    QMetaObject::invokeMethod(native, "updateNetWmUserTime", Qt::DirectConnection,
                              Q_ARG(QWindow *,window), Q_ARG(xcb_timestamp_t, timestamp));
}

Display *QX11Info::display()
{
    QPlatformNativeInterface *native = qApp->platformNativeInterface();

    void *display = native->nativeResourceForScreen(QByteArray("display"), QGuiApplication::primaryScreen());
    return reinterpret_cast<Display *>(display);
}

xcb_connection_t *QX11Info::connection()
{
    QPlatformNativeInterface *native = qApp->platformNativeInterface();

    void *connection = native->nativeResourceForWindow(QByteArray("connection"), 0);
    return reinterpret_cast<xcb_connection_t *>(connection);
}


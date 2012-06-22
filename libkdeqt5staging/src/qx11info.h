#ifndef QX11INFO_H
#define QX11INFO_H

struct Display;
struct xcb_connection_t;

class QX11Info
{
public:
    static int appDpiX(int screen=-1);
    static int appDpiY(int screen=-1);

    static void setAppDpiX(int screen, int xdpi);
    static void setAppDpiY(int screen, int ydpi);

    static WId appRootWindow(int screen=-1);
    static int appScreen();

    static unsigned long appTime();
    static unsigned long appUserTime();

    static void setAppTime(unsigned long time);
    static void setAppUserTime(unsigned long time);

    static Display *display();
    static xcb_connection_t *connection();

private:
    QX11Info();
};

#endif // QX11INFO_H


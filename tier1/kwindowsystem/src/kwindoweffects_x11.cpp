/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "kwindoweffects.h"

#include <QVarLengthArray>

#include "kwindowsystem.h"
#include <config-kwindowsystem.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <QX11Info>

static const char *DASHBOARD_WIN_NAME = "dashboard";
static const char *DASHBOARD_WIN_CLASS = "dashboard";

namespace KWindowEffects
{

bool isEffectAvailable(Effect effect)
{
    if (!KWindowSystem::self()->compositingActive()) {
        return false;
    }
    QString effectName;

    switch (effect) {
    case Slide:
        effectName = QLatin1String("_KDE_SLIDE");
        break;
    case WindowPreview:
        effectName = QLatin1String("_KDE_WINDOW_PREVIEW");
        break;
    case PresentWindows:
        effectName = QLatin1String("_KDE_PRESENT_WINDOWS_DESKTOP");
        break;
    case PresentWindowsGroup:
        effectName = QLatin1String("_KDE_PRESENT_WINDOWS_GROUP");
        break;
    case HighlightWindows:
        effectName = QLatin1String("_KDE_WINDOW_HIGHLIGHT");
        break;
    case BlurBehind:
        effectName = QLatin1String("_KDE_NET_WM_BLUR_BEHIND_REGION");
        break;
    case Dashboard:
        // TODO: Better namespacing for atoms
        effectName = QLatin1String("_WM_EFFECT_KDE_DASHBOARD");
        break;
    default:
        return false;
    }

    // hackish way to find out if KWin has the effect enabled,
    // TODO provide proper support
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, effectName.toLatin1().data(), False);
    int cnt;
    Atom *list = XListProperties(dpy, DefaultRootWindow(dpy), &cnt);
    if (list != NULL) {
        bool ret = (qFind(list, list + cnt, atom) != list + cnt);
        XFree(list);
        return ret;
    }
    return false;
}

void slideWindow(WId id, SlideFromLocation location, int offset)
{
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom( dpy, "_KDE_SLIDE", False );
    QVarLengthArray<long, 2> data(2);

    data[0] = offset;

    switch (location) {
    case LeftEdge:
        data[1] = 0;
        break;
    case TopEdge:
        data[1] = 1;
        break;
    case RightEdge:
        data[1] = 2;
        break;
    case BottomEdge:
        data[1] = 3;
    default:
        break;
    }

    if (location == NoEdge) {
        XDeleteProperty(dpy, id, atom);
    } else {
        XChangeProperty(dpy, id, atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
}

void slideWindow(QWidget *widget, SlideFromLocation location)
{
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom( dpy, "_KDE_SLIDE", False );
    QVarLengthArray<long, 2> data(2);
    data[0] = -1;

    switch (location) {
    case LeftEdge:
        data[1] = 0;
        break;
    case TopEdge:
        data[1] = 1;
        break;
    case RightEdge:
        data[1] = 2;
        break;
    case BottomEdge:
        data[1] = 3;
    default:
        break;
    }

    if (location == NoEdge) {
        XDeleteProperty(dpy, widget->effectiveWinId(), atom);
    } else {
        XChangeProperty(dpy, widget->effectiveWinId(), atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
}

QList<QSize> windowSizes(const QList<WId> &ids)
{
    QList<QSize> windowSizes;
    Q_FOREACH (WId id, ids) {
        if (id > 0) {
            KWindowInfo info = KWindowSystem::windowInfo(id, NET::WMGeometry|NET::WMFrameExtents);
            windowSizes.append(info.frameGeometry().size());
        } else {
            windowSizes.append(QSize());
        }
    }
    return windowSizes;
}

void showWindowThumbnails(WId parent, const QList<WId> &windows, const QList<QRect> &rects)
{
    if (windows.size() != rects.size()) {
        return;
    }
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
    if (windows.isEmpty()) {
        XDeleteProperty(dpy, parent, atom);
        return;
    }

    int numWindows = windows.size();

    // 64 is enough for 10 windows and is a nice base 2 number
    QVarLengthArray<long, 64> data(1 + (6 * numWindows));
    data[0] = numWindows;

    QList<WId>::const_iterator windowsIt;
    QList<QRect>::const_iterator rectsIt = rects.constBegin();
    int i = 0;
    for (windowsIt = windows.constBegin(); windowsIt != windows.constEnd(); ++windowsIt) {

        const int start = (i * 6) + 1;
        const QRect thumbnailRect = (*rectsIt);

        data[start] = 5;
        data[start+1] = (*windowsIt);
        data[start+2] = thumbnailRect.x();
        data[start+3] = thumbnailRect.y();
        data[start+4] = thumbnailRect.width();
        data[start+5] = thumbnailRect.height();
        ++rectsIt;
        ++i;
    }

    XChangeProperty(dpy, parent, atom, atom, 32, PropModeReplace,
                    reinterpret_cast<unsigned char *>(data.data()), data.size());
}

void presentWindows(WId controller, const QList<WId> &ids)
{
    const int numWindows = ids.count();
    QVarLengthArray<long, 32> data(numWindows);
    int actualCount = 0;

    for (int i = 0; i < numWindows; ++i) {
        data[i] = ids.at(i);
        ++actualCount;

    }

    if (actualCount != numWindows) {
        data.resize(actualCount);
    }

    if (!data.isEmpty()) {
        Display *dpy = QX11Info::display();
        Atom atom = XInternAtom(dpy, "_KDE_PRESENT_WINDOWS_GROUP", False);
        XChangeProperty(dpy, controller, atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
}

void presentWindows(WId controller, int desktop)
{
    QVarLengthArray<long, 1> data(1);
    data[0] = desktop;
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_PRESENT_WINDOWS_DESKTOP", False);
    XChangeProperty(dpy, controller, atom, atom, 32, PropModeReplace,
                    reinterpret_cast<unsigned char *>(data.data()), data.size());
}

void highlightWindows(WId controller, const QList<WId> &ids)
{
    const int numWindows = ids.count();
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_HIGHLIGHT", False);

    if (numWindows == 0) {
        Atom atom = XInternAtom(dpy, "_KDE_WINDOW_HIGHLIGHT", False);
        XDeleteProperty(dpy, controller, atom);
    }

    QVarLengthArray<long, 32> data(numWindows);
    int actualCount = 0;

    for (int i = 0; i < numWindows; ++i) {
        data[i] = ids.at(i);
        ++actualCount;

    }

    if (actualCount != numWindows) {
        data.resize(actualCount);
    }

    if (!data.isEmpty()) {
        XChangeProperty(dpy, controller, atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
}

void enableBlurBehind(WId window, bool enable, const QRegion &region)
{
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_BLUR_BEHIND_REGION", False);

    if (enable) {
        QVector<QRect> rects = region.rects();
        QVector<unsigned long> data;
        Q_FOREACH (const QRect &r, rects) {
            data << r.x() << r.y() << r.width() << r.height();
        }

        XChangeProperty(dpy, window, atom, XA_CARDINAL, 32, PropModeReplace,
                        reinterpret_cast<const unsigned char *>(data.constData()), data.size());
    } else {
        XDeleteProperty(dpy, window, atom);
    }
}

void markAsDashboard(WId window)
{
    XClassHint classHint;
    classHint.res_name = const_cast<char *>(DASHBOARD_WIN_NAME);
    classHint.res_class = const_cast<char *>(DASHBOARD_WIN_CLASS);
    XSetClassHint(QX11Info::display(), window, &classHint);
}

}


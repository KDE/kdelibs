/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
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

#ifndef CONTAINMENT_P_H
#define CONTAINMENT_P_H

#include <kactioncollection.h>
#include <kmenu.h>

#include "plasma.h"
#include "applet.h"
#include "corona.h"

static const int INTER_CONTAINMENT_MARGIN = 6;
static const int CONTAINMENT_COLUMNS = 2;
static const int VERTICAL_STACKING_OFFSET = 10000;

class KJob;

namespace KIO
{
    class Job;
}

namespace Plasma
{

class AccessAppletJob;
class Containment;

class ContainmentPrivate
{
public:
    ContainmentPrivate(Containment *c)
        : q(c),
          formFactor(Planar),
          location(Floating),
          focusedApplet(0),
          wallpaper(0),
          screen(-1), // no screen
          lastScreen(-1),
          desktop(-1), // all desktops
          lastDesktop(-1),
          type(Containment::NoContainmentType),
          showDropZoneDelayTimer(0),
          drawWallpaper(true),
          dropZoneStarted(false),
          containmentActionsSource(Global)
    {
    }

    ~ContainmentPrivate()
    {
        // qDeleteAll is broken with Qt4.8, delete it by hand
        while (!applets.isEmpty()) {
            delete applets.first();
        }
        applets.clear();

        qDeleteAll(dropMenus);
        dropMenus.clear();
    }

    void triggerShowAddWidgets();
    void requestConfiguration();
    void checkStatus(Plasma::ItemStatus status);
    void setScreen(int newScreen, int newDesktop, bool preventInvalidDesktops = true);

    /**
     * Called when constraints have been updated on this containment to provide
     * constraint services common to all containments. Containments should still
     * implement their own constraintsEvent method
     */
    void containmentConstraintsEvent(Plasma::Constraints constraints);

    void initApplets();
    void checkContainmentFurniture();
    bool isPanelContainment() const;
    void setLockToolText();
    void appletDeleted(Applet*);
    void appletAppeared(Applet*);
    void clearDataForMimeJob(KIO::Job *job);
    void remoteAppletReady(Plasma::AccessAppletJob *job);
    void mimeTypeRetrieved(KIO::Job *job, const QString &mimetype);
    void dropJobResult(KJob *);
    void addContainmentActions(KMenu &desktopMenu, QEvent *event);
    void addAppletActions(KMenu &desktopMenu, Applet *applet, QEvent *event);
    void checkRemoveAction();
    void configChanged();

    Applet *addApplet(const QString &name, const QVariantList &args = QVariantList(),
                      const QRectF &geometry = QRectF(-1, -1, -1, -1), uint id = 0,
                      bool delayedInit = false);

    KActionCollection *actions();

    /**
     * add the regular actions & keyboard shortcuts onto Applet's collection
     */
    static void addDefaultActions(KActionCollection *actions, Containment *c = 0);

    /**
     * give keyboard focus to applet within this containment
     */
    void focusApplet(Plasma::Applet *applet);

    /**
     * Handles dropped/pasted mimetype data
     * @param screenPos screen-relative position
     * @param dropEvent the drop event (if null, the clipboard is used instead)
     */
    void dropData(QPoint screenPos, QDropEvent *dropEvent = 0);

    /**
     * inits the containmentactions if necessary
     * if it needs configuring, this warns the user and returns false
     * if a menu is passed in, then it populates that menu with the actions from the plugin
     * @param trigger the string to identify the correct plugin with
     * @param screenPos used to show the configure menu, only used if no menu is passed in
     * @param menu an optional menu to use to populate with actions, instead of triggering the
     *             action directly
     * @return true if it's ok to run the action
     */
    bool prepareContainmentActions(const QString &trigger, const QPoint &screenPos, KMenu *menu = 0);

    /**
     * Delayed drop zone display
     */
    void showDropZoneDelayed();

    QHash<QString, ContainmentActions*> *actionPlugins();

    static bool s_positioningPanels;

    Containment *q;
    FormFactor formFactor;
    Location location;
    Applet::List applets;
    Applet *focusedApplet;
    Plasma::Wallpaper *wallpaper;
    QHash<QString, ContainmentActions*> localActionPlugins;
    int screen;
    int lastScreen;
    int desktop;
    int lastDesktop;
    QList<QAction *> toolBoxActions;
    QString activityId;
    Containment::Type type;
    QHash<KJob*, QPointF> dropPoints;
    QHash<KJob*, KMenu*> dropMenus;
    QTimer *showDropZoneDelayTimer;
    bool drawWallpaper : 1;
    bool dropZoneStarted : 1;

    enum ContainmentActionsSource {
        Global = 0,
        Activity,
        Local
    };
    ContainmentActionsSource containmentActionsSource;
    static QHash<QString, ContainmentActions*> globalActionPlugins;
    static const char defaultWallpaper[];
    static const char defaultWallpaperMode[];
};

} // Plasma namespace

#endif

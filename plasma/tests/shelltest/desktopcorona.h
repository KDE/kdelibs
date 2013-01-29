/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2013 Marco Martin <mart@kde.org>
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
class  *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DESKTOPCORONA_H
#define DESKTOPCORONA_H

#include "plasma/corona.h"

class QDesktopWidget;
class QQuickView;

namespace Plasma
{
    class Applet;
} // namespace Plasma


class DesktopCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    explicit DesktopCorona(QObject * parent = 0);
    ~DesktopCorona();

    /**
     * Loads the default (system wide) layout for this user
     **/
    void loadDefaultLayout();

    /**
     * Ensures we have the necessary containments for every screen
     */
    void checkScreens(bool signalWhenExists = false);

    /**
     * Ensures we have the necessary containments for the given screen
     */
    void checkScreen(int screen, bool signalWhenExists = false);

    void checkDesktop(/*Activity *activity,*/ bool signalWhenExists, int screen, int desktop);

    int numScreens() const;
    QRect screenGeometry(int id) const;
    QRegion availableScreenRegion(int id) const;
    QRect availableScreenRect(int id) const;


protected Q_SLOTS:
    void screenCountChanged(int newCount);
    void screenResized(int screen);
    void workAreaResized(int screen);

    void checkViews();

private:
    QDesktopWidget *m_desktopWidget;
    QList <QQuickView *> m_views;
};

#endif



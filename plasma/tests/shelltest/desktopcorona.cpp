/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "desktopcorona.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include "view.h"

static const QString s_panelTemplatesPath("plasma-layout-templates/panels/*");

DesktopCorona::DesktopCorona(QObject *parent)
    : Plasma::Corona(parent),
      m_desktopWidget(QApplication::desktop())
{
    connect(m_desktopWidget, SIGNAL(resized(int)),
            this, SLOT(screenResized(int)));
    connect(m_desktopWidget, SIGNAL(screenCountChanged(int)),
            this, SLOT(screenCountChanged(int)));
    connect(m_desktopWidget, SIGNAL(workAreaResized(int)),
            this, SLOT(workAreaResized(int)));

    checkViews();
}

DesktopCorona::~DesktopCorona()
{
}



void DesktopCorona::loadDefaultLayout()
{
    Plasma::Containment *cont = addContainment("desktop");
    Plasma::Applet *appl = cont->addApplet("foo");
    qDebug() << "Containment:" << cont->name() << "Applet:" << appl->name() << appl;
}

void DesktopCorona::checkScreens(bool signalWhenExists)
{
    // quick sanity check to ensure we have containments for each screen
    int num = numScreens();
    for (int i = 0; i < num; ++i) {
        checkScreen(i, signalWhenExists);
    }
}

void DesktopCorona::checkScreen(int screen, bool signalWhenExists)
{
    // signalWhenExists is there to allow PlasmaApp to know when to create views
    // it does this only on containment addition, but in the case of a screen being
    // added and the containment already existing for that screen, no signal is emitted
    // and so PlasmaApp does not know that it needs to create a view for it. to avoid
    // taking care of that case in PlasmaApp (which would duplicate some of the code below,
    // DesktopCorona will, when signalWhenExists is true, emit a containmentAdded signal
    // even if the containment actually existed prior to this method being called.
    //
    //note: hte signal actually triggers view creation only for panels, atm.
    //desktop views are created in response to containment's screenChanged signal instead, which is
    //buggy (sometimes the containment thinks it's already on the screen, so no view is created)

    //TODO: restore activities
    //Activity *currentActivity = activity(m_activityController->currentActivity());
    //ensure the desktop(s) have a containment and view
    checkDesktop(/*currentActivity,*/ signalWhenExists, screen, 0);


    //ensure the panels get views too
    if (signalWhenExists) {
        foreach (Plasma::Containment * c, containments()) {
            if (c->screen() != screen) {
                continue;
            }

            Plasma::Containment::Type t = c->containmentType();
            if (t == Plasma::Containment::PanelContainment ||
                t == Plasma::Containment::CustomPanelContainment) {
                emit containmentAdded(c);
            }
        }
    }
}

void DesktopCorona::checkDesktop(/*Activity *activity,*/ bool signalWhenExists, int screen, int desktop)
{
    Plasma::Containment *c = /*activity->*/containmentForScreen(screen, desktop);

    //TODO: remove following when activities are restored
    if (!c) {
        c = addContainment("desktop");
    }

    if (!c) {
        return;
    }

    c->setScreen(screen, desktop);
    c->flushPendingConstraintsEvents();
    requestConfigSync();

    if (signalWhenExists) {
        emit containmentAdded(c);
    }
}

int DesktopCorona::numScreens() const
{
    return m_desktopWidget->screenCount();
}

QRect DesktopCorona::screenGeometry(int id) const
{
    return m_desktopWidget->screenGeometry(id);
}

QRegion DesktopCorona::availableScreenRegion(int id) const
{
    return m_desktopWidget->availableGeometry(id);
}

QRect DesktopCorona::availableScreenRect(int id) const
{
    return m_desktopWidget->availableGeometry(id);
}



///// SLOTS

void DesktopCorona::screenCountChanged(int newCount)
{
    qDebug() << "New screen count" << newCount;
    checkViews();
}

void DesktopCorona::screenResized(int screen)
{
    qDebug() << "Screen resized" << screen;
}

void DesktopCorona::workAreaResized(int screen)
{
    qDebug() << "Work area resized" << screen;
}

void DesktopCorona::checkViews()
{
    if (m_views.count() == m_desktopWidget->screenCount()) {
        return;
    } else if (m_views.count() < m_desktopWidget->screenCount()) {
        for (int i = m_views.count(); i < m_desktopWidget->screenCount(); ++i) {
            View *view = new View;
            
            m_views << view;
        }
    } else {
        for (int i = m_desktopWidget->screenCount(); i < m_views.count(); ++i) {
            View *view = m_views.last();
            view->deleteLater();
            m_views.pop_back();
        }
    }

    //check every containment is in proper view
    for (int i = 0; i < m_desktopWidget->screenCount(); ++i) {
        
    }
}

#include "desktopcorona.moc"


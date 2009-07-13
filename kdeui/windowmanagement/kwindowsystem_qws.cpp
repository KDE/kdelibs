/*
    This file is part of the KDE libraries
    Copyright (C) 2007 Laurent Montel (montel@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kwindowsystem.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kuniqueapplication.h>
#include <kxerrorhandler.h>
#include <QtGui/QBitmap>
#include <QDesktopWidget>
#include <QtGui/QDialog>
#include <QtDBus/QtDBus>
#include <kdebug.h>

class KWindowSystemStaticContainer {
public:
    KWindowSystemStaticContainer() : d(0) {}
    KWindowSystem kwm;
    KWindowSystemPrivate* d;
};

K_GLOBAL_STATIC(KWindowSystemStaticContainer, g_kwmInstanceContainer)

KWindowSystem* KWindowSystem::self()
{
    return &(g_kwmInstanceContainer->kwm);
}

int KWindowSystem::currentDesktop()
{
    return 1;
}

int KWindowSystem::numberOfDesktops()
{
    return 1;
}

void KWindowSystem::setCurrentDesktop( int desktop )
{
    kDebug() << "KWindowSystem::setCurrentDesktop( int desktop ) isn't yet implemented!";
    //TODO
}

void KWindowSystem::setOnAllDesktops( WId win, bool b )
{
     kDebug() << "KWindowSystem::setOnAllDesktops( WId win, bool b ) isn't yet implemented!";
     //TODO
}

void KWindowSystem::setOnDesktop( WId win, int desktop )
{
     //TODO
     kDebug() << "KWindowSystem::setOnDesktop( WId win, int desktop ) isn't yet implemented!";
}

WId KWindowSystem::activeWindow()
{
    //return something
    kDebug() << "WId KWindowSystem::activeWindow()   isn't yet implemented!";
    return 0;
}

void KWindowSystem::activateWindow( WId win, long time )
{
    //TODO
    kDebug() << "KWindowSystem::activateWindow( WId win, long time )isn't yet implemented!";
}

void KWindowSystem::forceActiveWindow( WId win, long time )
{
    //TODO
    kDebug() << "KWindowSystem::forceActiveWindow( WId win, long time ) isn't yet implemented!";
}

void KWindowSystem::demandAttention( WId win, bool set )
{
     //TODO
     kDebug() << "KWindowSystem::demandAttention( WId win, bool set ) isn't yet implemented!";
}

QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale )
{
    kDebug() << "QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale ) isn't yet implemented!";
    return QPixmap();
}


QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale, int flags )
{
    kDebug() << "QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale, int flags ) isn't yet implemented!";
    return QPixmap();
}

void KWindowSystem::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon )
{
    //TODO
    kDebug() << "KWindowSystem::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon ) isn't yet implemented!";
}


void KWindowSystem::setState( WId win, unsigned long state )
{
   //TODO
   kDebug() << "KWindowSystem::setState( WId win, unsigned long state ) isn't yet implemented!";
}

void KWindowSystem::clearState( WId win, unsigned long state )
{
    //TODO
    kDebug() << "KWindowSystem::clearState( WId win, unsigned long state ) isn't yet implemented!";
}

void KWindowSystem::minimizeWindow( WId win, bool animation)
{
     //TODO
     kDebug() << "KWindowSystem::minimizeWindow( WId win, bool animation) isn't yet implemented!";
}

void KWindowSystem::unminimizeWindow( WId win, bool animation )
{
     //TODO
     kDebug() << "KWindowSystem::unminimizeWindow( WId win, bool animation ) isn't yet implemented!";
}

void KWindowSystem::raiseWindow( WId win )
{
     //TODO
     kDebug() << "KWindowSystem::raiseWindow( WId win ) isn't yet implemented!";
}

void KWindowSystem::lowerWindow( WId win )
{
     //TODO
     kDebug() << "KWindowSystem::lowerWindow( WId win ) isn't yet implemented!";
}

bool KWindowSystem::compositingActive()
{
    return false;
}

QRect KWindowSystem::workArea( int desktop )
{
    //TODO
    kDebug() << "QRect KWindowSystem::workArea( int desktop ) isn't yet implemented!";
    return QRect();
}

QRect KWindowSystem::workArea( const QList<WId>& exclude, int desktop )
{
    //TODO
    kDebug() << "QRect KWindowSystem::workArea( const QList<WId>& exclude, int desktop ) isn't yet implemented!";
    return QRect();
}

QString KWindowSystem::desktopName( int desktop )
{
    return i18n("Desktop %1",  desktop );
}

void KWindowSystem::setDesktopName( int desktop, const QString& name )
{
     kDebug() << "KWindowSystem::setDesktopName( int desktop, const QString& name ) isn't yet implemented!";
    //TODO
}

bool KWindowSystem::showingDesktop()
{
    return false;
}

void KWindowSystem::setUserTime( WId win, long time )
{
    kDebug() << "KWindowSystem::setUserTime( WId win, long time ) isn't yet implemented!";
    //TODO
}

bool KWindowSystem::icccmCompliantMappingState()
{
    return false;
}

void KWindowSystem::connectNotify( const char* signal )
{
  kDebug() << "connectNotify( const char* signal )  isn't yet implemented!";
//TODO
}

void KWindowSystem::setExtendedStrut( WId win, int left_width, int left_start, int left_end,
        int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
        int bottom_width, int bottom_start, int bottom_end )
{
  kDebug() << "KWindowSystem::setExtendedStrut isn't yet implemented!";
  //TODO
}
void KWindowSystem::setStrut( WId win, int left, int right, int top, int bottom )
{
  kDebug() << "KWindowSystem::setStrut isn't yet implemented!";
  //TODO
}

QString KWindowSystem::readNameProperty( WId window, unsigned long atom )
{
  //TODO
  kDebug() << "QString KWindowSystem::readNameProperty( WId window, unsigned long atom ) isn't yet implemented!";
  return QString();
}

void KWindowSystem::doNotManage( const QString& title )
{
  //TODO
  kDebug() << "KWindowSystem::doNotManage( const QString& title ) isn't yet implemented!";
}

QList<WId> KWindowSystem::stackingOrder()
{
  //TODO
  QList<WId> lst;
  kDebug() << "QList<WId> KWindowSystem::stackingOrder() isn't yet implemented!";
  return lst;
}

const QList<WId>& KWindowSystem::windows()
{
  //TODO
  static QList<WId> lst;
  kDebug() << "const QList<WId>& KWindowSystem::windows()  isn't yet implemented!";
  return lst;
}

void KWindowSystem::setType( WId win, NET::WindowType windowType )
{
 //TODO
 kDebug() << "setType( WId win, NET::WindowType windowType ) isn't yet implemented!";
}

void KWindowSystem::setMainWindow( QWidget* subwindow, WId id )
{
 //TODO
 kDebug(240) << "KWindowSystem::setMainWindow( QWidget*, WId ) isn't yet implemented!";
}

void KWindowSystem::allowExternalProcessWindowActivation( int pid )
{
    // TODO
    kDebug() << "KWindowSystem::allowExternalProcessWindowActivation( int pid ) isn't yet implemented!";
}

#include "kwindowsystem.moc"


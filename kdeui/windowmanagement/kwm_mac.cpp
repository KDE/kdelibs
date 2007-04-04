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

#include "kwm.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kuniqueapplication.h>
#include <kxerrorhandler.h>
#include <qbitmap.h>
#include <QDesktopWidget>
#include <qdialog.h>
#include <QtDBus/QtDBus>
#include <kdebug.h>

int KWM::currentDesktop()
{
    return 1;
}

int KWM::numberOfDesktops()
{
    return 1;
}

void KWM::setCurrentDesktop( int desktop )
{
    kDebug() << "KWM::setCurrentDesktop( int desktop ) isn't yet implemented!" << endl;
    //TODO
}

void KWM::setOnAllDesktops( WId win, bool b )
{
     kDebug() << "KWM::setOnAllDesktops( WId win, bool b ) isn't yet implemented!" << endl;
     //TODO
}

void KWM::setOnDesktop( WId win, int desktop )
{
     //TODO
     kDebug() << "KWM::setOnDesktop( WId win, int desktop ) isn't yet implemented!" << endl;
}

int KWM::numberOfViewports(int desktop)
{ 
    return 1;
}

int KWM::currentViewport(int desktop)
{
    return 1;
}

void KWM::setCurrentDesktopViewport( int desktop, QPoint viewport )
{
    //TODO
    kDebug() << "KWM::setCurrentDesktopViewport( int desktop, QPoint viewport )  isn't yet implemented!" << endl;
}

WId KWM::activeWindow()
{
    //return something
    kDebug() << "WId KWM::activeWindow()   isn't yet implemented!" << endl;
}

void KWM::activateWindow( WId win, long time )
{
    //TODO
    kDebug() << "KWM::activateWindow( WId win, long time )isn't yet implemented!" << endl;
}

void KWM::forceActiveWindow( WId win, long time )
{
    //TODO
    kDebug() << "KWM::forceActiveWindow( WId win, long time ) isn't yet implemented!" << endl;
}

void KWM::demandAttention( WId win, bool set )
{
     //TODO
     kDebug() << "KWM::demandAttention( WId win, bool set ) isn't yet implemented!" << endl;
}

QPixmap KWM::icon( WId win, int width, int height, bool scale )
{
    kDebug() << "QPixmap KWM::icon( WId win, int width, int height, bool scale ) isn't yet implemented!" << endl;
    return QPixmap();
}


QPixmap KWM::icon( WId win, int width, int height, bool scale, int flags )
{
    kDebug() << "QPixmap KWM::icon( WId win, int width, int height, bool scale, int flags ) isn't yet implemented!" << endl;
    return QPixmap();
}

void KWM::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon )
{
    //TODO
    kDebug() << "KWM::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon ) isn't yet implemented!" << endl;
}


void KWM::setState( WId win, unsigned long state )
{
   //TODO
   kDebug() << "KWM::setState( WId win, unsigned long state ) isn't yet implemented!" << endl;
}

void KWM::clearState( WId win, unsigned long state )
{
    //TODO
    kDebug() << "KWM::clearState( WId win, unsigned long state ) isn't yet implemented!" << endl;
}

void KWM::minimizeWindow( WId win, bool animation)
{
     //TODO
     kDebug() << "KWM::minimizeWindow( WId win, bool animation) isn't yet implemented!" << endl;
}

void KWM::unminimizeWindow( WId win, bool animation )
{
     //TODO
     kDebug() << "KWM::unminimizeWindow( WId win, bool animation ) isn't yet implemented!" << endl;
}

void KWM::raiseWindow( WId win )
{
     //TODO
     kDebug() << "KWM::raiseWindow( WId win ) isn't yet implemented!" << endl;
}

void KWM::lowerWindow( WId win )
{
     //TODO
     kDebug() << "KWM::lowerWindow( WId win ) isn't yet implemented!" << endl;
}

bool KWM::compositingActive()
{
    return false;
}

QRect KWM::workArea( int desktop )
{
    //TODO 
    kDebug() << "QRect KWM::workArea( int desktop ) isn't yet implemented!" << endl;
    return QRect();
}

QRect KWM::workArea( const QList<WId>& exclude, int desktop )
{
    //TODO
    kDebug() << "QRect KWM::workArea( const QList<WId>& exclude, int desktop ) isn't yet implemented!" << endl;
    return QRect();
}

QString KWM::desktopName( int desktop )
{
    return i18n("Desktop %1",  desktop );
}

void KWM::setDesktopName( int desktop, const QString& name )
{
     kDebug() << "KWM::setDesktopName( int desktop, const QString& name ) isn't yet implemented!" << endl;
    //TODO
}

bool KWM::showingDesktop()
{
    return false;
}

void KWM::setUserTime( WId win, long time )
{
    kDebug() << "KWM::setUserTime( WId win, long time ) isn't yet implemented!" << endl;
    //TODO
}

bool KWM::icccmCompliantMappingState()
{
    return false;
}

void KWM::connectNotify( const char* signal )
{
  kDebug() << "connectNotify( const char* signal )  isn't yet implemented!" << endl;
//TODO
}

void KWM::setExtendedStrut( WId win, int left_width, int left_start, int left_end,
        int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
        int bottom_width, int bottom_start, int bottom_end )
{
  kDebug() << "KWM::setExtendedStrut isn't yet implemented!" << endl;
  //TODO
}
void KWM::setStrut( WId win, int left, int right, int top, int bottom )
{
  kDebug() << "KWM::setStrut isn't yet implemented!" << endl;
  //TODO
}

QString KWM::readNameProperty( WId window, unsigned long atom )
{
  //TODO
  kDebug() << "QString KWM::readNameProperty( WId window, unsigned long atom ) isn't yet implemented!" << endl;
  return QString();
}

void KWM::doNotManage( const QString& title )
{
  //TODO
  kDebug() << "KWM::doNotManage( const QString& title ) isn't yet implemented!" << endl;
}

const QList<WId>& KWM::stackingOrder()
{
  //TODO
  QList<WId> lst;
  kDebug() << "const QList<WId>& KWM::stackingOrder() isn't yet implemented!" << endl;
  return lst;
}

const QList<WId>& KWM::windows()
{
  //TODO
  QList<WId> lst;
  kDebug() << "const QList<WId>& KWM::windows()  isn't yet implemented!" << endl;
  return lst;
}

void KWM::setType( WId win, NET::WindowType windowType )
{
 //TODO
 kDebug() << "setType( WId win, NET::WindowType windowType ) isn't yet implemented!" << endl;
}


#include "kwm.moc"


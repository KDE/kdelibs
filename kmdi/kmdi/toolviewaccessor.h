/* This file is part of the KDE libraries
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

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

#ifndef _KMDI_TOOLVIEWACCESSOR_H_
#define _KMDI_TOOLVIEWACCESSOR_H_

#include <qwidget.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qapplication.h>
#include <qdatetime.h>

#include <kdockwidget.h>

namespace KMDIPrivate {
   class GUIClient;
   class ToolViewAccessorPrivate;
}

namespace KMDI {

class MainWindow;

class ToolViewAccessor : public QObject
{
   Q_OBJECT

   friend class KMDI::MainWindow;
   friend class KMDIPrivate::GUIClient;

private:
   /**
   * Internally used by KMDI::MainWindow to store a temporary information that the method
   * activate() is unnecessary and that it can by escaped.
   * This saves from unnecessary calls when activate is called directly.
   */
   bool m_bInterruptActivation;
   /**
   * Internally used to prevent cycles between KMDI::MainWindow::activateView() and KMdiChildView::activate().
   */
   bool m_bMainframesActivateViewIsPending;
   /**
   *
   */
   bool m_bFocusInEventIsPending;

private:
  ToolViewAccessor( KMDI::MainWindow *parent , QWidget *widgetToWrap, const QString& tabToolTip = 0, const QString& tabCaption = 0);
  ToolViewAccessor( KMDI::MainWindow *parent);
public:
  ~ToolViewAccessor();
  QWidget *wrapperWidget();
  QWidget *wrappedWidget();
  void place(KDockWidget::DockPosition pos = KDockWidget::DockNone, QWidget* pTargetWnd = 0L,int percent = 50);
  void placeAndShow(KDockWidget::DockPosition pos = KDockWidget::DockNone, QWidget* pTargetWnd = 0L,int percent = 50);
  void show();
public slots:
  void setWidgetToWrap(QWidget* widgetToWrap, const QString& tabToolTip = 0, const QString& tabCaption = 0);
  void hide();
private:
   KMDIPrivate::ToolViewAccessorPrivate *d;
   KMDI::MainWindow *mdiMainFrm;

protected:
  bool eventFilter(QObject *o, QEvent *e);
};

}

#endif //_KMDITOOLVIEWACCESSOR_H_

// kate: space-indent on; indent-width 2; replace-tabs on;

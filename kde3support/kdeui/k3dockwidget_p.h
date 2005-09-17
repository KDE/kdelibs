/* This file is part of the KDE libraries
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/***********************************************************************
************************************************************************
 IMPORTANT - IMPORTANT - IMPORTANT - IMPORTANT - IMPORTANT - IMPORTANT
************************************************************************
************************************************************************

THIS IS ___NOT___ PART OF THE PUBLIC API YET. DON'T USE IT IN YOUR
APPLICATIONS,SINCE IT'S MOST PROBABLY ___NOT___ GOING TO STAY BINARY
COMPATIBLE. THIS HEADER IS ONLY INSTALLED, BECAUSE IT IS NEEDED IN
 KDE 3.1'S KATE APPLICATON

************************************************************************
************************************************************************
 IMPORTANT - IMPORTANT - IMPORTANT - IMPORTANT - IMPORTANT - IMPORTANT
************************************************************************
***********************************************************************/

#ifndef KDOCKWIDGET_P_H
#define KDOCKWIDGET_P_H

#include <k3dockwidget.h>
#include <qstringlist.h>
// Add some describing comment !!

class KDE3SUPPORT_EXPORT K3DockContainer
{
public:
  K3DockContainer();
  virtual ~K3DockContainer();
  virtual K3DockWidget *parentDockWidget();
  virtual void insertWidget (K3DockWidget *, QPixmap, const QString &, int &);
  virtual void showWidget(K3DockWidget *);
  virtual void removeWidget(K3DockWidget*);
  virtual void undockWidget(K3DockWidget*);
  virtual void save(KConfig *cfg,const QString& group_or_prefix);
  virtual void save(QDomElement& dockElement);
  virtual void load(KConfig *cfg,const QString& group_or_prefix);
  virtual void load(QDomElement& dockElement);
  virtual void setToolTip (K3DockWidget *, QString &);
  virtual void  setPixmap(K3DockWidget*,const QPixmap&);
  QStringList containedWidgets() const;
  virtual bool dockDragEnter(K3DockWidget* dockWidget, QMouseEvent *event);
  virtual bool dockDragMove(K3DockWidget* dockWidget, QMouseEvent *event);
  virtual bool dockDragLeave(K3DockWidget* dockWidget, QMouseEvent *event);
protected:
  friend class K3DockManager;
  friend class K3DockSplitter;
  void prepareSave(QStringList &names);
  void activateOverlapMode(int nonOverlapSize);
  void deactivateOverlapMode();
  bool isOverlapMode();
private:

	struct ListItem {
		struct ListItem *prev;
		struct ListItem *next;
		char *data;
	};



  struct ListItem *m_childrenListBegin;
  struct ListItem *m_childrenListEnd;

  class K3DockContainerPrivate;
  K3DockContainerPrivate *d;
  bool m_overlapMode;
  int m_nonOverlapSize;
};

#endif

/* This file is part of the KDE libraries
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org

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

#include <kdockwidget.h>

// Add some describing comment !!

class EXPORT_DOCKCLASS KDockContainer
{
public:
  KDockContainer();
  virtual ~KDockContainer();
  virtual KDockWidget *parentDockWidget();
  virtual void insertWidget (KDockWidget *, QPixmap, const QString &, int &);
  virtual void removeWidget(KDockWidget*);
  virtual void undockWidget(KDockWidget*);
  virtual void save(KConfig *cfg);
  virtual void load(KConfig *cfg);
  virtual void setToolTip (KDockWidget *, QString &);
protected:
  friend class KDockManager;
  void prepareSave(QStringList &names);
private:

	struct ListItem {
		struct ListItem *prev;
		struct ListItem *next;
		char *data;
	};



  struct ListItem *m_childrenListBegin;
  struct ListItem *m_childrenListEnd;

  class KDockContainerPrivate;
  KDockContainerPrivate *d;
};

#endif

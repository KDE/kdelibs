/***************************************************************************
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDOCKWIDGETTEST_H
#define KDOCKWIDGETTEST_H

#include <kdockwidget.h>
#include <kdockwidget_p.h>
#include <qtabwidget.h>
#include <qlabel.h>
class QPushButton;

class DockApplication : public KDockMainWindow
{Q_OBJECT
public:
  DockApplication( const char* name );
  ~DockApplication();

public slots:
  void rConfig();
  void wConfig();
  void cap();
  void greenMain();
  void blueMain();
  void nullMain();

protected:
  void initMenuBar();
  void initToolBars();
  void initStatusBar();

private:
  void updateButton();
  KDockWidget* dock;
  KDockWidget* dock1;
	KDockWidget* dock4;
	KDockWidget* dock5;
	KDockWidget* dock6;

  QWidget* mainW;
  QWidget* l;
  QPushButton* m_bname;
};

class CTW:public QTabWidget,public KDockContainer
{
        Q_OBJECT
public:
        CTW(QWidget *parent):QTabWidget(parent,"MyDockContainer"),KDockContainer(){insertTab(new QLabel("BLAH",this),"BLUP");}
        virtual ~CTW(){;}
        KDockWidget *parentDockWidget(){return ((KDockWidget*)parent());}
        void insertWidget (KDockWidget *w, QPixmap, const QString &, int &){qDebug("widget inserted"); insertTab(w,"NO");}
        void setToolTip (KDockWidget *, QString &){qDebug("Tooltip set");}
};


#endif



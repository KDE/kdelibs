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

#include <k3dockwidget.h>
#include <k3dockwidget_p.h>
#include <qtabwidget.h>
#include <qlabel.h>
class QPushButton;

class DockApplication : public K3DockMainWindow
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
  void gSetPix1();
  void gSetPix2();

protected:
  void initMenuBar();
  void initToolBars();
  void initStatusBar();

private:
  void updateButton();
  K3DockWidget* dock;
  K3DockWidget* dock1;
  K3DockWidget* dock4;
  K3DockWidget* dock5;
  K3DockWidget* dock6;

  QWidget* mainW;
  QWidget* l;
  QPushButton* m_bname;
};

class CTW:public QTabWidget,public K3DockContainer
{
        Q_OBJECT
public:
        CTW(QWidget *parent):QTabWidget(parent,"MyDockContainer"),K3DockContainer(){insertTab(new QLabel("BLAH",this),"BLUP");}
        virtual ~CTW(){;}
        K3DockWidget *parentDockWidget(){return ((K3DockWidget*)parent());}
        void insertWidget (K3DockWidget *w, QPixmap, const QString &, int &){qDebug("widget inserted"); insertTab(w,"NO");}
        void setToolTip (K3DockWidget *, QString &){qDebug("Tooltip set");}
};


#endif



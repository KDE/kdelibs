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
  QWidget* mainW;
  QWidget* l;
  QPushButton* m_bname;
};

#endif



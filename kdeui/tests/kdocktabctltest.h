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

#ifndef KDOCKTABCTLTEST_H
#define KDOCKTABCTLTEST_H

#include <qstring.h>
#include <kmainwindow.h>
#include <kdocktabctl.h>

class QMultiLineEdit;

class application : public KMainWindow
{Q_OBJECT
public:
  application( const char* name );
  ~application();

protected:
  void initMenuBar();

protected slots:
  void slotAddTab();
  void slotDelTab();
  void slotDisTab();
  void slotEnableLastTab();
  void slotShowIcon();
  void slotHideIcon();
  void slotTAB_RIGHT();
  void slotTAB_TOP();

private:
  KDockTabCtl* tab;
  int lastTabDisabled;
  QMultiLineEdit* edit;
};

#endif

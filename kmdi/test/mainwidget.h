/***************************************************************************
                          mainwidget.h  -  description
                             -------------------
    begin                : Mon Nov 8 1999
    copyright            : (C) 1999 by Falk Brettschneider
    email                : falkbr@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <kmdimainfrm.h>

/**
  *@author Falk Brettschneider
  */

class MainWidget : public KMdiMainFrm
{
   Q_OBJECT
public:
	MainWidget();
   void initMenu();

protected: // Protected methods
   virtual void resizeEvent( QResizeEvent *pRSE);
};

#endif

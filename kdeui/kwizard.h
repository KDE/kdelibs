/*  This file is part of the KDE Libraries
    Copyright (C) 1999 Harri Porten (porten@kde.org)

    Replacement for KWizard from KDE 1.x.
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KWIZARD_H
#define __KWIZARD_H

#include <qwizard.h>

/**
* General-purpose multi-page dialog.
*
* KWizard is a class for a multi-page dialog. The user can navigate trough
* the pages with "Next" and "Back" buttons and is optionally offered "Finish",
* "Cancel" and "Help" buttons. A wizard comes in handy for tutorials or
* configuration dialogs with sequential steps.  
* 
* KWizard is just a wrapper for Qt's @ref QWizard class. See the
* @ref QWizard documentation for a detailed description of available
* functions. The only added functionality is a KDE conformant translation
* mechanism for the built-in buttons.
*
* @author Harri Porten <porten@kde.org>
* @version 0.3 
*/
class KWizard : public QWizard
{
  Q_OBJECT
public:
/**
* Constructor
*/
  KWizard(QWidget *parent = 0, const char *name = 0, bool modal = false, WFlags f = 0);
/**
* Destructor
*/
  ~KWizard() {}
};

#endif // __KWIZARD_H



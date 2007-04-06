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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/  

#ifndef K3WIZARD_H
#define K3WIZARD_H

#include <kde3support_export.h>
#include <Qt3Support/Q3Wizard>

/**
* General-purpose multi-page dialog.
*
* KWizard is a class for a multi-page dialog. The user can navigate trough
* the pages with "Next" and "Back" buttons and is optionally offered "Finish",
* "Cancel" and "Help" buttons. A wizard comes in handy for tutorials or
* configuration dialogs with sequential steps.  
* 
* KWizard is just a wrapper for Qt's QWizard class. See the
* QWizard documentation for a detailed description of available
* functions. The only added functionality is a KDE conformant translation
* mechanism for the built-in buttons.
*
* @author Harri Porten <porten@kde.org>
* @version 0.3 
*/
class KDE3SUPPORT_EXPORT K3Wizard : public Q3Wizard
{
  Q_OBJECT
public:
/**
* Constructor
*/
  K3Wizard(QWidget *parent = 0, const char *name = 0, bool modal = false, Qt::WFlags f = 0);
/**
* Destructor
*/
  ~K3Wizard() {}
};

#endif // K3WIZARD_H



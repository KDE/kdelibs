#ifndef DISTRIBUTIONLISTEDITOR_H
#define DISTRIBUTIONLISTEDITOR_H

/*
    This file is part of libkabc.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdialogbase.h>

class QWidget;

namespace KABC {

class AddressBook;
class DistributionListEditorPrivate;
  
class DistributionListEditor : public KDialogBase
{
  Q_OBJECT
  
  public:
    DistributionListEditor(AddressBook *book, QWidget *parent, 
                           const char *name = 0);
    virtual ~DistributionListEditor();
    
  protected slots:
    void add();
    void remove();
    void rename();
    void itemSelected(int id);
    void modified();
    
    virtual void slotApply();
    virtual void slotOk();
    
  private:
    void initGUI();
    
    DistributionListEditorPrivate *d;
};

}

#endif

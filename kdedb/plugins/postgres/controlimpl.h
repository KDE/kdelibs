/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
#ifndef __CONTROLIMPL_H
#define __CONTROLIMPL_H

#include <kdb/control.h>

#include <kdialogbase.h>

class QListView;
class QListViewItem;
class QLineEdit;
class QPushButton;

class PGControlDialog: public KDialogBase
{
    Q_OBJECT
 public:
    PGControlDialog(KConfigBase *conf, QWidget * parent);
    ~PGControlDialog() {};

 protected slots:
        
    void slotNew();
    void slotDelete();
    void slotHostChanged(const QString &);
    void slotDbChanged(const QString &);
    void slotItemSelected(QListViewItem *);
    void slotOk();
    
 private:
    KConfigBase *m_conf;
    QListView *m_lst;
    QLineEdit *m_host;
    QLineEdit *m_db;
    QPushButton *m_new;
    QPushButton *m_delete;
    
};


class ControlImpl : public KDB::Control
{
    Q_OBJECT

public:

    ControlImpl(const char * name);
    ~ControlImpl();

    virtual bool showDialog(KConfigBase *conf, QWidget *parent, const char * name);

};


#endif






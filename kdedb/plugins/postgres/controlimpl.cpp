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

#include "controlimpl.h"

#include <kdialogbase.h>
#include <kconfig.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>

#include "controlimpl.moc"

class PGControlDialog: public KDialogBase
{
public:
    PGControlDialog(KConfigBase *conf, QWidget * parent);
    ~PGControlDialog() {};

private:
    KConfigBase *m_conf;
    QListView *m_lst;
};

PGControlDialog::PGControlDialog(KConfigBase *conf, QWidget *parent)
    : KDialogBase(parent, "PGControl", true, i18n("Configure Postgres plugin"), Ok|Cancel, Ok, true),
      m_conf(conf)
{
    QFrame *main = makeMainWidget();
    QGridLayout *main_layout = new QGridLayout(main);

    m_lst = new QListView(main);
    m_lst->addColumn(i18n("Host"));
    m_lst->addColumn(i18n("Primary Database"));

    main_layout->addWidget(m_lst,0,0);
    
}

ControlImpl::ControlImpl(const char *name)
    : KDB::Control(name)
{
}

ControlImpl::~ControlImpl()
{
}

bool 
ControlImpl::showDialog(KConfigBase *conf, QWidget *parent, const char * /*name*/)
{
    PGControlDialog *dlg = new PGControlDialog(conf, parent);
    if (dlg->exec() == QDialog::Accepted)
        return true;
    return false;
}

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
#include <qpushbutton.h>
#include <qlineedit.h>

#include "controlimpl.moc"

/*------------------------------------------------------------------------------
 * PGControlDialog implementation
 *------------------------------------------------------------------------------*/

PGControlDialog::PGControlDialog(KConfigBase *conf, QWidget *parent)
    : KDialogBase(parent, "PGControl", true, i18n("Configure Postgres plugin"), Ok|Cancel, Ok, true),
      m_conf(conf)
{
    QFrame *main = makeMainWidget();
    QGridLayout *main_layout = new QGridLayout(main);
    main_layout->setSpacing(5);
    main_layout->setMargin(10);
    
    m_lst = new QListView(main);
    m_lst->addColumn(i18n("Host"));
    m_lst->addColumn(i18n("Primary Database"));

    main_layout->addMultiCellWidget(m_lst,0,1,0,2);

    QFrame *sep = new QFrame(main);
    sep->setFrameStyle(QFrame::HLine|QFrame::Sunken);
    main_layout->addMultiCellWidget(sep,2,2,0,2);
    
    QLabel *l = new QLabel(main);
    l->setText(i18n("Host"));
    main_layout->addWidget(l,3,0);

    l = new QLabel(main);
    l->setText(i18n("Database"));
    main_layout->addWidget(l,3,1);

    m_host = new QLineEdit(main,"m_host");
    m_host->setEnabled(false);
    main_layout->addWidget(m_host,4,0);
    
    m_db = new QLineEdit(main,"m_db");
    m_db->setEnabled(false);
    main_layout->addWidget(m_db,4,1);

    m_new = new QPushButton(main);
    m_new->setText(i18n("&New"));
    main_layout->addWidget(m_new,4,2);           

    m_delete = new QPushButton(main);
    m_delete->setText(i18n("&Delete"));
    main_layout->addWidget(m_delete,3,2);           

    connect(m_host, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotHostChanged(const QString&)));
    connect(m_db, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotDbChanged(const QString&)));

    connect(m_new, SIGNAL(clicked()),
            this, SLOT(slotNew()));
    connect(m_delete, SIGNAL(clicked()),
            this, SLOT(slotDelete()));

    connect(m_lst, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(slotItemSelected(QListViewItem *)));

    KConfigGroupSaver(m_conf, "KDBPostgresPlugin");
    m_conf->setGroup("KDBPostgresPlugin");

    int numHost = m_conf->readNumEntry("NumHosts",0);

    while (numHost--) {
        QString pref = QString("%1_").arg(numHost);

        QString host = m_conf->readEntry(pref + "host");
        QString db = m_conf->readEntry(pref + "db");
        QListViewItem *itm = new QListViewItem(m_lst,host, db);
        m_lst->setCurrentItem(itm);
        m_lst->ensureItemVisible(itm);
    }
}

void 
PGControlDialog::slotNew()
{
    m_host->setEnabled(true);
    m_host->setText("localhost");
    m_db->setEnabled(true);
    m_db->setText("template1");

    QListViewItem *itm = new QListViewItem(m_lst,m_host->text(), m_db->text());
    m_lst->setCurrentItem(itm);
    m_lst->ensureItemVisible(itm);
}

void 
PGControlDialog::slotDelete()
{
    QListViewItem *itm = m_lst->currentItem();
    if (itm)
        delete itm;
}

void 
PGControlDialog::slotHostChanged(const QString &newVal)
{
    QListViewItem *itm = m_lst->currentItem();
    if (itm)
        itm->setText(0, newVal);
}

void 
PGControlDialog::slotDbChanged(const QString &newVal)
{
    QListViewItem *itm = m_lst->currentItem();
    if (itm)
        itm->setText(1, newVal);
}

void 
PGControlDialog::slotItemSelected(QListViewItem *itm)
{
    m_host->setText(itm->text(0));
    m_host->setEnabled(true);
    m_db->setText(itm->text(1));
    m_db->setEnabled(true);
    m_delete->setEnabled(true);
}


void
PGControlDialog::slotOk()
{
    KConfigGroupSaver(m_conf, "KDBPostgresPlugin");
    m_conf->setGroup("KDBPostgresPlugin");

    int numHost = 0;

    QListViewItem *it = m_lst->firstChild();

    while (it) {
        QString prefix = QString("%1_").arg(numHost++);

        m_conf->writeEntry(prefix + "host", it->text(0));
        m_conf->writeEntry(prefix + "db", it->text(1));
        it = it->nextSibling();
    }
    m_conf->writeEntry("NumHosts",numHost);

    KDialogBase::slotOk();
}


/*------------------------------------------------------------------------------
 * ControlImpl implementation
 *------------------------------------------------------------------------------*/
 
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


/* This file is part of the KDE libraries
    Copyright (C) 2006 Olivier Goffart <ogoffart at kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kassistantdialog.h"

#include <kstandardguiitem.h>
#include <klocale.h>
#include <kdebug.h>

#include <QHash>

class KAssistantDialog::Private
{
    public:
        Private(KAssistantDialog *q)
            : q(q)
        {
        }

        KAssistantDialog *q;
        QHash<KPageWidgetItem*, bool> valid;
        QHash<KPageWidgetItem*, bool> appropriate;
        KPageWidgetModel *pageModel;

        void init();
        void _k_slotUpdateButtons();

        QModelIndex getNext(QModelIndex nextIndex)
        {
            QModelIndex currentIndex;
            do {
                currentIndex=nextIndex;
                nextIndex=currentIndex.child(0, 0);
                if (!nextIndex.isValid())
                    nextIndex=currentIndex.sibling(currentIndex.row() + 1, 0);
            } while (nextIndex.isValid() && !appropriate.value(pageModel->item(nextIndex), true));
            return nextIndex;
        }

        QModelIndex getPrevious(QModelIndex nextIndex)
        {
            QModelIndex currentIndex;
            do {
                currentIndex=nextIndex;
                nextIndex=currentIndex.sibling(currentIndex.row() - 1, 0);
                if (!nextIndex.isValid())
                    nextIndex=currentIndex.parent();
            } while (nextIndex.isValid() && !appropriate.value(pageModel->item(nextIndex), true));
            return nextIndex;
        }
};

KAssistantDialog::KAssistantDialog(QWidget * parent, Qt::WFlags flags)
    : KPageDialog(parent, flags), d(new Private(this))
{
    d->init();
    //workaround to get the page model
    KPageWidget *pagewidget=findChild<KPageWidget*>();
    Q_ASSERT(pagewidget);
    d->pageModel=static_cast<KPageWidgetModel*>(pagewidget->model());
}

KAssistantDialog::KAssistantDialog(KPageWidget *widget, QWidget *parent, Qt::WFlags flags)
    : KPageDialog(widget, parent, flags), d(new Private(this))
{
    d->init();
    d->pageModel=static_cast<KPageWidgetModel*>(widget->model());
}

KAssistantDialog::~KAssistantDialog()
{
    delete d;
}

void KAssistantDialog::Private::init()
{
    q->setButtons(KDialog::Cancel | KDialog::User1 | KDialog::User2 | KDialog::User3 | KDialog::Help);
    q->setButtonGuiItem( KDialog::User3, KStandardGuiItem::back(KStandardGuiItem::UseRTL) );
    q->setButtonText( KDialog::User2, i18nc("Opposite to Back", "Next") );
    q->setButtonText(KDialog::User1, i18n("Finish"));
    q->setButtonIcon( KDialog::User2, KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon() );
    q->setButtonIcon( KDialog::User1, KIcon("dialog-ok-apply") );
    q->setDefaultButton(KDialog::User2);
    q->setFaceType(KPageDialog::Plain);

    q->connect(q, SIGNAL(user3Clicked()), q, SLOT(back()));
    q->connect(q, SIGNAL(user2Clicked()), q, SLOT(next()));
    q->connect(q, SIGNAL(user1Clicked()), q, SLOT(accept()));

    q->connect(q, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)), q, SLOT(_k_slotUpdateButtons()));
}


void KAssistantDialog::back()
{
    QModelIndex nextIndex=d->getPrevious(d->pageModel->index(currentPage()));
    if (nextIndex.isValid())
        setCurrentPage(d->pageModel->item(nextIndex));
}

void KAssistantDialog::next()
{
    QModelIndex nextIndex=d->getNext(d->pageModel->index(currentPage()));
    if (nextIndex.isValid())
        setCurrentPage(d->pageModel->item(nextIndex));
}

void KAssistantDialog::setValid(KPageWidgetItem * page, bool enable)
{
    d->valid[page]=enable;
    if (page == currentPage())
        d->_k_slotUpdateButtons();
}

bool KAssistantDialog::isValid(KPageWidgetItem * page) const
{
    return d->valid.value(page, true);
}

void KAssistantDialog::Private::_k_slotUpdateButtons()
{
    QModelIndex currentIndex=pageModel->index(q->currentPage());
    //change the caption of the next/finish button
    QModelIndex nextIndex=getNext(currentIndex);
    q->enableButton(KDialog::User1, !nextIndex.isValid() && q->isValid(q->currentPage()));
    q->enableButton(KDialog::User2, nextIndex.isValid() && q->isValid(q->currentPage()));
    q->setDefaultButton(nextIndex.isValid() ? KDialog::User2 : KDialog::User1);
    //enable or disable the back button;
    nextIndex=getPrevious(currentIndex);
    q->enableButton(KDialog::User3, nextIndex.isValid());
}

void KAssistantDialog::showEvent(QShowEvent * event)
{
    d->_k_slotUpdateButtons(); //called because last time that function was called is when the first page was added, so the next button show "finish"
    KPageDialog::showEvent(event);
}

void KAssistantDialog::setAppropriate(KPageWidgetItem * page, bool appropriate)
{
    d->appropriate[page]=appropriate;
    d->_k_slotUpdateButtons();
}

bool KAssistantDialog::isAppropriate(KPageWidgetItem * page) const
{
    return d->appropriate.value(page, true);
}

#include "kassistantdialog.moc"

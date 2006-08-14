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

#include <kstdguiitem.h>
#include <klocale.h>
#include <kdebug.h>

#include <QHash>

class KAssistantDialogPrivate
{
    public:
        QHash<KPageWidgetItem*, bool> valid;
        QHash<KPageWidgetItem*, bool> appropriate;
        KPageWidgetModel *pageModel;

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

KAssistantDialog::KAssistantDialog(QWidget * parent, Qt::WFlags flags) : KPageDialog(parent, flags), d(new KAssistantDialogPrivate)
{
    init();
    //workaround to get the page model
    KPageWidget *pagewidget=findChild<KPageWidget*>();
    Q_ASSERT(pagewidget);
    d->pageModel=static_cast<KPageWidgetModel*>(pagewidget->model());
}

KAssistantDialog::KAssistantDialog(KPageWidget *widget, QWidget *parent, Qt::WFlags flags) : KPageDialog(widget, parent, flags), d(new KAssistantDialogPrivate)
{
    init();
    d->pageModel=static_cast<KPageWidgetModel*>(widget->model());
}

KAssistantDialog::~KAssistantDialog()
{
    delete d;
}

void KAssistantDialog::init()
{
    setButtons(Cancel | User1 | User2 | User3 | Help);
    setButtonText(User3, i18n("Back"));
    setButtonText(User2, i18n("Next"));
    setButtonText(User1, i18n("Finish"));
    setDefaultButton(User2);
    showButtonSeparator(true);
    setFaceType(Plain);

    connect(this, SIGNAL(user3Clicked()), this, SLOT(back()));
    connect(this, SIGNAL(user2Clicked()), this, SLOT(next()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(accept()));

    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)), this, SLOT(slotCurrentPageChanged()));
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
        slotCurrentPageChanged();
}

bool KAssistantDialog::isValid(KPageWidgetItem * page)
{
    return d->valid.value(page, true);
}

void KAssistantDialog::slotCurrentPageChanged()
{
    QModelIndex currentIndex=d->pageModel->index(currentPage());
    //change the caption of the next/finish button
    QModelIndex nextIndex=d->getNext(currentIndex);
    enableButton(User1, !nextIndex.isValid() && isValid(currentPage()));
    enableButton(User2, nextIndex.isValid() && isValid(currentPage()));
    setDefaultButton(nextIndex.isValid() ? User2 : User1);
    //enable or disable the back button;
    nextIndex=d->getPrevious(currentIndex);
    enableButton(User3, nextIndex.isValid());
}

void KAssistantDialog::showEvent(QShowEvent * event)
{
    slotCurrentPageChanged(); //called because last time that function was called is when the first page was added, so the next button show "finish"
    KPageDialog::showEvent(event);
}

void KAssistantDialog::setAppropriate(KPageWidgetItem * page, bool appropriate)
{
    d->appropriate[page]=appropriate;
}

bool KAssistantDialog::isAppropriate(KPageWidgetItem * page)
{
    return d->appropriate.value(page, true);
}

#include "kassistantdialog.moc"

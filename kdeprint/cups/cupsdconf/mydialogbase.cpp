/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "mydialogbase.h"

#include <qlabel.h>
#include <klistview.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qstringlist.h>

#include <kseparator.h>
#include <klocale.h>

class MyPage : public QListViewItem
{
public:
        MyPage(QListView *lv, const QString& hdr, const QString& txt, const QPixmap& icon, QWidget *w);
        MyPage(QListViewItem *item, const QString& hdr, const QString& txt, const QPixmap& icon, QWidget *w);
        ~MyPage();

        QWidget* getPage() const { return widget_; }
        const QString& getHeader() const { return header_; }

private:
        QString header_;
        QWidget *widget_;
};

MyPage::MyPage(QListView *lv, const QString& hdr, const QString& txt, const QPixmap& icon, QWidget *w)
        : QListViewItem(lv,txt), header_(hdr), widget_(w)
{
        setPixmap(0, icon);
}

MyPage::MyPage(QListViewItem *item, const QString& hdr, const QString& txt, const QPixmap& icon, QWidget *w)
        : QListViewItem(item,txt), header_(hdr), widget_(w)
{
        setPixmap(0, icon);
}

MyPage::~MyPage()
{
}

//------------------------------------------------------------------------------------------------

MyDialogBase::MyDialogBase(QWidget *parent, const char *name)
        : KDialog(parent, name, true)
{
        tree_ = new KListView(this);
        tree_->addColumn("");
        tree_->setRootIsDecorated(false);
        tree_->header()->hide();
        tree_->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
        tree_->setLineWidth(1);
        tree_->setSorting(-1);
        connect(tree_, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotItemSelected(QListViewItem*)));

        QWidget         *right_ = new QWidget(this);

        title_ = new QLabel("Title", right_);

        KSeparator* sep1 = new KSeparator( KSeparator::HLine, right_);
        sep1->setFixedHeight(5);

        stack_ = new QWidgetStack(right_);

        KSeparator* sep2 = new KSeparator( KSeparator::HLine, this);
        sep2->setFixedHeight(15);

        QPushButton     *ok_ = new QPushButton(i18n("OK"), this);
        QPushButton     *cancel_ = new QPushButton(i18n("Cancel"), this);
        QPushButton     *apply_ = new QPushButton(i18n("Apply"), this);
        QPushButton     *help_ = new QPushButton(i18n("Short Help"), this);
        connect(ok_, SIGNAL(clicked()), SLOT(slotOk()));
        connect(cancel_, SIGNAL(clicked()), SLOT(slotCancel()));
        connect(apply_, SIGNAL(clicked()), SLOT(slotApply()));
        connect(help_, SIGNAL(clicked()), SLOT(slotHelp()));
        ok_->setDefault(true);

        QVBoxLayout     *mainLayout = new QVBoxLayout(this, 10, 0);
	QHBoxLayout	*panelLayout = new QHBoxLayout(0, 0, 5);
        QHBoxLayout     *btnLayout = new QHBoxLayout(0, 0, 10);
        QVBoxLayout     *rightLayout = new QVBoxLayout(right_, 5, 0);
	mainLayout->addLayout(panelLayout, 1);
	panelLayout->addWidget(tree_, 0);
	panelLayout->addWidget(right_, 1);
        mainLayout->addWidget(sep2, 0);
        mainLayout->addLayout(btnLayout, 0);
        btnLayout->addWidget(help_, 0);
        btnLayout->addStretch(1);
        btnLayout->addWidget(ok_, 0);
        btnLayout->addWidget(apply_, 0);
        btnLayout->addWidget(cancel_, 0);
        rightLayout->addWidget(title_, 0);
        rightLayout->addWidget(sep1, 0);
        rightLayout->addWidget(stack_, 1);
}

MyDialogBase::~MyDialogBase()
{
}

void MyDialogBase::slotOk()
{
        accept();
}

void MyDialogBase::slotCancel()
{
        reject();
}

void MyDialogBase::slotApply()
{
}

void MyDialogBase::slotItemSelected(QListViewItem *item)
{
        if (!item) return;
        MyPage  *page = (MyPage*)item;
        QString txt = "<b>" + page->getHeader() + "</b>";
        title_->setText(txt);
        stack_->raiseWidget(page->getPage());
}

QListViewItem* MyDialogBase::findParent(const QStringList& path)
{
        if (path.count() == 1) return 0;
        QListViewItem   *item = tree_->firstChild();
        uint            index(0);
        while (item && index < path.count()-1)
        {
                if (item->text(0) == path[index])
                {
                        index++;
                        if (index == path.count()-1) break;
                        item = item->firstChild();
                }
                else item = item->nextSibling();
        }
        return item;
}

void MyDialogBase::addPage(const QStringList& path, const QString& header, const QPixmap& icon, QWidget *w)
{
        if (path.count() < 1)
        {
                qWarning("Unable to add page without a valid path");
                return;
        }
        MyPage  *page;
        QListViewItem   *parent = findParent(path);
        bool    first = (tree_->childCount() == 0);
        if (parent)
        {
                page = new MyPage(parent, header, path.last(), icon, w);
                parent->setOpen(true);
        }
        else
                page = new MyPage(tree_, header, path.last(), icon, w);
        w->reparent(stack_, QPoint(0,0));
        if (first) tree_->setCurrentItem(page);
        else w->hide();

	tree_->setFixedWidth(tree_->sizeHint().width());
}

void MyDialogBase::slotHelp()
{
        QWhatsThis::enterWhatsThisMode();
}
#include "mydialogbase.moc"

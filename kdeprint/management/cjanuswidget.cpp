/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "cjanuswidget.h"

#include <QStackedWidget>
#include <qlabel.h>
#include <qpainter.h>
#include <klistbox.h>
#include <qlayout.h>
#include <kseparator.h>
#include <QEvent>

class CJanusWidget::CPage
{
public:
	QWidget		*m_widget;
	QString		m_text;
	QString		m_header;
	QPixmap		m_pixmap;
	CListBoxItem	*m_item;
};

//***********************************************************************************

class CJanusWidget::CListBoxItem : public Q3ListBoxItem
{
public:
	CListBoxItem(Q3ListBox *lb, Q3ListBoxItem *after, const QPixmap& pix, const QString& text);
	int height(const Q3ListBox*) const;
	int width(const Q3ListBox*) const;

protected:
	void paint(QPainter*);

private:
	QPixmap	m_pix;
};

CJanusWidget::CListBoxItem::CListBoxItem(Q3ListBox *lb, Q3ListBoxItem *after, const QPixmap& pix, const QString& text)
: Q3ListBoxItem(lb, after), m_pix(pix)
{
	setText(text);
}

int CJanusWidget::CListBoxItem::height(const Q3ListBox *lb) const
{
	return (m_pix.height() + lb->fontMetrics().lineSpacing() + 12);
}

int CJanusWidget::CListBoxItem::width(const Q3ListBox *lb) const
{
	int	w = QMAX(lb->fontMetrics().width(text()),m_pix.width());
	return (w + 10);
}

void CJanusWidget::CListBoxItem::paint(QPainter *p)
{
	int	w1 = (listBox()->contentsWidth()-m_pix.width())/2;

	p->drawPixmap(w1,5,m_pix);
	p->drawText(0,7+m_pix.height(),listBox()->contentsWidth(),p->fontMetrics().lineSpacing(),Qt::AlignHCenter,text());
}

//***********************************************************************************

class CJanusWidget::CListBox : public KListBox
{
public:
	CListBox(QWidget *parent = 0, const char *name = 0);
	~CListBox();

	void computeWidth();

protected:
	virtual bool eventFilter(QObject*, QEvent*);
};

CJanusWidget::CListBox::CListBox(QWidget *parent, const char *name)
: KListBox(parent,name)
{
	verticalScrollBar()->installEventFilter(this);
}

CJanusWidget::CListBox::~CListBox()
{
}

bool CJanusWidget::CListBox::eventFilter(QObject *o, QEvent *e)
{
	if (e->type() == QEvent::Show || e->type() == QEvent::Hide)
		computeWidth();
	return KListBox::eventFilter(o,e);
}

void CJanusWidget::CListBox::computeWidth()
{
	Q3ListBoxItem	*item = firstItem();
	int	w(40);
	while (item)
	{
		w = QMAX(w,item->width(this));
		item = item->next();
	}
	if (verticalScrollBar()->isVisible())
		w += verticalScrollBar()->sizeHint().width();
	w += (frameWidth()*2);
	setFixedWidth(w);
}

//***********************************************************************************

CJanusWidget::CJanusWidget(QWidget *parent)
    : QWidget(parent)
{

	m_stack = new QStackedWidget(this);
	m_header = new QLabel(this);
	QFont	f(m_header->font());
	f.setBold(true);
	m_header->setFont(f);

	KSeparator* sep = new KSeparator( Qt::Horizontal, this);
	sep->setFixedHeight(5);

	m_iconlist = new CListBox(this);
	f = m_iconlist->font();
	f.setBold(true);
	m_iconlist->setFont(f);
	connect(m_iconlist,SIGNAL(selectionChanged(Q3ListBoxItem*)),SLOT(slotSelected(Q3ListBoxItem*)));

	m_empty = new QWidget( this );
        m_empty->setObjectName( "Empty" );
	m_stack->insertWidget(0,m_empty);

	QHBoxLayout	*main_ = new QHBoxLayout(this, 0, 10);
	QVBoxLayout	*sub_ = new QVBoxLayout(0, 0, 5);
	main_->addWidget(m_iconlist,0);
	main_->addLayout(sub_,1);
	sub_->addWidget(m_header,0);
	sub_->addWidget(sep,0);
	sub_->addWidget(m_stack,1);
}

CJanusWidget::~CJanusWidget()
{
	qDeleteAll(m_pages);
	m_pages.clear();
}

void CJanusWidget::addPage(QWidget *w, const QString& text, const QString& header, const QPixmap& pix)
{
	CPage	*page = new CPage();
	m_pages.append(page);
	page->m_widget = w;
	page->m_text = text;
	page->m_header = header;
	page->m_pixmap = pix;
	page->m_item = new CListBoxItem(m_iconlist,findPrevItem(page),pix,text);
	m_iconlist->computeWidth();
	m_stack->insertWidget(m_pages.count(),w);

	if (m_iconlist->count() == 1)
		m_iconlist->setSelected(page->m_item,true);
}

void CJanusWidget::enablePage(QWidget *w)
{
	CPage	*page = findPage(w);
	if (page && !page->m_item)
	{
		page->m_item = new CListBoxItem(m_iconlist,findPrevItem(page),page->m_pixmap,page->m_text);
		m_iconlist->computeWidth();
		if (m_iconlist->count() == 1)
			m_iconlist->setSelected(page->m_item,true);
	}
}

void CJanusWidget::disablePage(QWidget *w)
{
	CPage	*page = findPage(w);
	if (page && page->m_item)
	{
		bool	needReselect(m_iconlist->isSelected(page->m_item));
		delete page->m_item;
		page->m_item = 0;
		m_iconlist->computeWidth();
		if (needReselect)
			if (m_iconlist->count() > 0)
				m_iconlist->setSelected(m_iconlist->firstItem(),true);
			else
				slotSelected(0);
	}
}

void CJanusWidget::slotSelected(Q3ListBoxItem *item)
{
	CPage	*page = findPage(item);
	if (page)
	{
		m_stack->setCurrentWidget(page->m_widget);
		m_header->setText(page->m_header);
	}
	else
	{
		m_header->setText("");
		m_stack->setCurrentWidget(m_empty);
	}
}

CJanusWidget::CPage* CJanusWidget::findPage(QWidget *w)
{
	QListIterator<CPage*>	it(m_pages);
	while(it.hasNext())
	{
		CPage *item = it.next();
		if (item->m_widget == w)
			return item;
	}
	return 0;
}

CJanusWidget::CPage* CJanusWidget::findPage(Q3ListBoxItem *i)
{
	QListIterator<CPage*>	it(m_pages);
	while(it.hasNext())
	{
		CPage *item = it.next();
		if (item->m_item == i)
			return item;
	}
	return 0;
}

Q3ListBoxItem* CJanusWidget::findPrevItem(CPage *p)
{
	QListIterator<CPage*>   it(m_pages);
	if (m_pages.indexOf(p) == -1)
		it.toBack();
	else
		it.previous();
	while(it.hasPrevious())
	{
		CPage *item = it.previous();
		if (item->m_item)
			return item->m_item;
	}
	return 0;
}

void CJanusWidget::clearPages()
{
	QListIterator<CPage*>	it(m_pages);
	while(it.hasNext())
	{
		CPage *item = it.next();
		delete item->m_widget;
		delete item->m_item;
	}
	m_pages.clear();
}

#include "cjanuswidget.moc"

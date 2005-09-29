/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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

/**
 * KTabCtl provides a universal tab control. It is in no ways limited to dialogs and
 * can be used for whatever you want. It has no buttons or any other stuff.
 *
 * However, this is based on the original QTabDialog.
 */

#include <qtabbar.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "ktabctl.h"

KTabCtl::KTabCtl(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    tabs = new QTabBar(this, "_tabbar");
    connect(tabs, SIGNAL(selected(int)), this, SLOT(showTab(int)));
    tabs->move(2, 1);

    blBorder = true;

}

KTabCtl::~KTabCtl()
{
	delete tabs;
}

void KTabCtl::resizeEvent(QResizeEvent *)
{
    int i;
    QRect r = getChildRect();

    if (tabs) {
        for (i=0; i<(int)pages.size(); i++) {
            pages[i]->setGeometry(r);
        }
        if( ( tabs->shape() == QTabBar::RoundedBelow ) ||
            ( tabs->shape() == QTabBar::TriangularBelow ) ) {
            tabs->move( 0, height()-tabs->height()-4 );
        }
    }
}

void KTabCtl::setFont(const QFont & font)
{
    QFont f(font);
    f.setWeight(QFont::Light);
    QWidget::setFont(f);

    setSizes();
}

void KTabCtl::setTabFont(const QFont & font)
{
    QFont f(font);
//    f.setWeight(QFont::Light);
    tabs->setFont(f);

    setSizes();
}

void KTabCtl::show()
{
    unsigned int i;

    if(isVisible())
	return;

    setSizes();

    for(i = 0; i < pages.size(); i++)
	pages[i]->hide();

    QResizeEvent r(size(), size());
    resizeEvent(&r);

    QWidget::show();
}

bool KTabCtl::isTabEnabled(const QString& name)
{
    unsigned int i;

    for(i = 0; i < pages.size(); i++)
	if (QString::fromLatin1(pages[i]->name()) == name)
	    return tabs->isTabEnabled(i);   /* return the enabled status */
    return false;     /* tab does not exist */
}

void KTabCtl::setTabEnabled(const QString& name, bool state)
{
    unsigned i;

    if (name.isEmpty())
        return;

    for (i = 0; i < pages.size(); i++)
	if (QString::fromLatin1(pages[i]->name()) == name)
	    tabs->setTabEnabled(i, state);
}

void KTabCtl::setSizes()
{
    unsigned i;

    QSize min(tabs->sizeHint());    /* the minimum required size for the tabbar */
    tabs->resize(min);         /* make sure that the tabbar does not require more space than actually needed. */


    QSize max(QCOORD_MAX,QCOORD_MAX);
    //int th = min.height();          /* the height of the tabbar itself (without pages and stuff) */

    for (i = 0; i < pages.size(); i++) {

        /*
         * check the actual minimum and maximum sizes
         */

	if (pages[i]->maximumSize().height() < max.height())
	    max.setHeight(pages[i]->maximumSize().height());
	if (pages[i]->maximumSize().width() < max.width())
	    max.setWidth( pages[i]->maximumSize().width());
	if ( pages[i]->minimumSize().height() > min.height())
	    min.setHeight( pages[i]->minimumSize().height());
	if ( pages[i]->minimumSize().width() > min.width())
	    min.setWidth( pages[i]->minimumSize().width());
    }

    // BL: min and max are sizes of children, not tabcontrol
    // min.setHeight(min.height() + th);

    if (max.width() < min.width())
	max.setWidth(min.width());
    if (max.height() < min.height())
	max.setHeight(min.height());

    /*
     * now, apply the calculated size values to all of the pages
     */

    for( i=0; i<(uint)pages.size(); i++ ) {
	pages[i]->setMinimumSize(min);
	pages[i]->setMaximumSize(max);
    }


    // BL: set minimum size of tabcontrol
    setMinimumSize(min.width()+4, min.height()+tabs->height()+4);

    /*
     * generate a resizeEvent, if we're visible
     */

    if(isVisible()) {
	QResizeEvent r(size(), size());
	resizeEvent(&r);
    }
}

void KTabCtl::setBorder( bool state )
{
    blBorder = state;
}

void KTabCtl::setShape( QTabBar::Shape shape )
{
    tabs->setShape( shape );
}

QSize
KTabCtl::sizeHint() const
{
	/* desired size of the tabbar */
	QSize hint(tabs->sizeHint());

	/* overall desired size of all pages */
	QSize pageHint;
	for (unsigned int i = 0; i < pages.size(); i++)
	{
		QSize sizeI(pages[i]->sizeHint());

		if (sizeI.isValid())
		{
			/* only pages with valid size are used */
			if (sizeI.width() > pageHint.width())
				pageHint.setWidth(sizeI.width());

			if (sizeI.height() > pageHint.height())
				pageHint.setHeight(sizeI.height());
		}
	}

	if (pageHint.isValid())
	{
		/* use maximum of width of tabbar and pages */
		if (pageHint.width() > hint.width())
			hint.setWidth(pageHint.width());

		/* heights must just be added */
		hint.setHeight(hint.height() + pageHint.height());

		/* 1999-09-18: Espen Sand
		   I cannot get the size to be correct unless the total
		   border size is included: ie 2*2 pixels.
		*/
		return (hint + QSize(4,4));
	}

	/*
	 * If not at least a one page has a valid sizeHint we have to return
	 * an invalid size as well.
	 */
	return (pageHint);
}

void KTabCtl::paintEvent(QPaintEvent *)
{
    if (!tabs)
	return;

    if( !blBorder )
        return;

    QPainter p;
    p.begin(this);

    int y0 = getChildRect().top() - 1;
    int y1 = getChildRect().bottom() + 2;
    int x1 = getChildRect().right() + 2;
    int x0 = getChildRect().left() - 1;

    p.setPen(colorGroup().light());
    p.drawLine(x0, y0 - 1, x1 - 1, y0 - 1);      /* 1st top line */
    p.setPen(colorGroup().midlight());
    p.drawLine(x0, y0, x1 - 1, y0);      /* 2nd top line */
    p.setPen(colorGroup().light());
    p.drawLine(x0, y0 + 1, x0, y1);      /* left line */
    p.setPen(black);
    p.drawLine(x1, y1, x0, y1);          /* bottom line */
    p.drawLine(x1, y1 - 1, x1, y0);
    p.setPen(colorGroup().dark());
    p.drawLine(x0 + 1, y1 - 1, x1 - 1, y1 - 1);  /* bottom */
    p.drawLine(x1 - 1, y1 - 2, x1 - 1, y0 + 1);
    p.end();
}

/*
 * return the client rect. This is the maximum size for any child
 * widget (page).
 */

QRect KTabCtl::getChildRect() const
{
    if( ( tabs->shape() == QTabBar::RoundedBelow ) ||
        ( tabs->shape() == QTabBar::TriangularBelow ) ) {
    	return QRect(2, 1, width() - 4,
		     height() - tabs->height() - 4);
    } else {
      	return QRect(2, tabs->height() + 1, width() - 4,
		     height() - tabs->height() - 4);
    }
}

/*
 * show a single page, depending on the selected tab
 * emit tabSelected(new_pagenumber) BEFORE the page is shown
 */

void KTabCtl::showTab(int i)
{
    unsigned int j;
    for (j = 0; j < pages.size(); j++) {
      if (j != (unsigned)i) {
        pages[j]->hide();
      }
    }

    if((unsigned)i < pages.size()) {
        emit(tabSelected(i));
		if( pages.size() >= 2 ) {
			pages[i]->raise();
		}
		tabs->setCurrentTab(i);
        pages[i]->setGeometry(getChildRect());
        pages[i]->show();
    }
}

/*
 * add a tab to the control. This tab will manage the given Widget w.
 * in most cases, w will be a QWidget and will only act as parent for the
 * actual widgets on this page
 * NOTE: w is not required to be of class QWidget, but expect strange results with
 * other types of widgets
 */

void KTabCtl::addTab(QWidget *w, const QString& name)
{
    QTab *t = new QTab();
    t->setText( name );
    t->setEnabled( true );
    int id = tabs->addTab(t);   /* add the tab itself to the tabbar */
    if (id == (int)pages.size()) {
	pages.resize(id + 1);
        pages[id] = w;          /* remember the widget to manage by this tab */
    }
    // BL: compute sizes
    setSizes();
}

void KTabCtl::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktabctl.moc"

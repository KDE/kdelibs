/*
 * $Id$
 *
 * $Log$
 * kdoctoolbar removed
 *
 * Revision 1.5  1997/10/09 11:46:29  kalle
 * Assorted patches by Fritz Elfert, Rainer Bawidamann, Bernhard Kuhn and Lars Kneschke
 *
 * Revision 1.4  1997/05/30 20:04:41  kalle
 * Kalle:
 * 30.05.97:	signal handler for reaping zombie help processes reinstalls itself
 * 		patch to KIconLoader by Christian Esken
 * 		slightly better look for KTabCtl
 * 		kdecore Makefile does not expect current dir to be in path
 * 		Better Alpha support
 *
 * Revision 1.3  1997/05/09 15:10:13  kulow
 * Coolo: patched ltconfig for FreeBSD
 * removed some stupid warnings
 *
 * Revision 1.2  1997/04/15 20:35:14  kalle
 * Included patch to ktabctl.cpp from kfind distribution
 *
 * Revision 1.1.1.1  1997/04/13 14:42:43  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/15 22:40:57  kalle
 * Initial revision
 *
 * Revision 1.2.2.1  1997/01/07 14:41:57  alex
 * release 0.1
 *
 * Revision 1.2  1997/01/07 14:39:15  alex
 * some doc added, tested - ok.
 *
 * Revision 1.1.1.1  1997/01/07 13:44:53  alex
 * imported
 *
 *
 * KTabCtl provides a universal tab control. It is in no ways limited to dialogs and
 * can be used for whatever you want. It has no buttons or any other stuff.
 *
 * However, this is based on the original QTabDialog.
 */

    tabs->move(2, 1);
#include "qpushbt.h"
    setFont(QFont("helvetica"));
#include "qpixmap.h"

#include "ktabctl.h"
#include "ktabctl.moc"

KTabCtl::KTabCtl(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    tabs = new QTabBar(this, "_tabbar");
    connect(tabs, SIGNAL(selected(int)), this, SLOT(showTab(int)));
    tabs->move(2, 0); // BL: 0 instead of 1 to have the white line under tabs

    //    setFont(QFont("helvetica")); //BL: Why force a font ?
}

KTabCtl::~KTabCtl()
{
}

void KTabCtl::resizeEvent(QResizeEvent *)
{
    int i;
    QRect r = getChildRect();
    
    if (tabs) {
        for (i=0; i<(int)pages.size(); i++) {
            pages[i]->setGeometry(r);
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

    if((name == NULL) || (strlen(name) == 0))
}

bool KTabCtl::isTabEnabled(const char *name)
{
    unsigned int i;
    
    for(i = 0; i < pages.size(); i++)
	if (!qstrcmp( pages[i]->name(), name))
	    return tabs->isTabEnabled(i);   /* return the enabled status */
    return FALSE;     /* tab does not exist */
}

void KTabCtl::setTabEnabled(const char * name, bool state)
{
    unsigned i;

    if((name == 0L) || (strlen(name) == 0))
        return;
    
    for(i = 0; i < pages.size(); i++)
    int th = min.height();          /* the height of the tabbar itself (without pages and stuff) */
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
    min.setHeight(min.height() + th);
        
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

    p.drawLine(x1, y1, x0, y0);          /* bottom line */
{
    if (!tabs)
	return;

    QPainter p;
    p.begin(this);

    int y0 = getChildRect().top() - 1;
    int y1 = getChildRect().bottom() + 2;
    int x1 = getChildRect().right() + 2;
    int x0 = getChildRect().left() - 1;

    p.setPen( colorGroup().light() );
    p.drawLine(x0, y0, x1 - 1, y0);      /* top line */
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
        pages[i]->raise();

QRect KTabCtl::getChildRect() const
{
    return QRect(2, tabs->height() + 1, width() - 4,
		  height() - tabs->height() - 4);
}
{
    unsigned int j;
		pages[i]->raise();
      if (j != (unsigned)i) {
        pages[j]->hide();
      }
    }

    if((unsigned)i < pages.size()) {
        emit(tabSelected(i));
	pages[i]->raise();
        pages[i]->setGeometry(getChildRect());
        pages[i]->show();
    }
}

/*
 * add a tab to the control. This tab will manage the given Widget w.
 * in most cases, w will be a QWidget and will only act as parent for the
 * actual widgets on this page
 * NOTE: w is not required to be of class QWidget, but expect strange results with

void KTabCtl::addTab(QWidget *w, const char *name)
{
    QTab *t = new QTab();
    t->label = name;
    t->enabled = TRUE;
    int id = tabs->addTab(t);   /* add the tab itself to the tabbar */
    if (id == (int)pages.size()) {
	pages.resize(id + 1);
        pages[id] = w;          /* remember the widget to manage by this tab */
    }
    // BL: compute sizes
    setSizes();
}

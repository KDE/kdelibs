/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>
                   Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <qvaluelist.h>

#include <kwin.h>
#include <kwinmodule.h>
#include <klocale.h>
#include <kstringhandler.h>
#ifndef Q_WS_QWS //FIXME
#include <netwm.h>
#endif
#include <kapplication.h>
#include <dcopclient.h>

#undef Bool
#include "kwindowlistmenu.h"
#include "kwindowlistmenu.moc"

// helper class
class NameSortedInfoList : public QPtrList<KWin::Info>
{
public:
    NameSortedInfoList() { setAutoDelete(true); };
    ~NameSortedInfoList() {};

private:
    int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 );
};

int NameSortedInfoList::compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
{
#ifndef Q_WS_QWS //FIXME
    KWin::Info *i1 = static_cast<KWin::Info *>(s1);
    KWin::Info *i2 = static_cast<KWin::Info *>(s2);
    QString title1, title2;
    if (i1)
        title1 = i1->visibleNameWithState().lower();
    if (i2)
        title2 = i2->visibleNameWithState().lower();
    return title1.compare(title2);
#endif
}

KWindowListMenu::KWindowListMenu(QWidget *parent, const char *name)
  : KPopupMenu(parent, name)
{
#ifndef Q_WS_QWS //FIXME
    kwin_module = new KWinModule(this);
#endif

    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
}

KWindowListMenu::~KWindowListMenu()
{

}

void KWindowListMenu::init()
{
    int i, d;
    i = 0;

#ifndef Q_WS_QWS //FIXME
    int nd = kwin_module->numberOfDesktops();
    int cd = kwin_module->currentDesktop();
    WId active_window = kwin_module->activeWindow();

    clear();
    map.clear();

    insertItem( i18n("Unclutter Windows"),
		this, SLOT( slotUnclutterWindows() ) );
    insertItem( i18n("Cascade Windows"),
		this, SLOT( slotCascadeWindows() ) );

    insertSeparator();

    for (d = 1; d <= nd; d++) {
	if (nd > 1)
	    insertItem( kwin_module->desktopName( d ), 1000 + d);

	int items = 0;

	if (!active_window && d == cd)
	    setItemChecked(1000 + d, TRUE);

        NameSortedInfoList list;
        list.setAutoDelete(true);

	for (QValueList<WId>::ConstIterator it = kwin_module->windows().begin();
             it != kwin_module->windows().end(); ++it) {
	    KWin::Info info = KWin::info( *it );
	    if ((info.desktop == d) || (d == cd && info.onAllDesktops))
                list.inSort(new KWin::Info(info));
        }

        for (KWin::Info* info = list.first(); info!=0; info = list.next(), i++)
        {
            QString title = info->visibleNameWithState();
            if ( info->windowType == NET::Normal || info->windowType == NET::Unknown ) {
                QPixmap pm = KWin::icon(info->win, 16, 16, true );
                items++;
                if (items == 1 && nd > 1)
                    insertSeparator();
		QString itemText =  KStringHandler::csqueeze(title,25);
		// Avoid creating unwanted accelerators.
		itemText.replace(QRegExp("&"), "&&");
                insertItem( pm, QString("   ")+ itemText, i);
                map.insert(i, info->win);
                if (info->win == active_window)
                    setItemChecked(i, TRUE);
            }
        }
        if (d < nd)
            insertSeparator();
    }
#endif
    adjustSize();
}

void KWindowListMenu::slotExec(int id)
{
#ifndef Q_WS_QWS //FIXME
    if (id > 1000)
        KWin::setCurrentDesktop(id - 1000);
    else if ( id >= 0 )
	KWin::setActiveWindow(map[id]);
#endif
}

void KWindowListMenu::slotUnclutterWindows()
{
    kapp->dcopClient()->send("kwin", "KWinInterface", "unclutterDesktop()", "");
}

void KWindowListMenu::slotCascadeWindows()
{
    kapp->dcopClient()->send("kwin", "KWinInterface", "cascadeDesktop()", "");
}

void KWindowListMenu::virtual_hook( int id, void* data )
{ KPopupMenu::virtual_hook( id, data ); }


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
#include <netwm.h>
#include <kapp.h>
#include <dcopclient.h>

#include "kwindowlistmenu.h"
#include "kwindowlistmenu.moc"

KWindowListMenu::KWindowListMenu(QWidget *parent, const char *name)
  : QPopupMenu(parent, name)
{
    kwin_module = new KWinModule(this);

    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

KWindowListMenu::~KWindowListMenu()
{

}

void KWindowListMenu::slotAboutToShow()
{
    int i, d;

    int nd = kwin_module->numberOfDesktops();
    int cd = kwin_module->currentDesktop();
    WId active_window = kwin_module->activeWindow();

    clear();

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

	QValueList<WId>::ConstIterator it;
	for (it = kwin_module->windows().begin(), i = 0;
	     it != kwin_module->windows().end(); ++it, ++i) {
	    KWin::Info info = KWin::info( *it );
	    if ( (info.desktop == d ) || (d == cd && info.onAllDesktops ) ) {
		QString title = info.visibleNameWithState();
		if ( info.windowType == NET::Normal || info.windowType == NET::Unknown ) {
		    QPixmap pm = KWin::icon(*it, 16, 16, true );
		    items++;
		    insertItem( pm, QString("   ")+ KStringHandler::csqueeze(title,25),i);
		    if (*it == active_window)
			setItemChecked(i, TRUE);
		}
	    }
	}
	if (d < nd)
	    insertSeparator();
    }
    adjustSize();
}

void KWindowListMenu::slotExec(int id)
{
    if (id > 1000)
        KWin::setCurrentDesktop(id - 1000);
    else if ( id >= 0 )  {
	WId w = kwin_module->windows()[id];
	KWin::setActiveWindow(w);
    }
}

void KWindowListMenu::slotUnclutterWindows()
{
    kapp->dcopClient()->send("kwin", "KWinInterface", "unclutterDesktop()", "");
}

void KWindowListMenu::slotCascadeWindows()
{
    kapp->dcopClient()->send("kwin", "KWinInterface", "cascadeDesktop()", "");
}

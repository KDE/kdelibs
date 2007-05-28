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

#include <qglobal.h>

#ifdef Q_WS_X11

#include "config.h"
#include <qpainter.h>
#include <qvaluelist.h>

#include <kwin.h> 
#include <kwinmodule.h> 

#include <klocale.h>
#include <kstringhandler.h>

#include <netwm.h> 
#include <kapplication.h>
#include <kstyle.h>
#include <dcopclient.h>

#undef Bool
#include "kwindowlistmenu.h"
#include "kwindowlistmenu.moc"

static QCString kwinName() {
    QCString appname;
    int screen_number = DefaultScreen(qt_xdisplay());
    if (screen_number == 0)
        appname = "kwin";
    else
        appname.sprintf("kwin-screen-%d", screen_number);
    return appname;
}

// helper class
namespace
{
class NameSortedInfoList : public QPtrList<KWin::WindowInfo>
{
public:
    NameSortedInfoList() { setAutoDelete(true); }
    ~NameSortedInfoList() {}

private:
    int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 );
};

int NameSortedInfoList::compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
{
    KWin::WindowInfo *i1 = static_cast<KWin::WindowInfo *>(s1);
    KWin::WindowInfo *i2 = static_cast<KWin::WindowInfo *>(s2);
    QString title1, title2;
    if (i1)
        title1 = i1->visibleNameWithState().lower();
    if (i2)
        title2 = i2->visibleNameWithState().lower();
    return title1.compare(title2);
}

} // namespace

KWindowListMenu::KWindowListMenu(QWidget *parent, const char *name)
  : KPopupMenu(parent, name)
{
    kwin_module = new KWinModule(this);

    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
}

KWindowListMenu::~KWindowListMenu()
{

}

static bool standaloneDialog( const KWin::WindowInfo* info, const NameSortedInfoList& list )
{
    WId group = info->groupLeader();
    if( group == 0 )
    {
        return info->transientFor() == qt_xrootwin();
    }
    for( QPtrListIterator< KWin::WindowInfo > it( list );
         it.current() != NULL;
         ++it )
        if( (*it)->groupLeader() == group )
            return false;
    return true;
}

void KWindowListMenu::init()
{
    int i, d;
    i = 0;

    int nd = kwin_module->numberOfDesktops();
    int cd = kwin_module->currentDesktop();
    WId active_window = kwin_module->activeWindow();

    // Make sure the popup is not too wide, otherwise clicking in the middle of kdesktop
    // wouldn't leave any place for the popup, and release would activate some menu entry.    
    int maxwidth = kapp->desktop()->screenGeometry( this ).width() / 2 - 100;

    clear();
    map.clear();

    int unclutter = insertItem( i18n("Unclutter Windows"),
                                this, SLOT( slotUnclutterWindows() ) );
    int cascade = insertItem( i18n("Cascade Windows"),
                              this, SLOT( slotCascadeWindows() ) );

    // if we only have one desktop we won't be showing titles, so put a separator in
    if (nd == 1)
    {
        insertSeparator();
    }


    QValueList<KWin::WindowInfo> windows;
    for (QValueList<WId>::ConstIterator it = kwin_module->windows().begin();
         it != kwin_module->windows().end(); ++it) {
         windows.append( KWin::windowInfo( *it, NET::WMDesktop ));
    }
    bool show_all_desktops_group = ( nd > 1 );
    for (d = 1; d <= nd + (show_all_desktops_group ? 1 : 0); d++) {
        bool on_all_desktops = ( d > nd );
	int items = 0;

	if (!active_window && d == cd)
	    setItemChecked(1000 + d, true);

        NameSortedInfoList list;
        list.setAutoDelete(true);

	for (QValueList<KWin::WindowInfo>::ConstIterator it = windows.begin();
             it != windows.end(); ++it) {
	    if (((*it).desktop() == d) || (on_all_desktops && (*it).onAllDesktops())
                || (!show_all_desktops_group && (*it).onAllDesktops())) {
	        list.inSort(new KWin::WindowInfo( (*it).win(),
                    NET::WMVisibleName | NET::WMState | NET::XAWMState | NET::WMWindowType,
                    NET::WM2GroupLeader | NET::WM2TransientFor ));
            }
        }

        for (KWin::WindowInfo* info = list.first(); info; info = list.next(), ++i)
        {
            QString itemText = KStringHandler::cPixelSqueeze(info->visibleNameWithState(), fontMetrics(), maxwidth);
            NET::WindowType windowType = info->windowType( NET::NormalMask | NET::DesktopMask
                | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
                | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );
            if ( (windowType == NET::Normal || windowType == NET::Unknown
                    || (windowType == NET::Dialog && standaloneDialog( info, list )))
                && !(info->state() & NET::SkipTaskbar) ) {
                QPixmap pm = KWin::icon(info->win(), 16, 16, true );
                items++;

                // ok, we have items on this desktop, let's show the title
                if ( items == 1 && nd > 1 )
                {
                    if( !on_all_desktops )
                        insertTitle(kwin_module->desktopName( d ), 1000 + d);
                    else
                        insertTitle(i18n("On All Desktops"), 2000 );
                }

                // Avoid creating unwanted accelerators.
                itemText.replace('&', QString::fromLatin1("&&"));
                insertItem( pm, itemText, i);
                map.insert(i, info->win());
                if (info->win() == active_window)
                    setItemChecked(i, true);
            }
        }

        if (d == cd)
        {
            setItemEnabled(unclutter, items > 0);
            setItemEnabled(cascade, items > 0);
        }
    }

    // no windows?
    if (i == 0)
    {
        if (nd > 1)
        {
            // because we don't have any titles, nor a separator
            insertSeparator();
        }

        setItemEnabled(insertItem(i18n("No Windows")), false);
    }
}

void KWindowListMenu::slotExec(int id)
{
    if (id == 2000)
        ; // do nothing
    else if (id > 1000)
        KWin::setCurrentDesktop(id - 1000);
    else if ( id >= 0 )
	KWin::forceActiveWindow(map[id]);
}

// This popup is much more useful from keyboard if it has the active
// window active by default - however, QPopupMenu tries hard to resist.
// QPopupMenu::popup() resets the active item, so this needs to be
// called after popup().
void KWindowListMenu::selectActiveWindow()
{
    for( unsigned int i = 0;
         i < count();
         ++i )
        if( isItemChecked( idAt( i )))
            {
            setActiveItem( i );
            break;
            }
}

void KWindowListMenu::slotUnclutterWindows()
{
    kapp->dcopClient()->send(kwinName(), "KWinInterface", "unclutterDesktop()", "");
}

void KWindowListMenu::slotCascadeWindows()
{
    kapp->dcopClient()->send(kwinName(), "KWinInterface", "cascadeDesktop()", "");
}

void KWindowListMenu::virtual_hook( int id, void* data )
{ KPopupMenu::virtual_hook( id, data ); }

#endif // Q_WS_X11


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
#include <q3ptrlist.h>
#include <QDesktopWidget>
#include <QX11Info>

#include <kwin.h> 
#include <kwinmodule.h> 

#include <klocale.h>
#include <kstringhandler.h>

#include <netwm.h> 
#include <QApplication>
#include <kstyle.h>
#include <dcopclient.h>

#undef Bool
#include "kwindowlistmenu.h"
#include "kwindowlistmenu.moc"

// helper class
namespace
{
class NameSortedInfoList : public Q3PtrList<KWin::WindowInfo>
{
public:
    NameSortedInfoList() { setAutoDelete(true); };
    ~NameSortedInfoList() {};

private:
    int compareItems( Q3PtrCollection::Item s1, Q3PtrCollection::Item s2 );
};

int NameSortedInfoList::compareItems( Q3PtrCollection::Item s1, Q3PtrCollection::Item s2 )
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

KWindowListMenu::KWindowListMenu(QWidget *parent, const char */*name*/)
  : KMenu(parent)
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
        return info->transientFor() == QX11Info::appRootWindow();
    }
    for( Q3PtrListIterator< KWin::WindowInfo > it( list );
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
    int maxwidth = qApp->desktop()->screenGeometry( this ).width() / 2 - 100;

    clear();

    QAction* unclutter = addAction( i18n("Unclutter Windows"),
                                this, SLOT( slotUnclutterWindows() ) );
    QAction* cascade = addAction( i18n("Cascade Windows"),
                              this, SLOT( slotCascadeWindows() ) );

    // if we only have one desktop we won't be showing titles, so put a separator in
    if (nd == 1)
    {
        addSeparator();
    }


    QList<KWin::WindowInfo> windows;
    foreach (WId id, kwin_module->windows())
         windows.append( KWin::windowInfo( id, NET::WMDesktop ));

    bool show_all_desktops_group = ( nd > 1 );
    for (d = 1; d <= nd + (show_all_desktops_group ? 1 : 0); d++) {
        bool on_all_desktops = ( d > nd );
    int items = 0;

    // KDE4 porting - huh? didn't know you could set an item checked before it's created?
    //if (!active_window && d == cd)
        //setItemChecked(1000 + d, true);

    NameSortedInfoList list;
    list.setAutoDelete(true);

    foreach (KWin::WindowInfo wi, windows) {
        if ((wi.desktop() == d) || (on_all_desktops && wi.onAllDesktops())
                || (!show_all_desktops_group && wi.onAllDesktops())) {
            list.inSort(new KWin::WindowInfo( wi.win(),
                    NET::WMVisibleName | NET::WMState | NET::XAWMState | NET::WMWindowType,
                    NET::WM2GroupLeader | NET::WM2TransientFor ));
            }
        }

        foreach (KWin::WindowInfo* info, list) {
            ++i;
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
                        addTitle(kwin_module->desktopName( d ));
                    else
                        addTitle(i18n("On All Desktops"));
                }

                // Avoid creating unwanted accelerators.
                itemText.replace('&', QLatin1String("&&"));
                QAction* a = addAction(pm, itemText, this, SLOT(slotForceActiveWindow()));
                a->setData((int)info->win());
                if (info->win() == active_window)
                    a->setChecked(true);
            }
        }

        if (d == cd)
        {
            unclutter->setEnabled(items > 0);
            cascade->setEnabled(items > 0);
        }
    }

    // no windows?
    if (i == 0)
    {
        if (nd > 1)
        {
            // because we don't have any titles, nor a separator
            addSeparator();
        }

        addAction(i18n("No Windows"))->setEnabled(false);
    }
}

void KWindowListMenu::slotForceActiveWindow()
{
    QAction* window = qobject_cast<QAction*>(sender());
    if (!window || !window->data().canConvert(QVariant::Int))
        return;

    KWin::forceActiveWindow(window->data().toInt());
}

void KWindowListMenu::slotSetCurrentDesktop()
{
    QAction* window = qobject_cast<QAction*>(sender());
    if (!window || !window->data().canConvert(QVariant::Int))
        return;

    KWin::setCurrentDesktop(window->data().toInt());
}

// This popup is much more useful from keyboard if it has the active
// window active by default - however, QPopupMenu tries hard to resist.
// QPopupMenu::popup() resets the active item, so this needs to be
// called after popup().
void KWindowListMenu::selectActiveWindow()
{
    foreach (QAction* action, actions())
        if (action->isChecked()) {
            setActiveAction(action);
            break;
        }
}

void KWindowListMenu::slotUnclutterWindows()
{
    DCOPRef("kwin", "KWinInterface").send("unclutterDesktop()");
}

void KWindowListMenu::slotCascadeWindows()
{
    DCOPRef("kwin", "KWinInterface").send("cascadeDesktop()");
}

void KWindowListMenu::virtual_hook( int id, void* data )
{ KMenu::virtual_hook( id, data ); }

#endif // Q_WS_X11


/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.
          (c) Michael Goffioul <goffioul@imec.be>

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

#include <kglobal.h>
#include <kconfig.h>
#include <qtimer.h>

#include "kpanelmenu.h"
#include "kpanelmenu.moc"
//#include "kaccelmanager.h"


class KPanelMenuPrivate
{
public:
    bool init;
    int clearDelay;
    QString startPath;
    QTimer t;
};

KPanelMenu::KPanelMenu(const QString &startDir, QWidget *parent, const char *name)
  : KPopupMenu(parent, name)
{
    init(startDir);
}

KPanelMenu::KPanelMenu(QWidget *parent, const char *name)
  : KPopupMenu(parent, name)
{
    init();
}

void KPanelMenu::init(const QString& path)
{
    d = new KPanelMenuPrivate;

    setInitialized( false );
    d->startPath = path;

    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));

    // setup cache timer
    KConfig *config = KGlobal::config();
    config->setGroup("menus");
    d->clearDelay = config->readNumEntry("MenuCacheTime", 60000); // 1 minute

    //KAcceleratorManager::manage(this);
    setKeyboardShortcutsEnabled(true);
}

KPanelMenu::~KPanelMenu()
{
    delete d;
}

void KPanelMenu::slotAboutToShow()
{
    // stop the cache timer
    if(d->clearDelay)
        d->t.stop();

    // teared off ?
    if ( isTopLevel() )
        d->clearDelay = 0;

    internalInitialize();
}

void KPanelMenu::slotClear()
{
    setInitialized( false );
    clear();
}

void KPanelMenu::hideEvent(QHideEvent *ev)
{
    // start the cache timer
    if(d->clearDelay) {
        disconnect(&(d->t), SIGNAL(timeout()), this, SLOT(slotClear()));
        connect(&(d->t), SIGNAL(timeout()), this, SLOT(slotClear()));
        d->t.start(d->clearDelay, true);
    }
    QPopupMenu::hideEvent(ev);
}

void KPanelMenu::disableAutoClear()
{
    d->clearDelay = 0;
}

const QString& KPanelMenu::path() const
{
    return d->startPath;
}

void KPanelMenu::setPath(const QString& p)
{
    d->startPath = p;
}

bool KPanelMenu::initialized() const
{
    return d->init;
}

void KPanelMenu::setInitialized(bool on)
{
    d->init = on;
}

void KPanelMenu::reinitialize()
{
    deinitialize();
    // Yes, reinitialize must call initialize(). Otherwise, menus
    // may not appear in the right place. Don't change this! If
    // you want delayed initialization, use deinitialize() instead.
    internalInitialize();
}

void KPanelMenu::deinitialize()
{
    slotClear();
}

void KPanelMenu::internalInitialize()
{
    if( initialized() )
        return;
    initialize();
    setInitialized( true );
}

void KPanelMenu::virtual_hook( int id, void* data )
{ KPopupMenu::virtual_hook( id, data ); }


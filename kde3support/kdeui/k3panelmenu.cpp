/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.
          (c) Michael Goffioul <kdeprint@swing.be>

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
#include <kconfiggroup.h>
#include <QTimer>

#include "k3panelmenu.h"
#include "moc_k3panelmenu.cpp"
//#include "kaccelmanager.h"


class K3PanelMenu::Private
{
public:
    bool init;
    int clearDelay;
    QString startPath;
    QTimer t;
};

K3PanelMenu::K3PanelMenu(const QString &startDir, QWidget *parent)
  : KMenu(parent)
{
    init(startDir);
}

K3PanelMenu::K3PanelMenu(QWidget *parent)
  : KMenu(parent)
{
    init();
}

void K3PanelMenu::init(const QString& path)
{
    d = new Private;

    setInitialized( false );
    d->startPath = path;

    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));

    // setup cache timer
    KConfigGroup config(KGlobal::config(), "menus");
    d->clearDelay = config.readEntry("MenuCacheTime", 60000); // 1 minute

    //KAcceleratorManager::manage(this);
    setKeyboardShortcutsEnabled(true);
}

K3PanelMenu::~K3PanelMenu()
{
    delete d;
}

void K3PanelMenu::slotAboutToShow()
{
    // stop the cache timer
    if(d->clearDelay)
        d->t.stop();

    // teared off ?
    if ( isTopLevel() )
        d->clearDelay = 0;

    internalInitialize();
}

void K3PanelMenu::slotClear()
{
    setInitialized( false );
    clear();
}

void K3PanelMenu::hideEvent(QHideEvent *ev)
{
    // start the cache timer
    if(d->clearDelay) {
        disconnect(&(d->t), SIGNAL(timeout()), this, SLOT(slotClear()));
        connect(&(d->t), SIGNAL(timeout()), this, SLOT(slotClear()));
        d->t.setSingleShot(true);
        d->t.start(d->clearDelay);
    }
    KMenu::hideEvent(ev);
}

void K3PanelMenu::disableAutoClear()
{
    d->clearDelay = 0;
}

const QString& K3PanelMenu::path() const
{
    return d->startPath;
}

void K3PanelMenu::setPath(const QString& p)
{
    d->startPath = p;
}

bool K3PanelMenu::initialized() const
{
    return d->init;
}

void K3PanelMenu::setInitialized(bool on)
{
    d->init = on;
}

void K3PanelMenu::reinitialize()
{
    deinitialize();
    // Yes, reinitialize must call initialize(). Otherwise, menus
    // may not appear in the right place. Don't change this! If
    // you want delayed initialization, use deinitialize() instead.
    internalInitialize();
}

void K3PanelMenu::deinitialize()
{
    slotClear();
}

void K3PanelMenu::internalInitialize()
{
    if( initialized() )
        return;
    initialize();
    setInitialized( true );
}


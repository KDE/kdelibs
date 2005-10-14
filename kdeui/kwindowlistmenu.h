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

#ifndef kwindowlistmenu_h
#define kwindowlistmenu_h

#include <kmenu.h>
#include <qmap.h>

#ifdef Q_WS_X11 // not yet available for non-X11

class KWinModule;
class KWindowListMenuPrivate;

class KDEUI_EXPORT KWindowListMenu : public KMenu
{
    Q_OBJECT

public:
    KWindowListMenu( QWidget *parent = 0, const char *name = 0 );
    virtual ~KWindowListMenu();

    void init();

public slots:
    /**
     * Pre-selects the active window in the popup menu, for faster
     * keyboard navigation. Needs to be called after popup().
     * Should not be used when the popup is invoked using the mouse. 
     */
    void selectActiveWindow();
    
protected slots:
    void slotForceActiveWindow();
    void slotSetCurrentDesktop();
    void slotUnclutterWindows();
    void slotCascadeWindows();

private:
    KWinModule*         kwin_module;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KWindowListMenuPrivate *d;
};

#endif // Q_WS_X11

#endif

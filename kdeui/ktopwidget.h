#ifndef _KTOPWIDGET_H
#define _KTOPWIDGET_H

#include <ktmainwindow.h>

/**
 * This was top level widget. It inherits KTMainWindow completely now.
 * You can still use is under this name, if you are so nostalgic.
 * You shouldn't use this widget it might be removed from libraries
 * in future.
 * @see KTMainWindow
 * @short Old KDE top level window
 * @author Stephan Kulow (coolo@kde.org), was maintained by Sven Radej (radej@kde.org)
 */

class KTopLevelWidget : public KTMainWindow {
    Q_OBJECT

     friend class KToolBar;

public:
    /**
     * Constructor.
     */
    KTopLevelWidget( const char *name = 0L );
    /**
     * Destructor. 
     */
    ~KTopLevelWidget();

protected:
    /**
     * This is called when the widget is closed.
     * The default implementation jut calls accepts the event.
     * This method is only difference from KTMainWindow. Old
     * applications reimplement closeEvent, new should use 
     * @ref KTMainWindow and reimplement some of special handlers
     * there.
     */
    virtual void closeEvent ( QCloseEvent *);

};

#endif
//Eh!!!

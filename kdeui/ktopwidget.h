#ifndef _KTOPWIDGET_H
#define _KTOPWIDGET_H

#error THIS FILE IS OBSOLETE.
#error Use KTMainWindow instead of KTopLevelWidget.

#include <ktmainwindow.h>

/**
 * This was top level widget. It inherits KTMainWindow completely now.
 * You can still use is under this name, if you are so nostalgic.
 * You shouldn't use this widget it might be removed from libraries
 * in future.
 *
 * The only difference is that KTMainWindow is per default created with
 * the WDestructiveClose flag. This means, it is automatically 
 * deleted when the widget accepts a close event. A KTopLevelWidget
 * needs to be deleted manually ( it is basically a just a KTMainWindow( "name", 0 ); )
 *
 *
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

};

#endif
//Eh!!!

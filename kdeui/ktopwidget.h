#ifndef _KTOPWIDGET_H
#define _KTOPWIDGET_H

#include <ktmainwindow.h>

/**
 * This was top level widget. It inherits KTMainWindow completely now.
 * You can still use is under this name, if you are so nostalgic.
 * @see KTMainWindow
 * @short Old name for KDE top level window
 * @author Stephan Kulow (coolo@kde.org)  Maintained by Sven Radej (radej@kde.org)

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
};

#endif
//Eh!!!

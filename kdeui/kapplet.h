#ifndef KAPPLET_H
#define KAPPLET_H

#include <qwidget.h>
#include <dcopobject.h>

class KAppletData;

/**
* KDE Panel Applet class
*
* This class implements panel applets
*
* @author Matthias Ettrich <ettrich@kde.org>
* @short KDE Panel Applet class
*/
class KApplet : public QWidget, DCOPObject
{
    Q_OBJECT
public:

    /**
   * Construct a KApplet object.
   */
    KApplet( QWidget* parent = 0, const char* name = 0 );
    /*
     * Destroy a KApplet object.
     */
    ~KApplet();

    /**
       Initializes the applet according to the passed command line
       parameters

       Evalutates some command line arguments, docks into the
       respective applet container and eventually call
       setupGeometry().
     */
    void init( int& argc, char ** argv );

    /**
       Sets up the applets geometry. This function needs to be
       reimplemented by subclasses.

       @param orientation is the applets orientation, either Qt::Horizontal 
       or Qt::Vertical
       @param width and 
       @param height define the size of the applet.

       The size parameter is meant as a hint in case an applet
       supports different look&feels depending on the target size. Of
       course, an applet could do these things in resizeEvent(), but
       this way it's more convinient.

       The applet container that will embed this applet will resize it
       to the standard size of its applets. If your applet needs more
       space, ensure to set a proper minimum size with
       QWidget::setMinimumWidth() or QWidget::setMinimumHeight()
       depending on the applets orientation.
       
       Keep in mind that setupGeometry() may be called several times
       during a life-cycle of your applet, for example when the applet
       container is resized, moved or changes orientation.
     */
    virtual void setupGeometry( Orientation orientation, int width, int height );


    QSize sizeHint() const;


    /**
       Returns  the current orientation set by the last KApplet::init() call.
    */
    Orientation orientation() const;

    
    
    // dcop internal
    bool process(const QCString &fun, const QByteArray &data,
		 QCString& replyType, QByteArray &replyData);

public slots:

    /*!
      Tells the applet container that the applet wants to be
      removed. Subclasses should provide a context menu with a
      "Remove" item connected to this slot.
     */
    void removeRequest();

    /*!
      Tells the applet container that the applet wants to be
      moved around. Subclasses should provide a context menu with a
      "Move" item connected to this slot.
     */
    void moveRequest();

    

private:
    KAppletData* d;
};


#endif

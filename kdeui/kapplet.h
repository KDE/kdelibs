#ifndef KAPPLET_H
#define KAPPLET_H

#include <qwidget.h>


class KAppletData;

/**
* KDE Panel Applet class
*
* This class implements panel applets
*
* @author Matthias Ettrich <ettrich@kde.org>
* @short KDE Panel Applet class
*/
class KApplet : public QWidget
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
       parameters.

       Will evalutate the command line arguments and call init()
       respectively.
     */
    void setup( int& argc, char ** argv );

    /**
       Initializes the applet. This function needs to be reimplemented
       by subclasses.

       @param orientation is the applets orientation, either Qt::Horizontal or Qt::Vertical
       @param width and @param height define the size of the applet. 
       
       The size parameter is  meant as a hint in case an applet supports different look&feels 
       depending on the target size. Of course, an applet could do these things in resizeEvent(),
       but this way it's more convinient.

       The applet container that will embed this applet will resize it to the
       standard size of its applets. If your applet needs more space, ensure to set a proper
       minimum size with QWidget::setMinimumWidth() or QWidget::setMinimumHeight() depending
       on the applets orientation.
     */
    virtual void init( Orientation orientation, int width, int height );

    QSize sizeHint() const;
    
    
    /**
       Returns  the current orientation set by the last KApplet::init() call.
    */
    Orientation orientation() const;
    
public slots:
    void removeRequest();
    void moveRequest();

private:
    KAppletData* d;
};


#endif

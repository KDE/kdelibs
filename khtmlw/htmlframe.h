#ifndef HTMLFRAME_H
#define HTMLFRAME_H

#include <qwidget.h>
#include <qlist.h>
#include <qframe.h>

class HTMLFramePanner;
class HTMLFrameSet;
class KHTMLView;

class HTMLFramePanner : public QFrame
{
    Q_OBJECT
public:
    enum Orientation { HORIZONTAL = 1, VERTICAL = 2 };
    
    HTMLFramePanner( HTMLFramePanner::Orientation _orientation, QWidget *_parent = 0L, const char *_name = 0L );
    ~HTMLFramePanner();
    
    void setChild1( QWidget *_child ) { child1 = _child; }
    void setChild2( QWidget *_child ) { child2 = _child; }    

    void setIsMoveable( bool _move );

protected:
    virtual void mousePressEvent( QMouseEvent *_ev );
    virtual void mouseMoveEvent( QMouseEvent *_ev );
    virtual void mouseReleaseEvent( QMouseEvent *_ev );
    
    QWidget *child1;
    QWidget *child2;

    int initialX;
    int initialY;
    QPoint initialGlobal;
    
    HTMLFramePanner::Orientation orientation;

    bool moveable;
};

/**
 * This class is for INTERNAL USE ONLY.
 */
class HTMLFrameSet : public QWidget
{
    Q_OBJECT
public:
    HTMLFrameSet( QWidget *_parent, const char *_src );
    ~HTMLFrameSet();
    
    void append( QWidget *_w );

    virtual void parse();
  
    virtual int calcSize( const char *s, int *size, int _max );

    KHTMLView* getSelectedFrame();

    /**
     * @return TRUE if the user is allowed to resize the frame set.
     *
     * @see #bAllowResize
     */
    bool getAllowResize() { return bAllowResize; }
    /**
     * @return the width of the frames border ( read: @ref HTMLFramePanner ) in pixels
     *         or -1 for the default width.
     */
    int getFrameBorder() { return frameBorder; }
  
protected:
    virtual void resizeEvent( QResizeEvent* _ev );
    
    QList<QWidget> widgetList;

    HTMLFramePanner::Orientation orientation;

    QString cols;
    QString rows;
    
    /**
     * Array that holds the layout information for all embedded frames.
     */
    int *size;
    /**
     * Amount of frames as mentioned in the COLS or ROWS tag.
     */
    int elements;

    /**
     * The amount of frames we parsed until now.
     */
    int cFrames;

    /**
     * The amount of pixels used for the frames border ( read @ref HTMLFramePanner ).
     * A value of -1 indicates the default.
     */
    int frameBorder;
  
    /**
     * This flags is usually TRUE. But if we have the &lt;frame noresize&gt; tag, 
     * this flag is set to FALSE to indicate that the user may not resize this frame set.
     */
    bool bAllowResize;

    /**
     * This is the last panner added to the widgetList. This variable is used
     * during parsing only.
     */
    HTMLFramePanner *lastPanner;
};



#endif




















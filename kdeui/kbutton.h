#ifndef KBUTTON_H
#define KBUTTON_H

#include <kpixmap.h>
#include <qbutton.h>

class KButton : public QButton
{
    Q_OBJECT
public:
    KButton( QWidget *_parent, char *_name );
    ~KButton();

public slots:
    void slotPressed();
    void slotReleased();
    
protected:
    virtual void leaveEvent( QEvent *_ev );
    virtual void enterEvent( QEvent *_ev );    
        
    virtual void drawButton( QPainter *_painter );
    virtual void drawButtonLabel( QPainter *_painter );

    void paint( QPainter *_painter );
    
    int raised;    
};

#endif

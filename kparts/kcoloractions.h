#ifndef kcoloractions_h
#define kcoloractions_h

#include <qaction.h>
#include <qvaluelist.h>
#include <qcolor.h>
#include <qwidget.h>

class QMouseEvent;
class QPaintEvent;

class KColorAction : public QAction
{
    Q_OBJECT
    
public:
    enum Type {
	TextColor,
	FrameColor,
	BackgroundColor
    };
    
    KColorAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KColorAction( const QString& text, int accel,
		  QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KColorAction( const QString& text, Type type, int accel = 0,
		  QObject* parent = 0, const char* name = 0 );
    KColorAction( const QString& text, Type type, int accel,
		  QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KColorAction( QObject* parent = 0, const char* name = 0 );

    virtual void setColor( const QColor &c );
    QColor color() const;
    
    virtual void setType( Type type );
    Type type() const;
    
private:
    void init();
    void createPixmap();
    
    QColor col;
    Type typ;
        
};

class KColorBar : public QWidget
{
    Q_OBJECT
    
public:
    KColorBar( const QValueList<QColor> &cols, QWidget *parent, const char *name );

protected:
    void mousePressEvent( QMouseEvent *e );
    void paintEvent( QPaintEvent *e );
    
private:
    QValueList<QColor> colors;
    
signals:
    void leftClicked( const QColor &c );
    void rightClicked( const QColor &c );
    
};

class KColorBarAction : public QAction
{
    Q_OBJECT
    
public:
    KColorBarAction( const QString &text, int accel, 
		     QObject *receiver, const char *leftClickSlot, const char *rightClickSlot,
		     const QValueList<QColor> &cols, QObject *parent, const char *name );

    int plug( QWidget* );
    
signals:
    void leftClicked( const QColor &c );
    void rightClicked( const QColor &c );

private:
    QValueList<QColor> colors;
    QObject *receiver;
    char *leftClickSlot, *rightClickSlot;
    
};

#endif

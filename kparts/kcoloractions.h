#ifndef kcoloractions_h
#define kcoloractions_h

#include <qaction.h>

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

#endif

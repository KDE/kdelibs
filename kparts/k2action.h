#ifndef __k2action_h__
#define __k2action_h__ $Id$

#include <qaction.h>

class K2Action : public QAction
{
  Q_OBJECT
public:
    K2Action( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    K2Action( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    K2Action( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    K2Action( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    K2Action( QObject* parent = 0, const char* name = 0 );

    virtual int plug( QWidget *w );
    virtual void unplug( QWidget *w );

    virtual void setEnabled( bool b );

    virtual void setText( const QString& text );

    virtual void setIconSet( const QIconSet& iconSet );

};

#endif

#ifndef __kaction_h__
#define __kaction_h__ $Id$

#include <qaction.h>

class KAction : public QAction
{
  Q_OBJECT
public:
    KAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    KAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KAction( QObject* parent = 0, const char* name = 0 );

    virtual int plug( QWidget *w );
    virtual void unplug( QWidget *w );

    virtual void setEnabled( bool b );

    virtual void setText( const QString& text );

    virtual void setIconSet( const QIconSet& iconSet );

};

class KToggleAction : public QToggleAction
{
    Q_OBJECT
public:
    KToggleAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KToggleAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KToggleAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    KToggleAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KToggleAction( QObject* parent = 0, const char* name = 0 );

    int plug( QWidget* );

    virtual void setChecked( bool );
    bool isChecked() const;

    void uncheckGroup();

protected slots:
    void slotActivated();

protected:
    bool locked, locked2;
    bool checked;

};

class KSelectAction : public QSelectAction
{
    Q_OBJECT

public:
    KSelectAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KSelectAction( const QString& text, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KSelectAction( const QString& text, const QIconSet& pix, int accel = 0,
	     QObject* parent = 0, const char* name = 0 );
    KSelectAction( const QString& text, const QIconSet& pix, int accel,
	     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KSelectAction( QObject* parent = 0, const char* name = 0 );

    int plug( QWidget* );

    virtual void setCurrentItem( int i );	
    void setItems( const QStringList& lst );

protected slots:
    virtual void slotActivated( int );

};

#endif

/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __kaction_h__
#define __kaction_h__ $Id$

#include <qaction.h>
#include <qfontdatabase.h>

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
    void clear();

protected slots:
    void slotActivated( const QString &text );

signals:
    void activate();

private:
    bool m_lock;
};

class KFontAction : public KSelectAction
{
    Q_OBJECT

public:
    KFontAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, int accel,
		 QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, int accel = 0,
		 QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, int accel,
		 QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KFontAction( QObject* parent = 0, const char* name = 0 );

    void setFont( const QString &family );
    QString font() {
	return currentText();
    }

private:
    QStringList fonts;

};

class KFontSizeAction : public KSelectAction
{
    Q_OBJECT

public:
    KFontSizeAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, int accel,
		     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, int accel = 0,
		     QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, int accel,
		     QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KFontSizeAction( QObject* parent = 0, const char* name = 0 );

    void setFontSize( int size );
    int fontSize();

protected slots:
    virtual void slotActivated( int );
    virtual void slotActivated( const QString& );

signals:
    void fontSizeChanged( int );

private:
    void init();

    bool m_lock;

};

class KActionMenu : public QActionMenu
{
  Q_OBJECT
public:
    KActionMenu( const QString& text, QObject* parent = 0, const char* name = 0 );
    KActionMenu( const QString& text, const QIconSet& icon, QObject* parent = 0, const char* name = 0 );
    KActionMenu( QObject* parent = 0, const char* name = 0 );
  
    virtual int plug( QWidget* widget );
    virtual void unplug( QWidget* widget );
    
    virtual void setEnabled( bool b );

    virtual void setText( const QString& text );

    virtual void setIconSet( const QIconSet& iconSet );
};

#endif

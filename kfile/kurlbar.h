/* This file is part of the KDE libraries
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KURLBAR_H
#define KURLBAR_H

#include <qevent.h>
#include <qframe.h>
#include <qtooltip.h>

#include <kdialogbase.h>
#include <kicontheme.h>
#include <klistbox.h>
#include <kurl.h>

class KConfig;
class KURLBar;

class KURLBarItem : public QListBoxPixmap
{
public:
    KURLBarItem( KURLBar *parent, const KURL& url,
                 const QString& description = QString::null,
                 const QString& icon = QString::null,
                 int group = KIcon::Panel, KURLBarItem *after = 0L );
    ~KURLBarItem();

    void setURL( const KURL& url );
    void setIcon( const QString& icon, int group = KIcon::Panel );
    void setDescription( const QString& desc );
    void setToolTip( const QString& tip );

    virtual int width( const QListBox * ) const;
    virtual int height( const QListBox * ) const;

    const KURL& url() const                     { return m_url; }
    const QString& description() const          { return m_description; }
    const QString& icon() const                 { return m_icon; }
    QString toolTip() const;
    int iconGroup() const                       { return m_group; }
    virtual const QPixmap * pixmap() const      { return &m_pixmap; }

    void setApplicationLocal( bool local )      { m_appLocal = local; }
    bool applicationLocal() const               { return m_appLocal; }


protected:
    virtual void paint( QPainter *p );

private:
    int iconSize() const;

    KURL m_url;
    QString m_description;
    QString m_icon;
    QString m_toolTip;
    QPixmap m_pixmap;
    int m_group;
    KURLBar *m_parent;
    bool m_appLocal :1;

    class KURLBarItemPrivate;
    KURLBarItemPrivate *d;
};

class KURLBarListBox;

class KURLBar : public QFrame
{
    Q_OBJECT

public:
    KURLBar( bool useGlobalItems,
             QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~KURLBar();

    virtual KURLBarItem * insertItem( const KURL& url,
                                      const QString& description,
                                      bool applicationLocal = true,
                                      const QString& icon = QString::null,
                                      int group = KIcon::Panel );
    virtual void setOrientation( Orientation orient );

    // will clear all the items
    virtual void setListBox( KURLBarListBox * );
    KURLBarListBox *listBox() const { return m_listBox; }

    virtual void setIconSize( int size );
    int iconSize() const { return m_iconSize; }

    virtual void clear();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    virtual void readConfig(  KConfig *, const QString& itemGroup );
    virtual void writeConfig( KConfig *, const QString& itemGroup );

    virtual void readItem( int i, KConfig *, bool applicationLocal );
    virtual void writeItem( KURLBarItem *item, int i, KConfig * );

public slots:
    virtual void setCurrentItem( const KURL& url ); 
    
signals:
    void activated( const KURL& url );

protected:
    virtual void resizeEvent( QResizeEvent * );
    bool isVertical() const;

    KURLBarItem * m_activeItem;
    bool m_useGlobal :1;
    
protected slots:
    virtual void slotContextMenuRequested( QListBoxItem *, const QPoint& pos );
    virtual void slotSelected( QListBoxItem * );
    virtual void slotDropped( QDropEvent * );

private:
    KURLBarListBox *m_listBox;
    int m_iconSize;


    class KURLBarPrivate;
    KURLBarPrivate *d;
};


class KURLBarToolTip : public QToolTip
{
public:
    KURLBarToolTip( QListBox *view ) : QToolTip( view ), m_view( view ) {}

protected:
    virtual void maybeTip( const QPoint & );

private:
    QListBox *m_view;
};


class QDragObject;

class KURLBarListBox : public KListBox
{
    Q_OBJECT

public:
    KURLBarListBox( QWidget *parent = 0, const char *name = 0 );
    ~KURLBarListBox();

    virtual void setOrientation( Orientation orient );
    Orientation orientation() const { return m_orientation; }

signals:
    void dropped( QDropEvent *e );

protected:
    virtual QDragObject * dragObject();
    virtual void contentsDragEnterEvent( QDragEnterEvent * );
    virtual void contentsDropEvent( QDropEvent * );

private:
    Orientation m_orientation;
    KURLBarToolTip m_toolTip;

private:
    class KURLBarListBoxPrivate;
    KURLBarListBoxPrivate *d;
};


class QCheckBox;
class KIconButton;
class KLineEdit;

class KURLBarDropDialog : public KDialogBase
{
    Q_OBJECT

public:
    static bool getInformation( bool allowGlobal, const KURL& url,
                                QString& description, QString& icon,
                                bool& appLocal, QWidget *parent = 0 );

    KURLBarDropDialog( bool allowGlobal, const KURL& url,
                       const QString& description, QString icon,
                       QWidget *parent = 0, const char *name = 0 );
    ~KURLBarDropDialog();

    QString description() const;
    QString icon() const;
    bool applicationLocal() const;

protected:
    KLineEdit * m_edit;
    KIconButton * m_iconButton;
    QCheckBox * m_appLocal;

private:
    class KURLBarDropDialogPrivate;
    KURLBarDropDialogPrivate *d;
    
};


#endif // KURLBAR_H

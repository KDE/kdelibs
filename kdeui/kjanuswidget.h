/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Espen Sand (espensa@online.no)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef _KJANUS_WIDGET_H_
#define _KJANUS_WIDGET_H_

#include <qlist.h>
#include <qlistview.h>
#include <qwidget.h>

class QLabel;
class QWidgetStack;

class KTabCtl;
class KSeparator;

/**
 * This class provides a number of ready to use layouts (faces). It is used 
 * as an internal widget in @ref KDialogBase, but can also used as a 
 * widget of its own. 
 *
 * It provides TreeList, Tabbed, Plain and Swallow layouts.
 *
 * To be continued...
 *
 * @short Easy to used widget with many layouts
 * @author Espen Sand (espensa@online.no)
 */
class KJanusWidget : public QWidget
{
  Q_OBJECT
  
  public:
    enum Face
    {
      TreeList = 0,
      Tabbed,
      Plain,
      Swallow
    };

  public:
    KJanusWidget( QWidget *parent=0, const char *name=0, int face=Plain );
    ~KJanusWidget( void );

    bool showPage( int index );
    int  activePageIndex( void ) const;
    bool isValid( void ) const;
    int  face( void ) const;
    virtual QSize minimumSizeHint( void );

    QWidget *plainPage( void );
    QFrame  *addPage(const QString &item,const QString &header=QString::null);
    bool setSwallowedWidget( QWidget *widget );

  public slots:
    virtual void setFocus( void ); 

  private slots:
    bool slotShowPage( void );
    void slotFontChanged( void );

  private:
    bool showPage( QWidget *w );

  private:
    bool mValid;

    QList<QWidget> *mPageList;
    QList<QListViewItem> *mTreeNodeList;
    QStringList *mTitleList;

    int          mFace;
    QListView    *mTreeList;
    QWidgetStack *mPageStack;
    QLabel       *mTitleLabel;
    KTabCtl      *mTabControl;
    QWidget      *mPlainPage;
    QWidget      *mSwallowPage;
    QWidget      *mActivePageWidget;
    KSeparator   *mTitleSep;
    int          mActivePageIndex;
};

#endif






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
 * The TreeList face provides a list in the left area and pages in the 
 * right. The area are separated by a movable splitter. The look is somewhat
 * similar to the layout in the Control Center. A page is raised by 
 * selecting the corresponding tree list item.
 *
 * The Tabbed face is a common tabbed widget. The procedure for creating a 
 * page is similar for creating a TreeList. This has the advantage that if
 * your widget contain too many pages it is trivial to convert it into a 
 * TreeList. Just change the face in the @ref KJanusWidget constructor to
 * KJanusWidget::TreeList and you have a tree list layout instead.
 *
 * The Plain face provided an empty widget (QFrame) where you can place your
 * widgets. The KJanusWidget makes no assumptions regarding the contents so
 * you are free to add whatever you want.
 *
 * The Swallow face is provided in order to simplify the usage of existing
 * widgets and to allow changing the visible widget. You specify the widget
 * to be displayed by @ref setSwallowedWidget . Your widget will be 
 * reparented inside the widget. You can specify a Null (0) widget. A empty
 * space is then displayed.
 *
 * For all modes it is important that you specify the @ref QWidget::minimumSize
 * on the page, plain widget or the swallowed widget. This size is used
 * when the KJanusWidget determines its own minimum size. You get the latter
 * minimum size by using the @ref minimumSizeHint method.
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

    /** 
     * Constructore wher you specify the face.
     * 
     * @param parent Parent of the widget.
     * @param name Widget name.
     * @param int face The kind of dialog, Use TreeList, Tabbed, Plain or
     * Swallow
     */
    KJanusWidget( QWidget *parent=0, const char *name=0, int face=Plain );

    /**
     * Destructor
     */
    ~KJanusWidget( void );

    /**
     * Raises the page which was added by @ref addPage. If you want to display
     * the third page, specify 2.
     *
     * @parame index The index of the page you want to raise.
     */
    bool showPage( int index );

    /**
     * Returns the index of the page that are currentlt displayed.
     *
     * @return The index or -1 of the face is not TreeList or Tabbed
     */
    int  activePageIndex( void ) const;

    /**
     * Return true if the widget was properly created. Use this to verify 
     * that no memory allocation failed.
     *
     * @return true if widget is valid.
     */
    bool isValid( void ) const;

    /**
     * Returns the face type
     *
     * @return The face type.
     */
    int face( void ) const;

    /**
     * Returns the minimum size that must be made avaiable for the widget
     * so that uis can be displayed properly
     *
     * @return the minimum size.
     */
    virtual QSize minimumSizeHint( void );

    /**
     * Returns the empty widget that is available in Plain mode.
     *
     * @return The widget or 0 if the face in not Plain.
     */
    QFrame *plainPage( void );

    /**
     * Add a new page to either TreeList or Tabbed mode. The returned widget
     * is empty and you add your widget code to this widget.
     *
     * @param item String used in the tree list or Tab item.
     * @param header A longer string only used in TreeList mode to describe
     * the contents of a page. If empty, the item string will be used instead.
     *
     * @return The empty page or 0 if the face is not TreeList or Tabbed.
     */
    QFrame *addPage(const QString &item,const QString &header=QString::null);

    /**
     * Defines the widget to be swallowed. This method can be used several 
     * times. Only the latest defined widget will be shown.
     *
     * @param widget The widget to be swallowed. If 0, then an empty rectangle
     * is displayed.
     */
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
    QFrame       *mPlainPage;
    QWidget      *mSwallowPage;
    QWidget      *mActivePageWidget;
    KSeparator   *mTitleSep;
    int          mActivePageIndex;
};

#endif






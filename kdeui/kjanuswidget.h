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

#include <qgrid.h>
#include <qlist.h>
#include <qsplitter.h>
#include <qwidget.h>

#include <klistview.h>

class QGrid;
class QHBox;
class QLabel;
class QTabWidget;
class QVBox;
class QWidgetStack;
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
 * minimum size by using the @ref minimumSizeHint or @ref sizeHint methods.
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
     * Constructor where you specify the face.
     * 
     * @param parent Parent of the widget.
     * @param name Widget name.
     * @param int face The kind of dialog, Use TreeList, Tabbed, Plain or
     * Swallow.
     */
    KJanusWidget( QWidget *parent=0, const char *name=0, int face=Plain );

    /**
     * Destructor.
     */
    ~KJanusWidget( void );

    /**
     * Raises the page which was added by @ref addPage(). If you want to display
     * the third page, specify 2.
     *
     * @parame index The index of the page you want to raise.
     */
    bool showPage( int index );

    /**
     * Retrieve the index of the page that are currently displayed.
     *
     * @return The index or -1 of the face is not TreeList or Tabbed
     */
    int  activePageIndex( void ) const;

    /**
     * @return @p true if the widget was properly created. 
     *
     * Use this to verify 
     * that no memory allocation failed.
     */
    bool isValid( void ) const;

    /**
     * Retrieve the face type.
     *
     * @return The face type.
     */
    int face( void ) const;

    /**
     * Retrieve the minimum size that must be made available for the widget
     * so that UIs can be displayed properly
     *
     * @return The minimum size.
     */
    virtual QSize minimumSizeHint( void ) const;

    /**
     * Returns the recommended size for the widget in order to be displayed
     * properly.
     *
     * @return The recommended size.
     */
    virtual QSize sizeHint( void ) const;

    /**
     * Retrieve the empty widget that is available in Plain mode.
     *
     * @return The widget or 0 if the face in not Plain.
     */
    QFrame *plainPage( void );

    /**
     * Add a new page when the class is used in either TreeList or Tabbed 
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. In most cases you must create a layout
     * manager and associate it with this widget as well.
     *
     * @param item String used in the tree list or Tab item.
     * @param header A longer string only used in TreeList mode to describe
     *        the contents of a page. If empty, the item string will be 
     *        used instead.
     *
     * @return The empty page or 0 if the face is not TreeList or Tabbed.
     */
    QFrame *addPage(const QString &item,const QString &header=QString::null);

    /**
     * Add a new page when the class is used in either TreeList or Tabbed 
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. The returned widget is a @ref QVBox
     * so it contains a QVBoxLayout layout that lines up the child widgets 
     * are vertically.
     *
     * @param item String used in the tree list or Tab item.
     * @param header A longer string only used in TreeList mode to describe
     *        the contents of a page. If empty, the item string will be 
     *        used instead.
     *
     * @return The empty page or 0 if the face is not TreeList or Tabbed.
     */
    QVBox *addVBoxPage( const QString &item, 
			const QString &header=QString::null );

    /**
     * Add a new page when the class is used in either TreeList or Tabbed 
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. The returned widget is a @ref QHBox
     * so it contains a QHBoxLayout layout that lines up the child widgets 
     * are horizontally.
     *
     * @param item String used in the tree list or Tab item.
     * @param header A longer string only used in TreeList mode to describe
     *        the contents of a page. If empty, the item string will be 
     *        used instead.
     *
     * @return The empty page or 0 if the face is not TreeList or Tabbed.
     */
    QHBox *addHBoxPage( const QString &itemName, 
			const QString &header=QString::null );

    /**
     * Add a new page when the class is used in either TreeList or Tabbed 
     * mode. The returned widget is empty and you must add your widgets 
     * as children to this widget. The returned widget is a @ref QGrid
     * so it contains a QGridLayout layout that places up the child widgets 
     * in a grid.
     *
     * @param n Specifies the number of columns if 'dir' is QGrid::Horizontal
     *          or the number of rows if 'dir' is QGrid::Vertical.
     * @param dir Can be QGrid::Horizontal or QGrid::Vertical.
     * @param item String used in the tree list or Tab item.
     * @param header A longer string only used in TreeList mode to describe
     *        the contents of a page. If empty, the item string will be 
     *        used instead.
     *
     * @return The empty page or 0 if the face is not TreeList or Tabbed.
     */
    QGrid *addGridPage( int n, QGrid::Direction dir, 
			const QString &itemName, 
			const QString &header=QString::null );

    /**
     * Defines the widget to be swallowed. 
     *
     * This method can be used several 
     * times. Only the latest defined widget will be shown.
     *
     * @param widget The widget to be swallowed. If 0, then an empty rectangle
     * is displayed.
     */
    bool setSwallowedWidget( QWidget *widget );

    /**
     * This function has only effect in TreeList mode.
     *
     * Defines how the tree list is resized when the widget is resized
     * horizontally. By default the tree list keeps its width when the  
     * widget becomes wider.
     *
     * @param state The resize mode. If false (default) the TreeList keeps
     *              its current width when the widget becomes wider.
     */
     void setTreeListAutoResize( bool state );

  public slots:
    /**
     * Give the keyboard input focus to the widget.
     */
    virtual void setFocus( void ); 

  protected:
    /**
     * Reimplemented to handle the splitter width when the the face
     * is TreeList
     */
    virtual void showEvent( QShowEvent * );

  private slots:
    bool slotShowPage( void );
    void slotFontChanged( void );

  private:
    bool showPage( QWidget *w );
    void addPageWidget( QFrame *page, const QString &itemName, 
			const QString &header );

  private:
    bool mValid;

    QList<QWidget> *mPageList;
    QList<QListViewItem> *mTreeNodeList;
    QStringList *mTitleList;

    int          mFace;
    KListView    *mTreeList;
    QWidgetStack *mPageStack;
    QLabel       *mTitleLabel;
    QTabWidget   *mTabControl;
    QFrame       *mPlainPage;
    QWidget      *mSwallowPage;
    QWidget      *mActivePageWidget;
    KSeparator   *mTitleSep;
    int          mActivePageIndex;
    QSplitter::ResizeMode mTreeListResizeMode;
};

#endif






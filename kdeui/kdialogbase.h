/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 *  Espen Sand (espen@kde.org)
 *  Holger Freyther <freyther@kde.org>
 *  2005-2006 Olivier Goffart <ogoffart at kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */
#ifndef _KDIALOG_BASE_H_
#define _KDIALOG_BASE_H_

#include <kdialog.h>
#include <kjanuswidget.h>
#include <kguiitem.h>
#include <kstdguiitem.h>

class KPushButton;
class KSeparator;
class KURLLabel;
class QBoxLayout;
class QPixmap;

/**
 * Used internally by KDialogBase.
 * @internal
 */
class KDialogBaseButton;

/**
 * Used internally by KDialogBase.
 * @internal
 */
class KDialogBaseTile;

/**
 * @short A dialog base class with standard buttons and predefined layouts.
 *
 * Provides basic functionality needed by nearly all dialogs.
 *
 * It offers the standard action buttons you'd expect to find in a
 * dialog as well as the capability to define at most three configurable
 * buttons. You can define a main widget that contains your specific
 * dialog layout or you can use a predefined layout. Currently, @p
 * TreeList/Paged, @p Tabbed, @p Plain, @p Swallow and @p IconList
 * mode layouts (faces) are available.
 *
 * You can set a background tile (pixmap) for parts of the dialog. The
 * tile you select is shared by all instances of this class in your
 * application so that they all get the same look and feel.
 *
 * There is a tutorial available on http://developer.kde.org/ (NOT YET)
 * that contains
 * copy/paste examples as well a screenshots on how to use this class.
 *
 * <b>Dialog shapes:</b>\n
 *
 * You can either use one of the prebuilt, easy to use, faces or
 * define your own main widget. The dialog provides ready to use
 * TreeList, Tabbed, Plain, Swallow and IconList faces. KDialogBase uses
 * the KJanusWidget class internally to accomplish this. If you
 * use TreeList, Tabbed or IconList mode, then add pages with addPage().
 *
 * Pages that have been added can be removed again by simply deleting
 * the page.
 *
 * If you want complete control of how the dialog contents should look,
 * then you can define a main widget by using setMainWidget(). You
 * only need to set the minimum size of that widget and the dialog will
 * resize itself to fit this minimum size.  The dialog is resizeable, but
 * cannot be made smaller than its minimum size.
 *
 * <b>Example:</b>\n
 *
 * \code
 * UrlDlg::UrlDlg( QWidget *parent, const QString& caption,
 *                 const QString& urltext)
 *   : KDialogBase( parent, "urldialog", true, caption,
 *                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
 * {
 *   QWidget *page = new QWidget( this );
 *   setMainWidget(page);
 *   QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
 *
 *   QLabel *label = new QLabel( caption, page, "caption" );
 *   topLayout->addWidget( label );
 *
 *   lineedit = new QLineEdit( urltext, page, "lineedit" );
 *   lineedit->setMinimumWidth(fontMetrics().maxWidth()*20);
 *   topLayout->addWidget( lineedit );
 *
 *   topLayout->addStretch(10);
 * }
 * \endcode
 *
 * If you use makeVBoxMainWidget(), then the dialog above can be made
 * simpler but you lose the ability to add a stretchable area:
 *
 * \code
 * UrlDlg::UrlDlg( QWidget *parent, const QString& caption,
 *                 const QString& urltext)
 * : KDialogBase( parent, "urldialog", true, caption,
 *                KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
 * {
 *   QVBox *page = makeVBoxMainWidget();
 *   QLabel *label = new QLabel( caption, page, "caption" );
 *
 *   lineedit = new QLineEdit( urltext, page, "lineedit" );
 *   lineedit->setMinimumWidth(fontMetrics().maxWidth()*20);
 * }
 * \endcode
 *
 * This class can be used in many ways. Note that most KDE ui widgets
 * and many of KDE core applications use the KDialogBase so for more
 * inspiration you should study the code for these.
 *
 * @author Mirko Boehm (mirko@kde.org) and Espen Sand (espen@kde.org)
 */
class KDEUI_EXPORT KDialogBase : public KDialog
{
  Q_OBJECT

  public:

    /**
     *  @li @p TreeList - A dialog with a tree on the left side and a
     *                    representation of the contents on the right side.
     *  @li @p Tabbed -   A dialog using a QTabWidget.
     *  @li @p Plain -    A normal dialog. Use plainPage() as parent for widgets.
     *  @li @p Swallow -  Simplifes the usage of existing widgets. You specify
     *                    the widget to be displayed by setMainWidget().
     *  @li @p IconList - A dialog with an iconlist on the left side and a
     *                    representation of the contents on the right side.
     */
    enum DialogType
    {
      TreeList  = KJanusWidget::TreeList,
      Tabbed    = KJanusWidget::Tabbed,
      Plain     = KJanusWidget::Plain,
      Swallow   = KJanusWidget::Swallow,
      IconList  = KJanusWidget::IconList
    };

  public:

    /**
     * @deprecated  you should use KDialog dirrectly instead of this
     * Constructor for the standard mode where you must specify the main
     * widget with setMainWidget() .
     *
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only)
     * @param modal Controls dialog modality. If @p false, the rest of the
     *        program interface (example: other dialogs) is accessible while
     *        the dialog is open.
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies which buttons will be visible. If zero
     *        (0) no button box will be made.
     * @param defaultButton Specifies which button will be marked as
     *        the default. Use ButtonCode::NoDefault to indicate that no button
     *        should be marked as the default button.
     * @param separator If @p true, a separator line is drawn between the
     *        action buttons and the main widget.
     * @param user1 User button1 item.
     * @param user2 User button2 item.
     * @param user3 User button3 item.
     */
    KDialogBase( QWidget *parent=0, const char *name=0, bool modal=true,
		 const QString &caption=QString(),
		 int buttonMask=Ok|Apply|Cancel, ButtonCode defaultButton=Ok,
		 bool separator=false,
		 const KGuiItem &user1=KGuiItem(),
		 const KGuiItem &user2=KGuiItem(),
		 const KGuiItem &user3=KGuiItem() ) KDE_DEPRECATED;

    /**
     * In KDE4 a WFlag paramater should be added after modal and next
     * function can be removed.
     *
     * Constructor for the predefined layout mode where you specify the
     * kind of layout (face).
     *
     * @param dialogFace You can use TreeList, Tabbed, Plain, Swallow or
     *        IconList.
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies which buttons will be visible. If zero
     *        (0) no button box will be made.
     * @param defaultButton Specifies which button will be marked as
     *        the default. Use ButtonCode::NoDefault to indicate that no button
     *        should be marked as the default button.
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only).
     * @param modal Controls dialog modality. If @p false, the rest of the
     *        program interface (example: other dialogs) is accessible while
     *        the dialog is open.
     * @param separator If @p true, a separator line is drawn between the
     *        action buttons and the main widget.
     * @param user1 User button1 text item.
     * @param user2 User button2 text item.
     * @param user3 User button3 text item.
     */
    KDialogBase( int dialogFace, const QString &caption,
		 int buttonMask, ButtonCode defaultButton,
		 QWidget *parent=0, const char *name=0, bool modal=true,
		 bool separator=false,
		 const KGuiItem &user1=KGuiItem(),
		 const KGuiItem &user2=KGuiItem(),
		 const KGuiItem &user3=KGuiItem() );


    /**
     * Constructor for the predefined layout mode where you specify the
     * kind of layout (face).
     *
     * @param dialogFace You can use TreeList, Tabbed, Plain, Swallow or
     *        IconList.
     * @param f widget flags, by default it is just set to WStyle_DialogBorder.
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only).
     * @param modal Controls dialog modality. If @p false, the rest of the
     *        program interface (example: other dialogs) is accessible while
     *        the dialog is open.
     * @param buttonMask Specifies which buttons will be visible. If zero
     *        (0) no button box will be made.
     * @param defaultButton Specifies which button will be marked as
     *        the default. Use ButtonCode::NoDefault to indicate that no button
     *        should be marked as the default button.
     * @param separator If @p true, a separator line is drawn between the
     *        action buttons and the main widget.
     * @param user1 User button1 text item.
     * @param user2 User button2 text item.
     * @param user3 User button3 text item.
     * @since: 3.2
     */

    KDialogBase( KDialogBase::DialogType dialogFace, Qt::WFlags f,
		 QWidget *parent=0, const char *name=0, bool modal=true,
		 const QString &caption=QString(),
		 int buttonMask=Ok|Apply|Cancel, ButtonCode defaultButton=Ok,
		 bool separator=false,
		 const KGuiItem &user1=KGuiItem(),
		 const KGuiItem &user2=KGuiItem(),
		 const KGuiItem &user3=KGuiItem() );

    /**
     * @deprecated  use KDialog directly
     * Constructor for a message box mode where the @p buttonMask can only
     * contain Yes, No, or Cancel.
     *
     * If you need other names you can rename
     * the buttons with setButtonText(). The dialog box is not resizable
     * by default but this can be changed by setInitialSize(). If you
     * select 'modal' to be true, the dialog will return Yes, No, or Cancel
     * when closed otherwise you can use the signals yesClicked(),
     * noClicked(), or cancelClicked() to determine the state.
     *
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies which buttons will be visible. If zero
     *        (0) no button box will be made.
     * @param defaultButton Specifies which button will be marked as
     *        the default. Use ButtonCode::NoDefault to indicate that no button
     *        should be marked as the default button.
     * @param escapeButton Specifies which button will be activated by
     *        when the dialog receives a @p Key_Escape keypress.
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only).
     * @param modal Controls dialog modality. If @p false, the rest of the
     *        program interface (example: other dialogs) is accessible
     *        while the dialog is open.
     * @param separator If @p true, a separator line is drawn between the
     *        action buttons and the main widget.
     * @param yes Text to use for the first button (defaults to i18n("Yes"))
     * @param no Text to use for the second button (defaults to i18n("No"))
     * @param cancel Text to use for the third button (defaults to i18n("Cancel"))
     */
    KDialogBase( const QString &caption, int buttonMask=Yes|No|Cancel,
		 ButtonCode defaultButton=Yes, ButtonCode escapeButton=Cancel,
		 QWidget *parent=0, const char *name=0,
		 bool modal=true, bool separator=false,
		 const KGuiItem &yes = KStdGuiItem::yes(), // i18n("&Yes")
		 const KGuiItem &no = KStdGuiItem::no(), // i18n("&No"),
		 const KGuiItem &cancel = KStdGuiItem::cancel() // i18n("&Cancel")
		 ) KDE_DEPRECATED ;

    /**
     * Destructor.
     */
    ~KDialogBase();

    /**
     * Retrieve the empty page when the predefined layout is used in @p Plain
     * mode.
     *
     * This widget must be used as the toplevel widget of your dialog
     * code.
     *
     * @return The widget or 0 if the predefined layout mode is not @p Plain
     *         or if you don't use any predefined layout.
     */
    QFrame *plainPage();

    /**
     * Add a page to the dialog when the class is used in @p TreeList ,
     * @p IconList or @p Tabbed mode.
     *
     * The returned widget must be used as the
     * toplevel widget for this particular page.
     * Note: The returned frame widget has no
     * layout manager associated with it. In order to use it you must
     * create a layout yourself as the example below illustrates:
     *
     * \code
     * QFrame *page = addPage( i18n("Layout") );
     * QVBoxLayout *topLayout = new QVBoxLayout( page, 0, KDialog::spacingHint() );
     * QLabel *label = new QLabel( i18n("Layout type"), page );
     * topLayout->addWidget( label );
     * ..
     * \endcode
     *
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes. Ignored in @p Tabbed
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in @p IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    QFrame  *addPage( const QString &itemName,
		      const QString &header=QString(),
		      const QPixmap &pixmap=QPixmap() );

    /**
     * This is like addPage() just above, with the difference that the first
     * element is a list of strings.
     *
     * These strings are used to form a path
     * of folders down to the given page. The initial elements are names
     * for the folders, while the last element is the name of the page.
     * Note: This does yet only work for the @p TreeList face. Later this may
     * be added for the @p IconList face too. In other faces than the
     * @p TreeList, all the strings except the last one is ignored.
     **/
    QFrame  *addPage( const QStringList &items,
		      const QString &header=QString(),
		      const QPixmap &pixmap=QPixmap() );

    /**
     * Add a page to the dialog when the class is used in @p TreeList,
     * @p IconList or @p Tabbed mode.
     *
     * The returned widget must be used as the toplevel widget for
     * this particular page. The widget contains a QVBoxLayout
     * layout so the widget children are lined up vertically.  You can
     * use it as follows:
     *
     * \code
     * QVBox *page = addVBoxPage( i18n("Layout") );
     * QLabel *label = new QLabel( i18n("Layout type"), page );
     * ..
     * \endcode
     *
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes. Ignored in @p Tabbed
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in @p IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    KVBox *addVBoxPage( const QString &itemName,
			const QString &header=QString(),
			const QPixmap &pixmap=QPixmap() );

    /**
     * This is like addVBoxPage() just above, with the difference
     * that the first element is a list of strings.
     *
     * These strings are used to form a path
     * of folders down to the given page. The initial elements are names
     * for the folders, while the last element is the name of the page.
     * Note: This does yet only work for the @p TreeList face. Later this may
     * be added for the @p IconList face too. In other faces than the
     * @p TreeList, all the strings except the last one is ignored.
     **/
    KVBox *addVBoxPage( const QStringList &items,
			const QString &header=QString(),
			const QPixmap &pixmap=QPixmap() );

    /**
     * Add a page to the dialog when the class is used in @p TreeList,
     * @p IconList or @p Tabbed mode.
     *
     * The returned widget must be used as the
     * toplevel widget for this particular page. The widget contains a
     * QHBoxLayout layout so the widget children are lined up horizontally.
     * You can use it as follows:
     *
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes. Ignored in Tabbed
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    KHBox *addHBoxPage( const QString &itemName,
			const QString &header=QString(),
			const QPixmap &pixmap=QPixmap() );

    /**
     * This is like addHBoxPage() just above, with the
     * difference that the first element is a list of strings.
     *
     * These strings are used to form a path
     * of folders down to the given page. The initial elements are names
     * for the folders, while the last element is the name of the page.
     * Note: This does yet only work for the @p TreeList face. Later this may
     * be added for the @p IconList face too. In other faces than the
     * @p TreeList, all the strings except the last one is ignored.
     **/
    KHBox *addHBoxPage( const QStringList &items,
			const QString &header=QString(),
			const QPixmap &pixmap=QPixmap() );

// Deprecated - use addPage() instead, and add a QGridLayout
#ifdef KDE3_SUPPORT
    /**
     * Add a page to the dialog when the class is used in @p TreeList,
     * @p IconList or @p Tabbed mode.
     *
     * The returned widget must be used as the
     * toplevel widget for this particular page. The widget contains a
     * QGridLayout layout so the widget children are  positioned in a grid.
     *
     * @param n Specifies the number of columns if @p dir is Qt::Horizontal
     *          or the number of rows if @p dir is Qt::Vertical.
     * @param dir Can be Qt::Horizontal or Qt::Vertical.
     * @param itemName String used in the list or as tab item name.
     * @param header Header text use in the list modes @p Ignored in @p Tabbed
     *        mode. If empty, the item text is used instead.
     * @param pixmap Used in @p IconList mode. You should prefer a pixmap
     *        with size 32x32 pixels.
     *
     * @return The page widget which must be used as the toplevel widget for
     *         the page.
     */
    Q3Grid *addGridPage( int n, Qt::Orientation dir,
			const QString &itemName,
			const QString &header=QString(),
			const QPixmap &pixmap=QPixmap() );


    /**
     * This is like addGridPage() just above, with the difference
     * that the first element is a list of strings.
     *
     * These strings are used to form a path
     * of folders down to the given page. The initial elements are names
     * for the folders, while the last element is the name of the page.
     * Note: This does yet only work for the @p TreeList face. Later this may
     * be added for the @p IconList face too. In other faces than the
     * @p TreeList, all the strings except the last one is ignored.
     **/
    Q3Grid *addGridPage( int n, Qt::Orientation dir,
			const QStringList &items,
			const QString &header=QString(),
			const QPixmap &pixmap=QPixmap() );
#endif


    /**
     * Sets the icon used in @p TreeList Mode for the given path.
     *
     * @param path The path for which this icon should be shown.
     * @param pixmap The icon used.
     **/
    void setFolderIcon(const QStringList &path,const QPixmap &pixmap);

    /**
     * Make a main widget.
     *
     * The function will make a QFrame widget
     * and use setMainWidget() to register it. You can @em not use this
     * function more than once, @em not if you have already defined a
     * main widget with setMainWidget() and @em not if you have used the
     * constructor where you define the face (@p Plain, @p Swallow, @p Tabbed,
     * @p TreeList).
     *
     * @return The main widget or 0 if any of the rules described above
     *         were broken.
     */
    QFrame *makeMainWidget();

    /**
     * Make a main widget.
     *
     * The function will make a QVBox widget
     * and use setMainWidget() to register it. You @em can use this
     * function more than once, but @em not if you have already defined a
     * main widget with setMainWidget() and @em not if you have used the
     * constructor where you define the face (@p Plain, @p Swallow, @p Tabbed,
     * @p TreeList, @p IconList).
     *
     * @return The main widget or 0 if any of the rules described above
     *         were broken.
     */
    KVBox *makeVBoxMainWidget();

    /**
     * Make a main widget.
     *
     * The function will make a QHBox widget
     * and use setMainWidget() to register it. You can @em not use this
     * function more than once, @em not if you have already defined a
     * main widget with setMainWidget() and @p not if you have used the
     * constructor where you define the face (@p Plain, @p Swallow, @p Tabbed,
     * @p TreeList, @p IconList).
     *
     * @return The main widget or 0 if any of the rules described above
     *         were broken.
     */
    KHBox *makeHBoxMainWidget();

// Deprecated - use makeMainWidget() instead, and add a QGridLayout
#ifdef KDE3_SUPPORT
    /**
     * Make a main widget.
     *
     * The function will make a QGrid widget
     * and use setMainWidget() to register it. You can @em not use this
     * function more than once, @em not if you have already defined a
     * main widget with setMainWidget and @em not if you have used the
     * constructor where you define the face (Plain, Swallow, Tabbed,
     * TreeList, IconList).
     *
     * @param n Specifies the number of columns if 'dir' is Qt::Horizontal
     *          or the number of rows if 'dir' is Qt::Vertical.
     * @param dir Can be Qt::Horizontal or Qt::Vertical.
     *
     * @return The main widget or 0 if any of the rules described above
     *         were broken.
     */
    Q3Grid *makeGridMainWidget( int n, Qt::Orientation dir );
#endif

    /**
     * Hide or display the OK button.
     *
     *  The OK button must have
     * been created in the constructor to be displayed.
     *
     * @param state If @p true, display the button(s).
	 * @deprecated  use ShowButton
     */
    void showButtonOK( bool state ) KDE_DEPRECATED;

    /**
     * Hide or display the Apply button.
     *
     *  The Apply button must have
     * been created in the constructor to be displayed.
     *
     * @param state true display the button(s).
	 * @deprecated  use ShowButton
     */
    void showButtonApply( bool state )  KDE_DEPRECATED;

    /**
     * Hide or display the Cancel button. The Cancel button must have
     * been created in the constructor to be displayed.
     *
     * @param state @p true display the button(s).
	 * @deprecated  use ShowButton
     */
    void showButtonCancel( bool state ) KDE_DEPRECATED;

    /**
     * Sets the page with @p index to be displayed.
     *
     * This method will only
     * work when the dialog is using the predefined shape of TreeList,
     * IconList or Tabbed.
     *
     * @param index Index of the page to be shown.
     * @return @p true if the page is shown, @p false otherwise.
     */
    bool showPage( int index );

    /**
     * Returns the index of the active page.
     *
     * This method will only work when the dialog is using the
     * predefined shape of Tabbed, TreeList or IconList.
     *
     * @return The page index or -1 if there is no active page.
     */
    int activePageIndex() const;


    /**
     * Returns the index of a page created with addPage(),
     * addVBoxPage(), addHBoxPage() or addGridPage().
     * You can can compare this index with the value returned from
     * activePageIndex() if you need to do some page specific actions
     * in your code.
     *
     * The returned index will never change so you can safely use this
     * function once and save the value.
     *
     * @param widget The widget returned by addPage(), addVBoxPage(),
     * addHBoxPage() or addGridPage().
     *
     * @return The index or -1 if the face is not Tabbed, TreeList or
     *         IconList
     */
    int pageIndex( QWidget *widget ) const;


    /**
     * Sets the main user definable widget.
     *
     * If the dialog is using the predefined Swallow mode, the widget will
     * be reparented to the internal swallow control widget. If the dialog
     * is being used in the standard mode then the @p widget must have the
     * dialog as parent.
     *
     * @param widget The widget to be displayed as main widget. If it
     * is 0, then the dialog will show an empty space of 100x100 pixels
     * instead.
     */
    void setMainWidget( QWidget *widget );


    /**
     * Convenience method.
     *
     *  Freezes the dialog size using the minimum size
     * of the dialog. This method should only be called right before
     * show() or exec().
     */
    void disableResize();

    /**
     * Sets the appearance of the OK button.
     *
     * If the default parameters are used
     * (that is, if no KGuiItem is given) KStdGuiItem::ok() is used.
     *
     * @param item KGuiItem.
     * @since 3.2
     * @deprecated  use setButtonGuiItem
     */
    void setButtonOK( const KGuiItem &item = KStdGuiItem::ok() ) KDE_DEPRECATED;


    /**
     * Sets the appearance of the Apply button.
     *
     * If the default parameters are used
     * (that is, if no KGuiItem is given) KStdGuiItem::apply() is used.
     *
     * @param item KGuiItem.
     * @since 3.2
     * @deprecated  use setButtonGuiItem
     */
    void setButtonApply( const KGuiItem &item = KStdGuiItem::apply() ) KDE_DEPRECATED;


    /**
     * Sets the appearance of the Cancel button.
     *
     * If the default parameters are used
     * (that is, if no KGuiItem is given) KStdGuiItem::cancel() is used.
     *
     * @param item KGuiItem.
     * @since 3.2
     * @deprecated  use setButtonGuiItem
     */
    void setButtonCancel( const KGuiItem &item = KStdGuiItem::cancel() ) KDE_DEPRECATED;



    /**
     * This function has only effect in TreeList mode.
     *
     * Defines how the tree list widget is resized when the dialog is
     * resized horizontally. By default the tree list keeps its width
     * when the dialog becomes wider.
     *
     * @param state The resize mode. If false (default) the tree list keeps
     *        its current width when the dialog becomes wider.
     */
     void setTreeListAutoResize( bool state );

    /**
     * This function has only effect in TreeList mode.
     *
     * This tells the widgets whether the icons given in the addPage,
     * addVBoxPage, addHBoxPage, or addGridPage methods should
     * be shown in the TreeList.
     *
     * Note: This method must be called before calling any of the methods
     * which add icons to the page.
     *
     * @param state If true the icons are shown.
     **/
    void setShowIconsInTreeList(bool state);

    /**
     * This function has only effect in TreeList mode.
     *
     * This tells the widgets whether the root should be decorated.
     * For details see QListView::setRootIsDecorated
     *
     * @param state Root will be decorated if true.
     **/
    void setRootIsDecorated( bool state );

    /**
     * This function has only effect in TreeList mode.
     *
     * This tells the TreeList to unfold the whole tree so that all entries
     * are visible.
     *
     * If the list is empty when you call this method newly created entries
     * will not automatically be opened. If the @p persist flag is set opened
     * entries cannot be closed again, though.
     *
     * @param persist If true the tree always stays unfolded.
     * @since 3.2
     */
    void unfoldTreeList( bool persist = false );

    /**
     * Add a widget at the bottom of the TreeList/IconList.
     *
     * @param widget  The widget to be added. It will be reparented into the
     *                KJanusWidget, therefor it will be deleted with the
     *                KJanusWidget, too. To be on the save side just don't keep
     *                the pointer to this widget.
     */
    void addWidgetBelowList( QWidget * widget );

    /**
     * Add a button at the bottom of the TreeList/IconList.
     *
     * @param text     The text on the PushButton.
     * @param recv     The object that is to receive the signal when the button
     *                 is clicked.
     * @param slot     The slot to connect to the clicked signal of the button.
     *
     * @since 3.2
     */
    void addButtonBelowList( const QString & text, QObject * recv, const char * slot );

    /**
     * The same as the above function, but with a KGuiItem providing the text
     * and icon for the button at the bottom of the TreeList/IconList.
     *
     * @param guiitem  The text and icon on the PushButton.
     * @param recv     The object that is to receive the signal when the button
     *                 is clicked.
     * @param slot     The slot to connect to the clicked signal of the button.
     *
     * @since 3.2
     */
    void addButtonBelowList( const KGuiItem & guiitem, QObject * recv, const char * slot );

    /**
     * This function has only effect in IconList mode.
     *
     * Defines how the icon list widget is displayed. By default it is
     * the widgets in the dialog pages that decide the minimum height
     * of the dialog. A vertical scrollbar can be used in the icon list
     * area.
     *
     * @param state The visibility mode. If true, the minimum height is
     *        adjusted so that every icon in the list is visible at the
     *        same time. The vertical scrollbar will never be visible.
     */
    void setIconListAllVisible( bool state );

    /**
     * Check whether the background tile is set or not.
     *
     * @return @p true if there is defined a background tile.
     */
    static bool haveBackgroundTile();

    /**
     * Returns a pointer to the background tile if there is one.
     *
     * @return The tile pointer or 0 if no tile is defined.
     *
     **/
    static const QPixmap *backgroundTile();


    /**
     * Sets the background tile.
     *
     *  If it is Null (0), the background image is deleted.
     *
     * @param pix The background tile.
     */
    static void setBackgroundTile( const QPixmap *pix );

    /**
     * Enable hiding of the background tile (if any).
     *
     * @param state @p true will make the tile visible.
     */
    void showTile( bool state );

    /**
     * Calculate the size hint for the dialog.
     *
     * With this method it is easy to calculate a size hint for a
     * dialog derived from KDialogBase if you know the width and height of
     * the elements you add to the widget. The rectangle returned is
     * calculated so that all elements exactly fit into it. Thus, you may
     * set it as a minimum size for the resulting dialog.
     *
     * You should not need to use this method and never if you use one of
     * the predefined shapes.
     *
     * @param w The width of you special widget.
     * @param h The height of you special widget.
     * @return The minimum width and height of the dialog using @p w and @p h
     * as the size of the main widget.
     */
    QSize calculateSize( int w, int h ) const;


	virtual void accept();
	virtual void reject();

   /**
    * read the dialogs size from the configuration according to the screen size.
    * If no size is saved for one dimension of the screen, sizeHint() is returned.
    *
    * @param groupName Name of the group to read from. The old group
    *                  of KGlobal::config is preserved.
    * @deprecated use restoreDialogSize
    */
   QSize configDialogSize( const QString& groupName ) const KDE_DEPRECATED;

   /**
    * read the dialogs size from the configuration according to the screen size.
    * If no size is saved for one dimension of the screen, sizeHint() is returned.
    *
    * @param config The KConfig object to read from
    * @param groupName Name of the group to read from. The old group
    *                  of KGlobal::config is preserved.
    * @since 3.2
    * @deprecated use restoreDialogSize
    */
   QSize configDialogSize( KConfig& config, const QString& groupName ) const KDE_DEPRECATED;

   /**
    * save the dialogs size dependant on the screen dimension either to the
    * global or application config file.
    *
    * @param groupName The group to which the dialogs size is saved. See configDialogSize
    * to read the size.
    * @param global Set to true if the entry should go to the global config rather
    *        than to the applications config. Default is false.
    * @deprecated use restoreDialogSize( KConfigBase )
    */
   void saveDialogSize( const QString& groupName, bool global=false ) KDE_DEPRECATED;

   /**
    * save the dialogs size dependant on the screen dimension.
    *
    * @param config The KConfig object to write to.
    * @param groupName The group to which the dialogs size is saved. See
    * configDialogSize to read the size.
    * @param global Set to true if the entry should go to the global config.
    *        Default is false.
    * @since 3.2
    * @deprecated use restoreDialogSize( KConfigBase )
    */
   void saveDialogSize( KConfig& config, const QString& groupName,
			     bool global=false ) const KDE_DEPRECATED;



  public slots:

    /**
     * Destruct the Dialog delayed.
     *
     * You can call this function from
     * slots like closeClicked() and hidden().
     * You should not use the dialog any more after
     * calling this function.
     * @since 3.1
     */
    void delayedDestruct();


    /**
     * This method is called automatically whenever the background has
     * changed. You do not need to use this method.
     */
    void updateBackground();


 signals:
    /**
     * Do not use this signal. Is is kept for compatibility reasons.
     * @deprecated Use applyClicked() instead.
     */
    void apply();

    /**
     * The background tile has changed.
     */
    void backgroundChanged();


    /**
     * The current page being shown has changed. This signal is only emitted for the TreeList
     * and IconList faces.
     *
     * \since 4.0
     */
    void currentPageChanged(QWidget *page);


  protected slots:

    /**
     * Deletes the dialog immediately. If you want to delete the dialog
     * delayed use delayedDestruct() or QObject::deleteLater().
     *
     * Attention: Do no use connect this slot to signals from user
     * actions!
     */
    void slotDelayedDestruct();

    /**
     * @deprecated  use accept()
     */
    virtual void slotOk();

    /**
     * @deprecated use reject()
     */
    virtual void slotCancel();
    /**
     * @deprecated use applyClicked signal;
     */
	virtual void slotApply(){};



  private:

    /**
     * Prepares a relay that is used to send signals between
     * all KDialogBase instances of a program. Should only be used in the
     * constructor.
     */
    void makeRelay();

    /**
     * Prints an error message using qDebug if makeMainWidget ,
     * makeVBoxMainWidget , makeHBoxMainWidget or
     * makeGridMainWidget failed.
     */
    void printMakeMainWidgetError();


  private:
    KJanusWidget *mJanus;

    static KDialogBaseTile *mTile;
    bool   mShowTile;

  protected:
    virtual void virtual_hook( int id, void* data );
  private:
    class KDialogBasePrivate;
    KDialogBasePrivate* const d;
};

#endif

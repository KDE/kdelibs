/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 *  Espen Sand (espen@kde.org)
 *  Holger Freyther <freyther@kde.org>
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
 *
 */
#ifndef _KDIALOG_BASE_H_
#define _KDIALOG_BASE_H_

#include <kdialog.h>
#include <kjanuswidget.h>
#include <kguiitem.h>
#include <kstdguiitem.h>
#include <qptrlist.h>

class QPushButton;
class KSeparator;
class KURLLabel;
class QBoxLayout;
class QPixmap;
class KGuiItem;
/**
 * Used internally by @ref KDialogBase.
 * @internal
 */
class KDialogBaseButton;

/**
 * Used internally by @ref KDialogBase.
 * @internal
 */
class KDialogBaseTile;

/**
 * Provides basic functionality needed by nearly all dialogs.
 *
 * It offers the standard action buttons you'd expect to find in a
 * dialog as well as the capability to define at most three configurable
 * buttons. You can define a main widget that contains your specific
 * dialog layout or you can use a predefined layout. Currently, @p
 * TreeList/Paged, @p Tabbed, @p Plain, @p Swallow and @p IconList
 * mode layouts (faces) are available.
 *
 * The class takes care of the geometry management. You only need to define
 * a minimum size for the widget you want to use as the main widget.
 *
 * You can set a background tile (pixmap) for parts of the dialog. The
 * tile you select is shared by all instances of this class in your
 * application so that they all get the same look and feel.
 *
 * There is a tutorial available on http://developer.kde.org/ (NOT YET)
 * that contains
 * copy/paste examples as well a screenshots on how to use this class.
 *
 * @sect Standard buttons (action buttons):
 *
 * You select which buttons should be displayed, but you do not choose the
 * order in which they are displayed. This ensures a standard interface in
 * KDE. The button order can be changed, but this ability is only available
 * for a central KDE control tool. The following buttons are available:
 * OK, Cancel/Close, Apply/Try, Default, Help and three user definable
 * buttons: User1, User1 and User3. You must specify the text of the UserN
 * buttons. Each button has a virtual slot so you can overload the method
 * when required. The default slots emit a signal as well, so you can choose
 * to connect a signal instead of overriding the slot.
 * The default implementation of @ref slotHelp() will automatically enable
 * the help system if you have provided a path to the help text.
 * @ref slotCancel() and @ref slotClose() will run @ref QDialog::reject()
 * while @ref slotOk() will run @ref QDialog::accept(). You define a default
 * button in the constructor.
 *
 * If you don't want any buttons at all because your dialog is special
 * in some way, then set the buttonMask argument in the constructor to zero
 * (0). The optional button box separator line should not be enabled
 * in this case. Note that the KDialogBase will animate a button press
 * when the user press Escape. The button that is enabled is either Cancel,
 * Close or the button that is defined by @ref setEscapeButton() The
 * animation will not take place when the buttonMask is zero. Your
 * custom dialog code should reimplement the @ref keyPressEvent and
 * animate the cancel button so that the dialog behaves like regular
 * dialogs. NOTE: None of the regular slots (like @ref slotOk() ) or
 * signals that are related to the standard action buttons will be used
 * when you don't use these buttons.
 *
 * @sect Dialog shapes:
 *
 * You can either use one of the prebuilt, easy to use, faces or
 * define your own main widget. The dialog provides ready to use
 * TreeList, Tabbed, Plain, Swallow and IconList faces. KDialogBase uses
 * the @ref KJanusWidget class internally to accomplish this. If you
 * use TreeList, Tabbed or IconList mode, then add pages with @ref addPage().
 *
 * Pages that have been added can be removed again by simply deleting
 * the page.
 *
 * If you want complete control of how the dialog contents should look,
 * then you can define a main widget by using @ref setMainWidget(). You
 * only need to set the minimum size of that widget and the dialog will
 * resize itself to fit this minimum size.  The dialog is resizeable, but
 * cannot be made smaller than its minimum size.
 *
 * @sect Layout:
 *
 * The dialog consists of a help area on top (becomes visible if you define
 * a help path and use @ref enableLinkedHelp()), the main area which is
 * the built-in dialog face or your own widget in the middle and by default
 * a button box at the bottom. The button box can also be placed at the
 * right edge (to the right of the main widget). Use
 * @ref setButtonBoxOrientation() to control this behavior. A separator
 * can be placed above the button box (or to the left when the button box
 * is at the right edge). Normally you specify that you want a separator
 * in the constructor, but you can use @ref enableButtonSeparator() as well.
 *
 * @sect Standard compliance:
 *
 * The class is derived from @ref KDialog, so you get automatic access to
 * the @ref KDialog::marginHint(), @ref KDialog::spacingHint() and the
 * extended @ref KDialog::setCaption() method. NOTE: The main widget you
 * use will be positioned inside the dialog using a margin (or border)
 * equal to @ref KDialog::marginHint(). You should not add a margin yourself.
 * The example below (from kedit) shows how you use the top level widget
 * and its layout. The second argument (the border) to @ref QVBoxLayout
 * is 0. This situation is valid for @ref addPage , @ref addVBoxPage ,
 * @ref addHBoxPage , @ref addGridPage , @ref makeMainWidget ,
 * @ref makeVBoxMainWidget , @ref makeHBoxMainWidget and
 * @ref makeGridMainWidget as well.
 *
 * Example:
 *
 * <pre>
 * UrlDlg::UrlDlg( QWidget *parent, const QString& caption,
 *                 const QString& urltext)
 *   : KDialogBase( parent, "urldialog", true, caption,
 *		    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
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
 * </pre>
 *
 * If you use @ref makeVBoxMainWidget(), then the dialog above can be made
 * simpler but you lose the ability to add a stretchable area:
 *
 * <pre>
 * UrlDlg::UrlDlg( QWidget *parent, const QString& caption,
 *		const QString& urltext)
 * : KDialogBase( parent, "urldialog", true, caption,
		  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
 * {
 *   QVBox *page = makeVBoxMainWidget();
 *   QLabel *label = new QLabel( caption, page, "caption" );
 *
 *   lineedit = new QLineEdit( urltext, page, "lineedit" );
 *   lineedit->setMinimumWidth(fontMetrics().maxWidth()*20);
 * }
 * </pre>
 *
 * This class can be used in many ways. Note that most KDE ui widgets
 * and many of KDE core applications use the KDialogBase so for more
 * inspiration you should study the code for these.
 *
 * @short A dialog base class with standard buttons and predefined layouts.
 * @author Mirko Boehm (mirko@kde.org) and Espen Sand (espen@kde.org)
 */
class KDialogBase : public KDialog
{
  Q_OBJECT

  public:

    /**
     *  @li @p Help -    Show Help-button.
     *  @li @p Default - Show Default-button.
     *  @li @p Details - Show Details-button.
     *  @li @p Ok -      Show Ok-button.
     *  @li @p Apply -   Show Apply-button.
     *  @li @p Try -     Show Try-button.
     *  @li @p Cancel -  Show Cancel-button.
     *  @li @p Close -   Show Close-button.
     *  @li @p User1 -   Show User define-button 1.
     *  @li @p User2 -   Show User define-button 2.
     *  @li @p User3 -   Show User define-button 3.
     *  @li @p No -      Show No-button.
     *  @li @p Yes -     Show Yes-button.
     *  @li @p Stretch - Used internally. Ignored when used in a constructor.
     *  @li @p Filler  - Used internally. Ignored when used in a constructor.
     */
    enum ButtonCode
    {
      Help    = 0x00000001,
      Default = 0x00000002,
      Ok      = 0x00000004,
      Apply   = 0x00000008,
      Try     = 0x00000010,
      Cancel  = 0x00000020,
      Close   = 0x00000040,
      User1   = 0x00000080,
      User2   = 0x00000100,
      User3   = 0x00000200,
      No      = 0x00000080,
      Yes     = 0x00000100,
      Details = 0x00000400,
      Filler  = 0x40000000,
      Stretch = 0x80000000
    };

    enum ActionButtonStyle
    {
      ActionStyle0=0, // KDE std
      ActionStyle1,
      ActionStyle2,
      ActionStyle3,
      ActionStyle4,
      ActionStyleMAX
    };

    /**
     *  @li @p TreeList - A dialog with a tree on the left side and a
     *                    representation of the contents on the right side.
     *  @li @p Tabbed -   A dialog using a @ref QTabWidget.
     *  @li @p Plain -    A normal dialog.
     *  @li @p Swallow
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
     * Constructor for the standard mode where you must specify the main
     * widget with @ref setMainWidget() .
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
     *        the default.
     * @param separator If @p true, a separator line is drawn between the
     *        action buttons and the main widget.
     * @param user1 User button1 item.
     * @param user2 User button2 item.
     * @param user3 User button3 item.
     */
    KDialogBase( QWidget *parent=0, const char *name=0, bool modal=true,
                 const QString &caption=QString::null,
		 int buttonMask=Ok|Apply|Cancel, ButtonCode defaultButton=Ok,
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
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies which buttons will be visible. If zero
     *        (0) no button box will be made.
     * @param defaultButton Specifies which button will be marked as
     *        the default.
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
     * Constructor for a message box mode where the @p buttonMask can only
     * contain Yes, No, or Cancel.
     *
     * If you need other names you can rename
     * the buttons with @ref setButtonText(). The dialog box is not resizable
     * by default but this can be changed by @ref setInitialSize(). If you
     * select 'modal' to be true, the dialog will return Yes, No, or Cancel
     * when closed otherwise you can use the signals @ref yesClicked(),
     * @ref noClicked(), or @ref cancelClicked() to determine the state.
     *
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies which buttons will be visible. If zero
     *        (0) no button box will be made.
     * @param defaultButton Specifies which button will be marked as
     *        the default.
     * @param escapeButton Specifies which button will be activated by
     *        when the dialog receives a @p Key_Escape keypress.
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only).
     * @param modal Controls dialog modality. If @p false, the rest of the
     *        program interface (example: other dialogs) is accessible
     *        while the dialog is open.
     * @param separator If @p true, a separator line is drawn between the
     *        action buttons and the main widget.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user3 User button3 text.
     */
    KDialogBase( const QString &caption, int buttonMask=Yes|No|Cancel,
		 ButtonCode defaultButton=Yes, ButtonCode escapeButton=Cancel,
		 QWidget *parent=0, const char *name=0,
                 bool modal=true, bool separator=false,
		 const KGuiItem &yes = KStdGuiItem::yes(), // i18n("&Yes")
		 const KGuiItem &no = KStdGuiItem::no(), // i18n("&No"),
		 const KGuiItem &cancel = KStdGuiItem::cancel() // i18n("&Cancel")
	         );

    /**
     * Destructor.
     */
    ~KDialogBase();

    /**
     * Sets the orientation of the button box.
     *
     * It can be @p Vertical or @p Horizontal. If @p Horizontal
     * (default), the button box is positioned at the bottom of the
     * dialog. If @p Vertical it will be placed at the right edge of the
     * dialog.
     *
     * @param orientation The button box orientation.
     */
    void setButtonBoxOrientation( int orientation );

    /**
     * Sets the button that will be activated when the Escape key
     * is pressed.
     *
     * Normally you should not use this function. By default,
     * the Escape key is mapped to either the Cancel or the Close button
     * if one of these buttons are defined. The user expects that Escape will
     * cancel an operation so use this function with caution.
     *
     * @param id The button code.
     */
    void setEscapeButton( ButtonCode id );


    /**
     * Adjust the size of the dialog to fit the contents just before
     * @ref QDialog::exec() or @ref QDialog::show() is called.
     *
     * This method will not be called if the dialog has been explicitly
     * resized before showing it.
     **/
    virtual void adjustSize();
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    /**
     * Retrieve the empty page when the predefined layout is used in @p Plain
     * mode.
     *
     * This widget must used as the toplevel widget of your dialog
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
     * <pre>
     * QFrame *page = addPage( i18n("Layout") );
     * QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 6 );
     * QLabel *label = new QLabel( i18n("Layout type"), page );
     * topLayout->addWidget( label );
     * ..
     * </pre>
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
                      const QString &header=QString::null,
		      const QPixmap &pixmap=QPixmap() );

    /**
     * This is like @ref addPage() just above, with the difference that the first
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
                      const QString &header=QString::null,
		      const QPixmap &pixmap=QPixmap() );

    /**
     * Add a page to the dialog when the class is used in @p TreeList,
     * @p IconList or @p Tabbed mode.
     *
     * The returned widget must be used as the toplevel widget for
     * this particular page. The widget contains a @ref QVBoxLayout
     * layout so the widget children are lined up vertically.  You can
     * use it as follows:
     *
     * <pre>
     * QVBox *page = addVBoxPage( i18n("Layout") );
     * QLabel *label = new QLabel( i18n("Layout type"), page );
     * ..
     * </pre>
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
    QVBox *addVBoxPage( const QString &itemName,
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * This is like @ref addVBoxPage() just above, with the difference
     * that the first element is a list of strings.
     *
     * These strings are used to form a path
     * of folders down to the given page. The initial elements are names
     * for the folders, while the last element is the name of the page.
     * Note: This does yet only work for the @p TreeList face. Later this may
     * be added for the @p IconList face too. In other faces than the
     * @p TreeList, all the strings except the last one is ignored.
     **/
    QVBox *addVBoxPage( const QStringList &items,
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * Add a page to the dialog when the class is used in @p TreeList,
     * @p IconList or @p Tabbed mode.
     *
     * The returned widget must be used as the
     * toplevel widget for this particular page. The widget contains a
     * @ref QHBoxLayout layout so the widget children are lined up horizontally.
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
    QHBox *addHBoxPage( const QString &itemName,
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * This is like @ref addHBoxPage() just above, with the
     * difference that the first element is a list of strings.
     *
     * These strings are used to form a path
     * of folders down to the given page. The initial elements are names
     * for the folders, while the last element is the name of the page.
     * Note: This does yet only work for the @p TreeList face. Later this may
     * be added for the @p IconList face too. In other faces than the
     * @p TreeList, all the strings except the last one is ignored.
     **/
    QHBox *addHBoxPage( const QStringList &items,
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );

    /**
     * Add a page to the dialog when the class is used in @p TreeList,
     * @p IconList or @p Tabbed mode.
     *
     * The returned widget must be used as the
     * toplevel widget for this particular page. The widget contains a
     * @ref QGridLayout layout so the widget children are  positioned in a grid.
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
    QGrid *addGridPage( int n, Orientation dir,
			const QString &itemName,
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );


    /**
     * This is like @ref addGridPage() just above, with the difference
     * that the first element is a list of strings.
     *
     * These strings are used to form a path
     * of folders down to the given page. The initial elements are names
     * for the folders, while the last element is the name of the page.
     * Note: This does yet only work for the @p TreeList face. Later this may
     * be added for the @p IconList face too. In other faces than the
     * @p TreeList, all the strings except the last one is ignored.
     **/
    QGrid *addGridPage( int n, Orientation dir,
			const QStringList &items,
			const QString &header=QString::null,
			const QPixmap &pixmap=QPixmap() );


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
     * The function will make a @ref QFrame widget
     * and use @ref setMainWidget() to register it. You can @em not use this
     * function more than once, @em not if you have already defined a
     * main widget with @ref setMainWidget() and @em not if you have used the
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
     * The function will make a @ref QVBox widget
     * and use @ref setMainWidget() to register it. You @em can use this
     * function more than once, @em not if you have already defined a
     * main widget with @ref setMainWidget() and @em not if you have used the
     * constructor where you define the face (@p Plain, @p Swallow, @p Tabbed,
     * @p TreeList, @p IconList).
     *
     * @return The main widget or 0 if any of the rules described above
     *         were broken.
     */
    QVBox *makeVBoxMainWidget();

    /**
     * Make a main widget.
     *
     * The function will make a @ref QHBox widget
     * and use @ref setMainWidget() to register it. You can @em not use this
     * function more than once, @em not if you have already defined a
     * main widget with @ref setMainWidget() and @p not if you have used the
     * constructor where you define the face (@p Plain, @p Swallow, @p Tabbed,
     * @p TreeList, @p IconList).
     *
     * @return The main widget or 0 if any of the rules described above
     *         were broken.
     */
    QHBox *makeHBoxMainWidget();

    /**
     * Make a main widget.
     *
     * The function will make a @ref QGrid widget
     * and use @ref setMainWidget() to register it. You can @em not use this
     * function more than once, @em not if you have already defined a
     * main widget with @ref #setMainWidget and @em not if you have used the
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
    QGrid *makeGridMainWidget( int n, Orientation dir );


    /**
     * Hide or display the a separator line drawn between the action
     * buttons an the main widget.
     */
    void enableButtonSeparator( bool state );

    /**
     * Hide or display a general action button.
     *
     *  Only buttons that have
     * been created in the constructor can be displayed. This method will
     * not create a new button.
     *
     * @param id Button identifier.
     * @param state true display the button(s).
     */
    void showButton( ButtonCode id, bool state );

    /**
     * Hide or display the OK button.
     *
     *  The OK button must have
     * been created in the constructor to be displayed.
     *
     * @param state If @p true, display the button(s).
     */
    void showButtonOK( bool state );

    /**
     * Hide or display the Apply button.
     *
     *  The Apply button must have
     * been created in the constructor to be displayed.
     *
     * @param state true display the button(s).
     */
    void showButtonApply( bool state );

    /**
     * Hide or display the Cancel button. The Cancel button must have
     * been created in the constructor to be displayed.
     *
     * @param state @p true display the button(s).
     */
    void showButtonCancel( bool state );

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
     * Returns the index of a page created with @ref #addPage(),
     * @ref #addVBoxPage(), @ref addHBoxPage() or @ref #addGridPage().
     * You can can compare this index with the value returned from
     * @ref #activePageIndex() if you need to do some page specific actions
     * in your code.
     *
     * The returned index will never change so you can safely use this
     * function once and save the value.
     *
     * @param widget The widget returned by @ref #addPage(), @ref #addVBoxPage(),
     *        @ref #addHBoxPage() or @ref #addGridPage().
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
     * Returns the main widget if any.
     *
     * @return The current main widget. Can be 0 if no widget has been defined.
     */
    QWidget *mainWidget();

    /**
     * Convenience method.
     *
     *  Freezes the dialog size using the minimum size
     * of the dialog. This method should only be called right before
     * @ref show() or @ref exec().
     */
    void disableResize();

    /**
     * Convenience method. Sets the initial dialog size.
     *
     *  This method should
     * only be called right before @ref show() or @ref exec(). The initial
     * size will be
     * ignored if smaller than the dialog's minimum size.
     *
     * @param s Startup size.
     * @param noResize If @p true the dialog cannot be resized.
     */
    void setInitialSize( const QSize &s, bool noResize=false );

    /**
     * Convenience method. Add a size to the default minimum size of a
     * dialog.
     *
     * This method should only be called right before @ref show() or
     * @ref exec().
     *
     * @param s  Size added to minimum size.
     * @param noResize If @p true the dialog cannot be resized.
     */
    void incInitialSize( const QSize &s, bool noResize=false );

   /**
    * read the dialogs size from the configuration according to the screen size.
    * If no size is saved for one dimension of the screen, a default size
    * is choosed. The default width is 50 percent of the screen width, the
    * default height is 40 percent of the screen height.
    *
    * @param groupName Name of the group to read from. The old group
    *                  of KGlobal::config is preserved.
    */
   QSize configDialogSize( const QString& groupName ) const;

   /**
    * save the dialogs size dependant on the screen dimension either to the
    * global or application config file.
    *
    * @param The group to which the dialogs size is saved. See @ref configDialogSize
    * to read the size.
    * @param global Set to true if the entry should go to the global config rather
    *        than to the applications config. Default is false.
    */
   void saveDialogSize( const QString& groupName, bool global=false );

    /**
     * Sets the text of the OK button.
     *
     * If the default parameters are used
     * (that is, if no parameters are given) the standard texts are set:
     * The button shows "OK", the tooltip contains "Accept settings."
     * (internationalized) and the quickhelp text explains the standard
     * behavior of the OK button in settings dialogs.
     *
     * @param text Button text.
     * @param tooltip Tooltip text.
     * @param quickhelp Quick help text.
     */
    void setButtonOKText( const QString &text=QString::null,
			  const QString &tooltip=QString::null,
			  const QString &quickhelp=QString::null );

    /**
     * Sets the text of the Apply button.
     *
     * If the default parameters are
     * used (that is, if no parameters are given) the standard texts are set:
     * The button shows "Apply", the tooltip contains "Apply settings."
     * (internationalized) and the quickhelp text explains the standard
     * behavior of the apply button in settings dialogs.
     *
     * @param text Button text.
     * @param tooltip Tooltip text.
     * @param quickhelp Quick help text.
     */
    void setButtonApplyText( const QString &text=QString::null,
			     const QString &tooltip=QString::null,
			     const QString &quickhelp=QString::null );

    /**
     * Sets the text of the Cancel button.
     *
     *  If the default parameters are
     * used (that is, if no parameters are given) the standard texts are set:
     * The button shows "Cancel", everything else will not be set.
     *
     * @param text Button text.
     * @param tooltip Tooltip text.
     * @param quickhelp Quick help text.
     */
    void setButtonCancelText( const QString &text=QString::null,
			      const QString &tooltip=QString::null,
			      const QString &quickhelp=QString::null );

    /**
     * Sets the text of any button.
     *
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonText( ButtonCode id, const QString &text );

    /**
     * Sets the tooltip text of any button.
     *
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonTip( ButtonCode id, const QString &text );

    /**
     * Sets the "What's this?" text of any button.
     *
     * @param id The button identifier.
     * @param text Button text.
     */
    void setButtonWhatsThis( ButtonCode id, const QString &text );


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
     * This tells the widgets whether the icons given in the @ref addPage,
     * @ref addVBoxPage, @ref addHBoxPage, or @ref addGridPage methods should
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
     * For details see @ref QListView::setRootIsDecorated
     *
     * @param state Root will be decorated if true.
     **/
    void setRootIsDecorated( bool state );

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
     * @deprecated
     * Use @ref backgroundTile() instead.
     */
    static const QPixmap *getBackgroundTile();

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
     * @deprecated
     * Do not use this method. It is included for compatibility reasons.
     *
     * This method returns the border widths in all directions the dialog
     * needs for itself. Respect this, or get bad looking results.
     * The references are upper left x (@p ulx), upper left y (@p uly),
     * lower right x (@p lrx), and lower left y (@p lly).
     * The results are differences in pixels from the
     * dialogs corners.
     */
    void  getBorderWidths( int& ulx, int& uly, int& lrx, int& lry ) const;

    /**
     * @deprecated
     * Do not use this method. It is included for compatibility reasons.
     *
     * This method returns the contents rectangle of the work area. Place
     * your widgets inside this rectangle, and use it to set up
     * their geometry. Be careful: The rectangle is only valid after
     * resizing the dialog, as it is a result of the resizing process.
     * If you need the "overhead" the dialog needs for its elements,
     * use @ref getBorderWidths().
     */
    QRect getContentsRect() const;

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

    /**
     * Returns the help link text.
     *
     *  If no text has been defined,
     * "Get help..." (internationalized) is returned.
     *
     * @return The help link text.
     */
    QString helpLinkText() const;

    /**
     * Returns the action button that corresponds to the @p id.
     *
     * Normally
     * you should not use this function. @em Never delete the object returned
     * by this function. See also @ref enableButton(), @ref showButton(),
     * @ref setButtonTip(), @ref setButtonWhatsThis(), and @ref setButtonText().
     *
     * @param id Integer identifier of the button.
     * @return The action button or 0 if the button does not exists.
     *
     */
    QPushButton *actionButton( ButtonCode id );

  public slots:
    /**
     * Enable or disable (gray out) a general action button.
     *
     * @param id Button identifier.
     * @param state @p true enables the button(s).
     */
    void enableButton( ButtonCode id, bool state );

    /**
     * Enable or disable (gray out) the OK button.
     *
     * @param state @p true enables the button.
     */
    void enableButtonOK( bool state );

    /**
     * Enable or disable (gray out) the Apply button.
     *
     * @param state true enables the button.
     */
    void enableButtonApply( bool state );

    /**
     * Enable or disable (gray out) the Cancel button.
     *
     * @param state true enables the button.
     */
    void enableButtonCancel( bool state );

    /**
     * Display or hide the help link area on the top of the dialog.
     *
     * @param state @p true will display the area.
     */
    void enableLinkedHelp( bool state );

    /**
     * Destruct the Dialog delayed.
     *
     * You can call this function from
     * slots like @ref closeClicked() and @ref hidden().
     * You should not use the dialog any more after
     * calling this function.
     * @since 3.1
     */
    void delayedDestruct();

    /**
     * Sets the text that is shown as the linked text.
     *
     * If text is empty,
     * the text "Get help..." (internationalized) is used instead.
     *
     * @param text The link text.
     */
    void setHelpLinkText( const QString &text );

    /**
     * Sets the help path and topic.
     *
     * @param anchor Defined anchor in your docbook sources
     * @param appname Defines the appname the help belongs to
     *                If empty it's the current one
     */
    void setHelp( const QString &anchor,
		  const QString &appname = QString::null );

    /**
     * Connected to help link label.
     */
    void helpClickedSlot( const QString & );

    /**
     * Sets the status of the Details button.
     */
    void setDetails(bool showDetails);

    /**
     * Sets the widget that gets shown when "Details" is enabled.
     *
     * The dialog takes over ownership of the widget.
     * Any previously set widget gets deleted.
     */
    void setDetailsWidget(QWidget *detailsWidget);

    /**
     * This method is called automatically whenever the background has
     * changed. You do not need to use this method.
     */
    void updateBackground();

    /**
     * Force closing the dialog, setting its result code to the one Esc would set.
     * You shouldn't use this, generally (let the user make his choice!)
     * but it can be useful when you need to make a choice after a timeout
     * has happened, or when the parent widget has to go somewhere else
     * (e.g. html redirections).
     * @since 3.1
     */
    void cancel();

  signals:
    /**
     * The Help button was pressed. This signal is only emitted if
     * @ref slotHelp() is not replaced.
     */
    void helpClicked();

    /**
     * The Default button was pressed. This signal is only emitted if
     * @ref slotDefault() is not replaced.
     */
    void defaultClicked();


    /**
     * The User3 button was pressed. This signal is only emitted if
     * @ref slotUser3() is not replaced.
     */
    void user3Clicked();

    /**
     * The User2 button was pressed. This signal is only emitted if
     * @ref slotUser2() is not replaced.
     */
    void user2Clicked();

    /**
     * The User1 button was pressed. This signal is only emitted if
     * @ref slotUser1() is not replaced.
     */
    void user1Clicked();

    /**
     * The Apply button was pressed. This signal is only emitted if
     * @ref slotApply() is not replaced.
     */
    void applyClicked();

    /**
     * The Try button was pressed. This signal is only emitted if
     * @ref slotTry() is not replaced.
     */
    void tryClicked();

    /**
     * The OK button was pressed. This signal is only emitted if
     * @ref slotOk() is not replaced.
     */
    void okClicked();

    /**
     * The Yes button was pressed. This signal is only emitted if
     * @ref slotYes() is not replaced.
     */
    void yesClicked();

    /**
     * The No button was pressed. This signal is only emitted if
     * @ref slotNo() is not replaced.
     */
    void noClicked();

    /**
     * The Cancel button was pressed. This signal is only emitted if
     * @ref slotCancel() is not replaced.
     */
    void cancelClicked();

    /**
     * The Close button was pressed. This signal is only emitted if
     * @ref slotClose() is not replaced.
     */
    void closeClicked();

    /**
     * Do not use this signal. Is is kept for compatibility reasons.
     * Use @ref applyClicked() instead.
     */
    void apply();

    /**
     * The background tile has changed.
     */
    void backgroundChanged();

    /**
     * The dialog is about to be hidden.
     *
     * A dialog is hidden after a user clicks a button that ends
     * the dialog or when the user switches to another desktop or
     * minimizes the dialog.
     */
    void hidden();

    /**
     * The dialog has finished.
     *
     * A dialog emits finished after a user clicks a button that ends
     * the dialog.
     *
     * This signal is also emitted when you call @ref hide()
     *
     * If you have stored a pointer to the
     * dialog do @em not try to delete the pointer in the slot that is
     * connected to this signal.
     *
     * You should use @ref delayedDestruct() instead.
     */
    void finished();

    /**
     * The detailsWidget is about to get shown. This is your last chance
     * to call setDetailsWidget if you haven't done so yet.
     */
    void aboutToShowDetails();

    /**
     * A page is about to be shown.
     */
    void aboutToShowPage(QWidget *page);

  protected:
    /**
     * Maps some keys to the actions buttons. F1 is mapped to the Help
     * button if present and Escape to the Cancel or Close if present. The
     * button action event is animated.
     */
    virtual void keyPressEvent( QKeyEvent *e );

    /**
     * Emits the #hidden signal. You can connect to that signal to
     * detect when a dialog has been closed.
     */
    virtual void hideEvent( QHideEvent * );

    /**
     * Detects when a dialog is being closed from the window manager
     * controls. If the Cancel or Close button is present then the button
     * is activated. Otherwise standard @ref QDialog behavior
     * will take place.
     */
    virtual void closeEvent( QCloseEvent *e );

  protected slots:
    /**
     * Activated when the Help button has been clicked. If a help
     * text has been defined, the help system will be activated.
     */
    virtual void slotHelp();

    /**
     * Activated when the Default button has been clicked.
     */
    virtual void slotDefault();

    /**
     * Activated when the Details button has been clicked.
     * @see detailsClicked(bool)
     */
    virtual void slotDetails();

    /**
     * Activated when the User3 button has been clicked.
     */
    virtual void slotUser3();

    /**
     * Activated when the User2 button has been clicked.
     */
    virtual void slotUser2();

    /**
     * Activated when the User1 button has been clicked.
     */
    virtual void slotUser1();

    /**
     * Activated when the Ok button has been clicked. The
     * @ref QDialog::accept() is activated.
     */
    virtual void slotOk();

    /**
     * Activated when the Apply button has been clicked.
     */
    virtual void slotApply();

    /**
     * Activated when the Try button has been clicked.
     */
    virtual void slotTry();

    /**
     * Activated when the Yes button has been clicked. The
     * @ref QDialog::done( Yes ) is activated.
     */
    virtual void slotYes();

    /**
     * Activated when the Yes button has been clicked. The
     * @ref QDialog::done( No ) is activated.
     */
    virtual void slotNo();

    /**
     * Activated when the Cancel button has been clicked. The
     * @ref QDialog::reject() is activated in regular mode and
     * @ref QDialog::done( Cancel ) when in message box mode.
     */
    virtual void slotCancel();

    /**
     * Activated when the Close button has been clicked. The
     * @ref QDialog::reject() is activated.
     */
    virtual void slotClose();

    /**
     * Do not use this slot. Is is kept for compatibility reasons.
     * Activated when the Apply button has been clicked
     */
    virtual void applyPressed();

    /**
     * Updates the margins and spacings.
     */
    void updateGeometry();

    /**
     * Deletes the dialog immediately. If you want to delete the dialog
     * delayed use delayedDestruct() or QObject::deleteLater().
     *
     * Attention: Do no use connect this slot to signals from user
     * actions!
     */
    void slotDelayedDestruct();

  private:
    /**
     * Prepares the layout that manages the widgets of the dialog
     */
    void setupLayout();

    /**
     * Prepares a relay that is used to send signals between
     * all KDialogBase instances of a program. Should only be used in the
     * constructor.
     */
    void makeRelay();

    /**
     * Makes the button box and all the buttons in it. This method must
     * only be ran once from the constructor.
     *
     * @param buttonMask Specifies what buttons will be made.
     * @param defaultButton Specifies what button we be marked as the
     * default.
     * @param user1 User button1 item.
     * @param user2 User button2 item.
     * @param user2 User button3 item.
     */
    void makeButtonBox( int mask, ButtonCode defaultButton,
			const KGuiItem &user1 = KGuiItem(),
			const KGuiItem &user2 = KGuiItem(),
			const KGuiItem &user3 = KGuiItem() );

    /**
     * Sets the action button that is marked as default and has focus.
     *
     * @param p The action button.
     * @param isDefault If true, make the button default
     * @param isFocus If true, give the button focus.
     */
    void setButtonFocus( QPushButton *p, bool isDefault, bool isFocus );

    /**
     * Prints an error message using qDebug if @ref makeMainWidget ,
     * @ref makeVBoxMainWidget , @ref makeHBoxMainWidget or
     * @ref makeGridMainWidget failed.
     */
    void printMakeMainWidgetError();

  private slots:
    /**
     * Sets the action button order according to the 'style'.
     *
     * @param style The style index.
     */
    void setButtonStyle( int style );


  private:
    QBoxLayout   *mTopLayout;
    QWidget      *mMainWidget;
    KURLLabel    *mUrlHelp;
    KJanusWidget *mJanus;
    KSeparator   *mActionSep;

    bool mIsActivated;

    QString mAnchor;
    QString mHelpApp;
    QString mHelpLinkText;

    static KDialogBaseTile *mTile;
    bool   mShowTile;

    bool mMessageBoxMode;
    int  mButtonOrientation;
    ButtonCode mEscapeButton;

  protected:
    virtual void virtual_hook( int id, void* data );
  private:
    class KDialogBasePrivate;
    KDialogBasePrivate *d;
};

#endif

/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *                      and Holger Freyther <freyther@kde.org>
 *             2005-2006   Olivier Goffart <ogoffart @ kde.org>
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
 */

#ifndef _KDIALOG_H_
#define _KDIALOG_H_

class QLayoutItem;

class KPushButton;
class KSeparator;
class KURLLabel;
class QBoxLayout;
class QPixmap;

#include <QDialog>
#include <kguiitem.h>
#include <kstdguiitem.h>
#include <kdelibs_export.h>
#include <Qt>

/**
 * Used internally by KDialog.
 * @internal
 */
class KDialogButton;

/**
 * Used internally by KDialog.
 * @internal
 */
class KDialogTile;

/* TODO: the old KDialog had this property:

 * If the dialog is non-modal and has a parent, the default keybindings
 * (@p escape = @p reject(), @p enter = @p accept(), etc.) are disabled.

 */



/**
 * @short A dialog base class with standard buttons and predefined layouts.
 *
 * Provides basic functionality needed by nearly all dialogs.
 *
 * It offers the standard action buttons you'd expect to find in a
 * dialog as well as the capability to define at most three configurable
 * buttons. You can define a main widget that contains your specific
 * dialog layout 
 *
 * The class takes care of the geometry management. You only need to define
 * a minimum size for the widget you want to use as the main widget.
 *
 * By default, the dialog is modal
 *
 * <b>Standard buttons (action buttons):</b>\n
 *
 * You select which buttons should be displayed, but you do not choose the
 * order in which they are displayed. This ensures a standard interface in
 * KDE. The button order can be changed, but this ability is only available
 * for a central KDE control tool. The following buttons are available:
 * OK, Cancel/Close, Apply/Try, Default, Help and three user definable
 * buttons: User1, User2 and User3. You must specify the text of the UserN
 * buttons. Each button has a virtual slot so you can overload the method
 * when required. The default slots emit a signal as well, so you can choose
 * to connect a signal instead of overriding the slot.
 * The default implementation of slotHelp() will automatically enable
 * the help system if you have provided a path to the help text.
 * slotCancel() and slotClose() will run QDialog::reject()
 * while slotOk() will run QDialog::accept(). You define a default
 * button in the constructor.
 *
 * If you don't want any buttons at all because your dialog is special
 * in some way, then set the buttonMask argument in the constructor to zero
 * (0). The optional button box separator line should not be enabled
 * in this case. Note that the KDialog will animate a button press
 * when the user press Escape. The button that is enabled is either Cancel,
 * Close or the button that is defined by setEscapeButton() The
 * animation will not take place when the buttonMask is zero. Your
 * custom dialog code should reimplement the keyPressEvent and
 * animate the cancel button so that the dialog behaves like regular
 * dialogs. NOTE: None of the regular slots (like slotOk() ) or
 * signals that are related to the standard action buttons will be used
 * when you don't use these buttons.
 *
 * <b>Layout:</b>\n
 *
 * The dialog consists of a help area on top (becomes visible if you define
 * a help path and use enableLinkedHelp()), the main area which is
 * the built-in dialog face or your own widget in the middle and by default
 * a button box at the bottom. The button box can also be placed at the
 * right edge (to the right of the main widget). Use
 * setButtonBoxOrientation() to control this behavior. A separator
 * can be placed above the button box (or to the left when the button box
 * is at the right edge). Normally you specify that you want a separator
 * in the constructor, but you can use enableButtonSeparator() as well.
 *
 * <b>Standard compliance:</b>\n
 *
 * The marginHint() and spacingHint() sizes shall be used
 * whenever you layout the interior of a dialog. One special note. If
 * you make your own action buttons (OK, Cancel etc), the space
 * beteween the buttons shall be spacingHint(), whereas the space
 * above, below, to the right and to the left shall be marginHint().
 * If you add a separator line above the buttons, there shall be a
 * marginHint() between the buttons and the separator and a
 * marginHint() above the separator as well.
 *
 * <b>Example:</b>\n
 *
 * \code
 *   KDialog *dialog=new KDialog( this , i18n( "foo" )  ,
 *                                 KDialog::Ok | KDialog::Cancel | KDialog::Apply );
 *   FooWidget wid=new FooWidget( dialog );
 *   dialog->setMainWidget( wid );
 *   connect( dialog , SIGNAL(applyClicked()) , wid , SLOT(save()) );
 *   connect( dialog , SIGNAL(okClicked()) , wid , SLOT(save()) );
 *   connect( wid , SIGNAL(changed(bool)) , wid , SLOT(enabledButtonApply(bool)) );
 *   enableButtonApply(false);
 *   dialog->show();
 * \endcode
 *
 *
 * This class can be used in many ways. Note that most KDE ui widgets
 * and many of KDE core applications use the KDialogBase so for more
 * inspiration you should study the code for these.
 *
 *
 * @see KDialogBase
 * @author Thomas Tanghus <tanghus@earthling.net>
 * @author Espen Sand <espensa@online.no>
 * @author Mirko Boehm <mirko@kde.org> 
 * @author Olivier Goffart <ogoffart at kde.org>
 */
class KDEUI_EXPORT KDialog : public QDialog
{
  Q_OBJECT

  public:

    enum ButtonCode
    {
      Help    = 0x00000001, ///< Show Help button.
      Default = 0x00000002, ///< Show Default button.
      Ok      = 0x00000004, ///< Show Ok button. (this button accept the dialog)
      Apply   = 0x00000008, ///< Show Apply button.
      Try     = 0x00000010, ///< Show Try button.
      Cancel  = 0x00000020, ///< Show Cancel-button. (this button reject the dialog)
      Close   = 0x00000040, ///< Show Close-button. (this button reject the dialog)
      User1   = 0x00000080, ///< Show User defined button 1.
      User2   = 0x00000100, ///< Show User defined button 2.
      User3   = 0x00000200, ///< Show User defined button 3.
      No      = 0x00000080, ///< Show No button. (this button close the dialog and set the result to No)
      Yes     = 0x00000100, ///< Show Yes button. (this button close the fialog and set the result to Yes)
      Details = 0x00000400, ///< Show Details button.
      Filler  = 0x40000000, ///< @internal Ignored when used in a constructor.
      Stretch = 0x80000000, ///< @internal Ignored when used in a constructor.
      NoDefault             ///< Used when specifying a default button; indicates that no button should be marked by default. @since 3.3
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

  public:

#ifdef KDE3_SUPPORT
    /**
     * Constructor.
     *
     * Takes the same arguments as QDialog.
     */
    KDialog(QWidget *parent = 0, const char *name = 0,
	    bool modal = false, Qt::WFlags f = 0) KDE_DEPRECATED;
#endif

    /**
     * Constructor for the predefined layout mode where you specify the
     * kind of layout (face).
     *
     * @param parent Parent of the dialog.
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param buttonMask Specifies which buttons will be visible. If zero
     *        (0) no button box will be made.
     * @param dialogFace You can use TreeList, Tabbed, Plain, Swallow or
     *        IconList.
     * @param flags  the Widget flags passed to the QDialog constructor
     * @param user1 User button1 text item.
     * @param user2 User button2 text item.
     * @param user3 User button3 text item.
     */
    KDialog( QWidget *parent=0, const QString &caption = QString() ,
		int buttonMask = 0, Qt::WFlags flags = 0 ,
		const KGuiItem &user1=KGuiItem(),
		 const KGuiItem &user2=KGuiItem(),
		 const KGuiItem &user3=KGuiItem() );

    /**
     * Destructor.
     */
    ~KDialog();

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
    void setButtonBoxOrientation( Qt::Orientation orientation );

    /**
     * Sets the button that will be activated when the Escape key
     * is pressed.
     *
     * By default, the Escape key is mapped to either the Cancel or the Close button
     * if one of these buttons are defined. The user expects that Escape will
     * cancel an operation so use this function with caution.
     *
     * @param id The button code.
     */
    void setEscapeButton( ButtonCode id );


    /**
     * Sets the button that will be activated when the Enter key
     * is pressed.
     *
     * By default, this is the Ok button if it is present
     *
     * @param id The button code.
     * @since KDE 4.0
     */
    void setDefaultButton( ButtonCode id );



    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

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
     * Convenience method. Sets the initial dialog size.
     *
     *  This method should
     * only be called right before show() or exec(). The initial
     * size will be
     * ignored if smaller than the dialog's minimum size.
     *
     * @param s Startup size.
     */
    void setInitialSize( const QSize &s );

    /**
     * Convenience method. Add a size to the default minimum size of a
     * dialog.
     *
     * This method should only be called right before show() or
     * exec().
     *
     * @param s  Size added to minimum size.
     */
    void incInitialSize( const QSize &s );

   /**
    * read the dialogs size from the configuration according to the screen size.
    * If no size is saved for one dimension of the screen, sizeHint() is returned.
    *
    * @param groupName Name of the group to read from. The old group
    *                  of KGlobal::config is preserved.
    */
   QSize configDialogSize( const QString& groupName ) const;

   /**
    * read the dialogs size from the configuration according to the screen size.
    * If no size is saved for one dimension of the screen, sizeHint() is returned.
    *
    * @param config The KConfig object to read from
    * @param groupName Name of the group to read from. The old group
    *                  of KGlobal::config is preserved.
    * @since 3.2
    */
   QSize configDialogSize( KConfig& config, const QString& groupName ) const;

   /**
    * save the dialogs size dependant on the screen dimension either to the
    * global or application config file.
    *
    * @param groupName The group to which the dialogs size is saved. See configDialogSize
    * to read the size.
    * @param global Set to true if the entry should go to the global config rather
    *        than to the applications config. Default is false.
    */
   void saveDialogSize( const QString& groupName, bool global=false );

   /**
    * save the dialogs size dependant on the screen dimension.
    *
    * @param config The KConfig object to write to.
    * @param groupName The group to which the dialogs size is saved. See
    * configDialogSize to read the size.
    * @param global Set to true if the entry should go to the global config.
    *        Default is false.
    * @since 3.2
    */
   void saveDialogSize( KConfig& config, const QString& groupName,
			     bool global=false ) const;

  
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
     * Sets the KGuiItem directly for the button instead of using 3 methods to
     * set the text, tooltip and whatsthis strings. This also allows to set an
     * icon for the button which is otherwise not possible for the extra
     * buttons beside Ok, Cancel and Apply.
     *
     * @param id The button identifier.
     * @param item The KGuiItem for the button.
     *
     * @since 3.3
     */
    void setButtonGuiItem( ButtonCode id, const KGuiItem &item );

 
    /**
     * Returns the action button that corresponds to the @p id.
     *
     * Normally
     * you should not use this function. @em Never delete the object returned
     * by this function. See also enableButton(), showButton(),
     * setButtonTip(), setButtonWhatsThis(), and setButtonText().
     *
     * @param id Integer identifier of the button.
     * @return The action button or 0 if the button does not exists.
     *
     */
    KPushButton *actionButton( ButtonCode id );

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
     * Return the number of pixels you shall use between a
     * dialog edge and the outermost widget(s) according to the KDE standard.
     **/
    static int marginHint();

    /**
     * Return the number of pixels you shall use between
     * widgets inside a dialog according to the KDE standard.
     */
    static int spacingHint();

    /**
     * Resize every layout manager used in @p widget and its nested children.
     *
     * @param widget The widget used.
     * @param margin The new layout margin.
     * @param spacing The new layout spacing.
     */
    static void resizeLayout( QWidget *widget, int margin, int spacing );

    /**
     * Resize every layout associated with @p lay and its children.
     *
     * @param lay layout to be resized
     * @param margin The new layout margin
     * @param spacing The new layout spacing
     */
    static void resizeLayout( QLayout *lay, int margin, int spacing );

    /**
     * Centers @p widget on the desktop, taking multi-head setups into
     * account. If @p screen is -1, @p widget will be centered on its
     * current screen (if it was shown already) or on the primary screen.
     * If @p screen is -3, @p widget will be centered on the screen that
     * currently contains the mouse pointer.
     * @p screen will be ignored if a merged display (like Xinerama) is not
     * in use, or merged display placement is not enabled in kdeglobals.
     * @since 3.1
     */
    static void centerOnScreen( QWidget *widget, int screen = -1 );

    /**
     * Places @p widget so that it doesn't cover a certain @p area of the screen.
     * This is typically used by the "find dialog" so that the match it finds can
     * be read.
     * For @p screen, see centerOnScreen
     * @return true on success (widget doesn't cover area anymore, or never did),
     * false on failure (not enough space found)
     * @since 3.2
     */
    static bool avoidArea( QWidget *widget, const QRect& area, int screen = -1 );

  public slots:
    /**
     * Make a KDE compliant caption.
     *
     * @param caption Your caption. Do @p not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption( const QString &caption );

    /**
     * Make a plain caption without any modifications.
     *
     * @param caption Your caption. This is the string that will be
     * displayed in the window title.
     */
    virtual void setPlainCaption( const QString &caption );


  protected:
    /**
     * @internal
     */
    virtual void keyPressEvent(QKeyEvent*);


   signals:
    /**
     * Emitted when the margin size and/or spacing size
     * have changed.
     *
     * Use marginHint() and spacingHint() in your slot
     * to get the new values.
     */
    void layoutHintChanged();


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
     *
     * @note The help button works differently for the class
     * KCMultiDialog, so it does not make sense to call this
     * function for Dialogs of that type.  See
     * KCMultiDialog::slotHelp() for more information.
     */
    void setHelp( const QString &anchor,
		  const QString &appname = QString() );

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
     * slotHelp() is not replaced.
     */
    void helpClicked();

    /**
     * The Default button was pressed. This signal is only emitted if
     * slotDefault() is not replaced.
     */
    void defaultClicked();


    /**
     * The User3 button was pressed. This signal is only emitted if
     * slotUser3() is not replaced.
     */
    void user3Clicked();

    /**
     * The User2 button was pressed. This signal is only emitted if
     * slotUser2() is not replaced.
     */
    void user2Clicked();

    /**
     * The User1 button was pressed. This signal is only emitted if
     * slotUser1() is not replaced.
     */
    void user1Clicked();

    /**
     * The Apply button was pressed. This signal is only emitted if
     * slotApply() is not replaced.
     */
    void applyClicked();

    /**
     * The Try button was pressed. This signal is only emitted if
     * slotTry() is not replaced.
     */
    void tryClicked();

    /**
     * The OK button was pressed. This signal is only emitted if
     * slotOk() is not replaced.
     */
    void okClicked();

    /**
     * The Yes button was pressed. This signal is only emitted if
     * slotYes() is not replaced.
     */
    void yesClicked();

    /**
     * The No button was pressed. This signal is only emitted if
     * slotNo() is not replaced.
     */
    void noClicked();

    /**
     * The Cancel button was pressed. This signal is only emitted if
     * slotCancel() is not replaced.
     */
    void cancelClicked();

    /**
     * The Close button was pressed. This signal is only emitted if
     * slotClose() is not replaced.
     */
    void closeClicked();

	/**
     * A button has been pressed.
	 * @param button is the code of the pressed button.
	 */
	void buttonClicked( KDialog::ButtonCode button);

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
     * This signal is also emitted when you call hide()
     *
     * If you have stored a pointer to the
     * dialog do @em not try to delete the pointer in the slot that is
     * connected to this signal.
     *
     * You should use deleteLater() instead.
     */
    void finished();

    /**
     * The detailsWidget is about to get shown. This is your last chance
     * to call setDetailsWidget if you haven't done so yet.
     */
    void aboutToShowDetails();

  protected:

    /**
     * Emits the #hidden signal. You can connect to that signal to
     * detect when a dialog has been closed.
     */
    virtual void hideEvent( QHideEvent * );

    /**
     * Detects when a dialog is being closed from the window manager
     * controls. If the Cancel or Close button is present then the button
     * is activated. Otherwise standard QDialog behavior
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
     * QDialog::accept() is activated.
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
     * QDialog::done( Yes ) is activated.
     */
    virtual void slotYes();

    /**
     * Activated when the Yes button has been clicked. The
     * QDialog::done( No ) is activated.
     */
    virtual void slotNo();

    /**
     * Activated when the Cancel button has been clicked. The
     * QDialog::reject() is activated in regular mode and
     * QDialog::done( Cancel ) when in message box mode.
     */
    virtual void slotCancel();

    /**
     * Activated when the Close button has been clicked. The
     * QDialog::reject() is activated.
     */
    virtual void slotClose();

    /**
     * Updates the margins and spacings.
     */
    void updateGeometry();


  private:
    static const int mMarginSize;
    static const int mSpacingSize;

    /**
     * Makes the button box and all the buttons in it. This method must
     * only be ran once from the constructor.
     *
     * @param buttonMask Specifies what buttons will be made.
     * @param user1 User button1 item.
     * @param user2 User button2 item.
     * @param user2 User button3 item.
     */
    void makeButtonBox( int mask ,
			const KGuiItem &user1 = KGuiItem(),
			const KGuiItem &user2 = KGuiItem(),
			const KGuiItem &user3 = KGuiItem() );


    /**
     * Sets the action button order according to the 'style'.
     *
     * @param style The style index.
     */
    void setButtonStyle( int style );

    /**
     * Prepares the layout that manages the widgets of the dialog
     */
    void setupLayout();


    /**
     * Sets the action button that is marked as default and has focus.
     *
     * @param p The action button.
     * @param isDefault If true, make the button default
     * @param isFocus If true, give the button focus.
     */
    void setButtonFocus( QPushButton *p, bool isDefault, bool isFocus );

  protected:
    virtual void virtual_hook( int id, void* data );
  private:
    class KDialogPrivate;
    KDialogPrivate* const d;



};


 /**
  * \brief Queue for showing modal dialogs one after the other.
  *
  * This is useful if you want to show a modal dialog but are not in the
  * position to start a new event loop at that point in your code.
  *
  * The disadvantage is that you will not be able to get any information from
  * the dialog, so it can currently only be used for simple dialogs.
  *
  * You probably want to use KMessageBox::queueMessageBox() instead
  * of this class directly.
  *
  * @author Waldo Bastian <bastian@kde.org>
  */
class KDialogQueuePrivate;
class KDEUI_EXPORT KDialogQueue : public QObject
{
      Q_OBJECT

public:

      static void queueDialog(QDialog *);

      ~KDialogQueue();

protected:
      KDialogQueue();
      static KDialogQueue *self();

private slots:
      void slotShowQueuedDialog();

protected:
      KDialogQueuePrivate* const d;
      static KDialogQueue *_self;
};

#endif // __KDIALOG_H

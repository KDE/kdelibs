/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Mirko Sucker (mirko@kde.org) and 
 *  Espen Sand (espensa@online.no)
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

#include <qlayout.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include <kdialog.h>
#include <kjanuswidget.h>

class KDialogBaseTile;
class KSeparator;
class KURLLabel;


/**
 * This base class provides basic functionality needed by nearly all dialogs.
 * If offers the standard action buttons you expect to find in a dialog, 
 * as well as the ability to define at most three configurable buttons. You 
 * can define a main widget which contains your specific dialog layout or use 
 * a predefined layout. Currently, TreeList/Paged, Tabbed, Plain 
 * and Swallow mode layouts (faces) are available.
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
 * Standard buttons (action buttons):
 * You select what buttons that can be displayed, but you do not choose the 
 * order in which they are displayed. This ensures a standard interface in 
 * KDE. The button order can be changed, but this ability is only available
 * for a central KDE control tool. The following buttons are available:
 * OK, Cancel/Close, Apply/Try, Default, Help and three user definable 
 * buttons: User1, User1 and User3. You must specify the text of the UserN 
 * buttons. Each button has a virtual slot so you can overload the method 
 * when required. The default slots emit a signal as well, so you can choose
 * to connect a signal instead of overriding the slot.
 * The default implementation of @ref slotHelp will automatically enable the 
 * help system if you have provided a path to the help text. @ref slotCancel 
 * and @ref slotClose will run @ref QDialog::reject while @ref slotOk will run 
 * @ref QDialog::accept. You define a default button in the constructor.
 *
 * Dialog shapes:
 * You can either use one of the prebuild, easy to use, faces or define 
 * your own main widget. The dialog provides ready to use TreeList,  
 * Tabbed, Plain and Swallow faces. For the first two you then add pages 
 * with @ref addPage. If you want complete control on how the dialog 
 * contents should look like, then you can define a main widget by using 
 * @ref setMainWidget. You only need to set the minimum size of that 
 * widget and the dialog will resize itself to fit this minimum size. 
 * The dialog is resizeable, but can not be made smaller than its minimum 
 * size.
 *
 * Layout:
 * The dialog consist of a help area on top (becomes visible if you define
 * a help path and use enableLinkedHelp()), the built-in dialog face or
 * your own widget in the middle, and the button row at the bottom. You can
 * also specify a separator to be shown above the button row.
 *
 * Standard compliance:
 * The class is derived form @ref KDialog, so you get automatic access to
 * the @ref KDialog::marginHint, @ref KDialog::spacingHint and the 
 * extended @ref KDialog::setCaption method.
 *
 * @short A dialog base class which standard buttons and predefined layouts.
 * @author Mirko Sucker (mirko@kde.org) and Espen Sand (espensa@online.no)
 */
class KDialogBase : public KDialog
{
  Q_OBJECT

  public:
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
      Stretch = 0x80000000
    };

    enum ActionButtonStyle
    {
      ActionStyle0=0, // Kde std
      ActionStyle1,
      ActionStyle2,
      ActionStyle3,
      ActionStyle4,
      ActionStyleMAX
    };

    enum DialogType
    {
      TreeList = KJanusWidget::TreeList,
      Tabbed   = KJanusWidget::Tabbed,
      Plain    = KJanusWidget::Plain,
      Swallow  = KJanusWidget::Swallow
    };

    enum ResizeMode
    {
      ResizeFixed=0,
      ResizeMinimum,
      ResizeFree
    };

  private:
    struct SButton
    {
      int mask;
      int style;
      QList<QPushButton> list;
      

      void append( QPushButton *p )
      {
	list.append( p );
      }
      
      void resize( bool sameWidth, int margin, int spacing )
      {
	QPushButton *p;
	int w = 0;
	int t = 0;

	for( p = list.first(); p!=0; p =  list.next() )
	{
	  if( p->sizeHint().width() > w ) { w = p->sizeHint().width(); }
	}
	for( p = list.first(); p!=0; p =  list.next() )
	{
	  QSize s( p->sizeHint() );
	  if( sameWidth == true ) { s.setWidth( w ); }
	  p->setFixedSize( s );
	  t += s.width() + spacing;
	}
	
	p = list.first();
	box->setMinimumHeight( margin*2 + ( p==0? 0:p->sizeHint().height()) );
	box->setMinimumWidth( margin*2 + t - spacing );
      }

      QWidget *box;
      QPushButton *help;
      QPushButton *def;
      QPushButton *ok;
      QPushButton *apply;
      QPushButton *_try;
      QPushButton *cancel;
      QPushButton *close;
      QPushButton *user1;
      QPushButton *user2;
      QPushButton *user3;
    };

  public:

    /** 
     * Constructor for the standard mode where you must specify the main widget
     * with @ref setMainWidget .
     * 
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only)
     * @param modal Controls dialog modality. If false, the rest of the 
     * program interface (example: other dialogs) is accessible while the 
     * dialog is open.
     * @param caption The dialog caption. Do not specify the application name
     * here. The class will take care of that.
     * @param buttonMask Specifies what buttons will be visible.
     * @param defaultButton Specifies what button we be marked as the default.
     * @param separator If true, a separator line is drawn between the action
     * buttons an the main widget.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user3 User button3 text.
     */
    KDialogBase( QWidget *parent=0, const char *name=0, bool modal=true,
		 const QString &caption=QString::null,
		 int buttonMask=Ok|Apply|Cancel, int defaultButton=Ok,
		 bool separator=false, 
		 const QString &user1=QString::null,
		 const QString &user2=QString::null,
		 const QString &user3=QString::null);

    /** 
     * Constructor for the predefined layout mode where specify the kind of
     * layout (face).
     * 
     * @param dialogFace You can use TreeList, Tabbed, Plain or Swallow.
     * @param caption The dialog caption. Do not specify the application name
     * here. The class will take care of that.
     * @param buttonMask Specifies what buttons will be visible.
     * @param defaultButton Specifies what button we be marked as the default.
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only)
     * @param modal Controls dialog modality. If false, the rest of the 
     * program interface (example: other dialogs) is accessible while the 
     * dialog is open.
     * @param separator If true, a separator line is drawn between the action
     * buttons an the main widget.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user3 User button3 text.
     */
    KDialogBase( int dialogFace, const QString &caption, 
		 int buttonMask, int defaultButton, 
		 QWidget *parent=0, const char *name=0, bool modal=true, 
		 bool separator=false,
		 const QString &user1=QString::null,
		 const QString &user2=QString::null,
		 const QString &user3=QString::null);
    /**
     * Destructor
     */
    ~KDialogBase( void );

    /**
     * Use this method to make a modless (modal = false in constructor)
     * visible. If you reimplmement this method make sure you run it 
     * in the new menthod (i.e., KDialogBase::show()). Reimplemented 
     * from &ref QWidget.
     */
    virtual void show( void );

    /**
     * Use this method to make a modless (modal = false in constructor)
     * visible. If you reimplmement this method make sure you run it 
     * in the new menthod (i.e., KDialogBase::show( widget )).
     *
     * @param centerParent center the dialog with respect to this widget.
     */
    virtual void show( QWidget *centerParent );

    /**
     * Hides or displays the a separator line drawn between the action
     * buttons an the main widget.
     */
    void enableButtonSeparator( bool state );

    /**
     * Enables or disables (grays out) a general action button.
     *
     * @param flag A mask of @ref ButtonCode flags
     * @param state true enables the button(s). 
     */
    void enableButton( int flag, bool state );
    
    /**
     * Enables or disables (grays out) the OK button.
     *
     * @param state true enables the button. 
     */
    void enableButtonOK( bool state );

    /**
     * Enables or disables (grays out) the Apply button.
     *
     * @param state true enables the button. 
     */
    void enableButtonApply( bool state );
    
    /**
     * Enables or disables (grays out) the Cancel button.
     *
     * @param state true enables the button. 
     */
    void enableButtonCancel( bool state );
    
    /**
     * Hides or displays a general action button. Only buttons that have
     * been created in the constructor can be displayed. This method will
     * not create a new button.
     *
     * @param flag A mask of @ref ButtonCode flags.
     * @param state true display the button(s). 
     */
    void showButton( int flag, bool state );
    
    /**
     * Hides or displays the OK button. The OK button must have
     * been created in the constructor to be displayed.
     *
     * @param state true display the button(s). 
     */
    void showButtonOK( bool state );
    
    /**
     * Hides or displays the Apply button. The Apply button must have
     * been created in the constructor to be displayed.
     *
     * @param state true display the button(s). 
     */
    void showButtonApply( bool state );

    /**
     * Hides or displays the Cancel button. The Cancel button must have
     * been created in the constructor to be displayed.
     *
     * @param state true display the button(s). 
     */
    void showButtonCancel( bool state );

    /**
     * Sets the page with 'index' to be displayed. This method will only 
     * work when the dialog is using the prefefined shape of TreeList or
     * Tabbed.
     *
     * @param index Index of the page to be shown.
     * @return true if the page is shown, false otherwise.
     */
    bool showPage( int index );

    /**
     * Returns the index of the active page. This method will only 
     * work when the dialog is using the prefefined shape of TreeList or
     * Tabbed.
     *
     * @return The page index or -1 if there is no active page.
     */
    int  activePageIndex( void ) const;

    /**
     * Set the main user definable widget. If the dialog is using the 
     * predefined Swallow mode, the widget will be reparented to the internal
     * swallow control widget. If the dialog is being used in the standard
     * mode then the 'widget' must have the dialog as parent.
     *
     * @param *widget The widget to be displayed as main widget. If it
     * is 0, then the dialog will show an empty space of 100x100 pixels
     * instead.
     */
    void setMainWidget( QWidget *widget );

    /**
     * Returns the main widget if any.
     *
     * @return The current main widget. Can be 0 if no widget has been defined.
     */
    QWidget *getMainWidget( void ); 

    /**
     * Sets the resize mode.
     *
     * @param mode The resize mode. It can be ResizeFixed (no resize
     * possible), ResizeMinimum (resize is allowed, but limited by the 
     * minimum size) or ResizeFree (the main widget can be resized to zero
     * size )
     */
    void setResizeMode( int mode );

    /**
     * Sets a size that is added to the dialog minimum size the first
     * time it is displayed. The dialog can still be resized to its minimum
     * size. Note: The size value will be ignored if the resize mode is
     * ResizeFixed.
     * 
     * @param initialSizeStep - Value added to minimum size.
     *
     */
    void setInitialSizeStep( const QSize &initialSizeStep );

    /**
     * Updates the size of the dialog and the resize constraints depending
     * on the current resize mode. THis method is executed from 
     * @ref setResizeMode so you should not need to use this method.
     */
    void updateSize( void );

    /** 
     * Sets the text of the OK button. If the default parameters are used 
     * (that is, if no parameters are given) the standard texts are set:
     * The button shows "OK", the tooltip contains "Accept settings." 
     * (internationalized) and the quickhelp text explains the standard 
     * behaviour of the OK button in dialogs. 
     *
     * @param text Button text
     * @param tooltip Tooltip text
     * @param quickhelp Quick help text
     */
    void setButtonOKText( const QString &text=QString::null, 
			  const QString &tooltip=QString::null, 
			  const QString &quickhelp=QString::null );

    /** 
     * Sets the text of the Apply button. If the default parameters are 
     * used (that is, if no parameters are given) the standard texts are set:
     * The button shows "Apply", the tooltip contains "Apply settings." 
     * (internationalized) and the quickhelp text explains the standard 
     * behaviour of the apply button in dialogs. 
     *
     * @param text Button text
     * @param tooltip Tooltip text
     * @param quickhelp Quick help text
     */
    void setButtonApplyText( const QString &text=QString::null, 
			     const QString &tooltip=QString::null, 
			     const QString &quickhelp=QString::null );

    /** 
     * Sets the text of the Cancel button. If the default parameters are 
     * used (that is, if no parameters are given) the standard texts are set:
     * The button shows "Cancel", the tooltip contains "Cancel settings." 
     * (internationalized) and the quickhelp text explains the standard 
     * behaviour of the cancel button in dialogs. 
     *
     * @param text Button text
     * @param tooltip Tooltip text
     * @param quickhelp Quick help text
     */
    void setButtonCancelText( const QString &text=QString::null, 
			      const QString &tooltip=QString::null, 
			      const QString &quickhelp=QString::null );

    /** 
     * Returns a flag telling whether the background tile is set or not. 
     *
     * @return true if there is defined a background tile.
     */
    static bool  haveBackgroundTile( void );
    
    /** 
     * Returns a const pointer to the background tile if there is one. 
     *
     * @return The tile pointer or 0 if no tile is defined.
     *
     **/
    static const QPixmap* getBackgroundTile( void );
    
    /** 
     * Sets the background tile. If it is Null (0), the background
     * image is deleted. 
     *
     * @param pix The background tile
     */
    static void  setBackgroundTile( const QPixmap *pix );

    /**
     * Enables of hides the background tile (if any)
     *
     * @param state true will make the tile visible. 
     */
    void showTile( bool state );

    /** 
     * Do not use this method. It is included for compatibility reasons.     
     *
     * This method returns the border widths in all directions the dialog 
     * needs for itself. Respect this, or get bad looking results. 
     * The references are upper left x, upper left y, lower left x 
     * and lower left y. The results are differences in pixels from the 
     * dialogs corners. 
     */
    void  getBorderWidths( int& ulx, int& uly, int& lrx, int& lry ) const;

    /**
     * Do not use this method. It is included for compatibility reasons. 
     *
     * This method returns the contents rectangle of the work area. Place 
     * your widgets inside this rectangle, and use it to set up 
     * their geometry. Be carefull: The rectangle is only valid after 
     * resizing the dialog, as it is a result of the resizing process. 
     * If you need the "overhead" the dialog needs for its elements, 
     * use getBorderWidth. 
     */
    QRect getContentsRect( void );

    /** 
     * With this method it is easy to calculate a size hint for a 
     * dialog derived from KDialogBase if you know the width and height of 
     * the elements you add to the widget. The rectangle returned is 
     * calculated so that all elements exactly fit into it. Thus, you may 
     * set it as a minimum size for the resulting dialog. 
     *
     * You should not need to use this method and never if you use one of
     * the prefined shapes.
     *
     * @param w The width of you special widget.
     * @param h The height of you special widget.
     * @return The minimum width and height of the dialog using 'w' and 'h'
     * as the size of the main widget.
     */
    QSize calculateSize( int w, int h );
    
    /**
     * Emits the signal ::backgroundChanged. You never need to thouch this 
     * method yourself. It is executred automatically.
     */
    void  emitBackgroundChanged( void );
  
  public slots:
    /**
     * Displays or hides the help link area on the top of the dialog.
     *
     * @param state true will displa the area.
     */
    void enableLinkedHelp( bool state );

    /** 
     * Sets the help topic, and the text that is shown as the linked text.
     * If text is empty, the text is "Get help..."  (internationalized) 
     * is used insread
     *
     * @param path Path to help text
     * @param topic Topic in help text.
     * @param text Link text.
     */
    void setHelp( const QString &path, const QString& topic,
		  const QString& text );
    /** 
     * Connected to help link label. 
     */
    void helpClickedSlot( const QString & );

    /** 
     * Initializes the minimum size of the dialog. You do not need to
     * use this method. It is used automatically in @ref show.
     */
    void initializeGeometry( void );

    /** 
     * This methos is called automatically whenever the background has 
     * changed. You do not need to use this method.
     */
    void updateBackground( void );

  signals:
    /** 
     * The Help button was pressed. This signal is only emitted if
     * @ref slotHelp is not replaced.
     */
    void helpClicked( void );

    /** 
     * The Default button was pressed. This signal is only emitted if
     * @ref slotDefault is not replaced.
     */
    void defaultClicked( void );
    
    /** 
     * The User3 button was pressed. This signal is only emitted if
     * @ref slotUser3 is not replaced.
     */
    void user3Clicked( void );

    /** 
     * The User2 button was pressed. This signal is only emitted if
     * @ref slotUser2 is not replaced.
     */
    void user2Clicked( void );

    /** 
     * The User1 button was pressed. This signal is only emitted if
     * @ref slotUser1 is not replaced.
     */
    void user1Clicked( void );

    /** 
     * The Apply button was pressed. This signal is only emitted if
     * @ref slotApply is not replaced.
     */
    void applyClicked( void );

    /** 
     * The Try button was pressed. This signal is only emitted if
     * @ref slotTry is not replaced.
     */
    void tryClicked( void );

    /** 
     * The OK button was pressed. This signal is only emitted if
     * @ref slotOk is not replaced.
     */
    void okClicked( void );

    /** 
     * The Cancel button was pressed. This signal is only emitted if
     * @ref slotCancel is not replaced.
     */
    void cancelClicked();
    
    /** 
     * The Close button was pressed. This signal is only emitted if
     * @ref slotClose is not replaced.
     */
    void closeClicked();

    /** 
     * Do not use this signal. Is is kept for compatibility reasons.
     * Use @ref applyClicked instead.
     */
    void apply( void );

    /** 
     * The background tile has changed.
     */
    void backgroundChanged( void ); 


  protected:
    /**
     * Returns the empty page when the predefined layout is used in Plain
     * mode. This widget must used as the toplevel widget of your dialog
     * code.
     *
     * @return The widget or 0 if the predefined layout mode is not Plain
     * or if you don't use any predefined layout
     */
    QFrame *plainPage( void );

    /**
     * Add a page to the predefined layout when used in TreeList or Tabbed
     * mode. The returned widget must used as the toplevel widget for 
     * this particular page.
     *
     * @param item Name used in the list (TreeList mode) or Tab name 
     * (Tabbed mode).
     * @param header Header text use in TreeList mode. Ignored in Tabbed 
     * mode. If empty, the item text is used instead.
     *
     * @return The page widget which must be used as the toplevel widget for
     * the page.
     */
    QFrame  *addPage( const QString &item, 
                      const QString &header=QString::null );

    /**
     * The resize event handler. Takes care of position the components of
     * the dialog. If you subclass KDialogBase and need to define you own
     * resizeEvent, then make sure you the original resizeEvent it executed as
     * well: From your resizeEvent do KDialogBase::resizeEvent().
     */
    virtual void resizeEvent( QResizeEvent * );
    
    /**
     * Maps some keys to the actions buttons. F1 is mapped to the Help
     * button if present and Escape to the Cancel or Close if present. The
     * button action event is animated.
     */
    virtual void keyPressEvent( QKeyEvent * );

  protected slots:
    /**
     * Activated when the Help button has been clicked. If a help
     * text has been defined, the help system will be activated.
     */
    virtual void slotHelp( void );
  
    /**
     * Activated when the Default button has been clicked.
     */
    virtual void slotDefault( void );

    /**
     * Activated when the User3 button has been clicked.
     */
    virtual void slotUser3( void );

    /**
     * Activated when the User2 button has been clicked.
     */
    virtual void slotUser2( void );

    /**
     * Activated when the User1 button has been clicked.
     */
    virtual void slotUser1( void );

    /**
     * Activated when the Ok button has been clicked. The 
     * @ref QDialog::accept() is activated.
     */
    virtual void slotOk( void );

    /**
     * Activated when the Apply button has been clicked.
     */
    virtual void slotApply( void );

    /**
     * Activated when the Try button has been clicked.
     */
    virtual void slotTry( void );

    /**
     * Activated when the Cancel button has been clicked. The 
     * @ref QDialog::reject() is activated.
     */
    virtual void slotCancel( void );

    /**
     * Activated when the Close button has been clicked. The 
     * @ref QDialog::reject() is activated.
     */
    virtual void slotClose( void );

    /**
     * Do not use this slot. Is is kept for compatibility reasons.
     * Activated when the Apply button has been clicked
     */
    virtual void applyPressed( void );

    /**
     * Initializes geometry and resizes to the minimum size. 
     */
    void updateGeometry( void );

  private:
    /**
     * Prepares a relay that is used to send signals between
     * all KDialogBase instances of a program. Should only be used in the 
     * constructor.
     */
    void makeRelay( void );

    /**
     * Makes the help link area. Should only be used in the 
     * constructor.
     */
    void makeUrlBox( void );
    
    /**
     * Updates the geometry of the help link area box.
     */
    void setUrlBoxGeometry( void );

    /**
     * Makes the button box and all the buttons in it. This method must 
     * only be ran once from the constructor.
     *
     * @param buttonMask Specifies what buttons will be made.
     * @param defaultButton Specifies what button we be marked as the 
     * default.
     * @param user1 User button1 text.
     * @param user2 User button2 text.
     * @param user2 User button3 text. 
     */

    void makeButtonBox( int mask, int defaultButton, 
			const QString &user1 = QString::null,
			const QString &user2 = QString::null,
			const QString &user3 = QString::null );
    /**
     * Sets the action button that is marked as default and has focus.
     *
     * @param p The action button
     * @param isDefault If true, make the button default 
     * @param isFocus If true, give the button focus.
     */
    void setButtonFocus( QPushButton *p, bool isDefault, bool isFocus );

    /**
     * This method is ran once in the for a KDialogBase object. It is done
     * automatically in @ref show Never use this method yourself.
     */
    void activateCore( void );

  private slots:
    /**
     * Sets the action button order accoring to the 'style'.
     *
     * @param style The style index.
     */
    void setButtonStyle( int style );


    void cleanup( void );
    




  private:
    QWidget      *mMainWidget;
    KURLLabel    *mUrlHelp;
    KJanusWidget *mJanus;
    KSeparator   *mActionSep;

    SButton mButton;
    bool mIsActivated;

    QString mHelpPath;
    QString mHelpTopic;

    static KDialogBaseTile *mTile;
    bool   mShowTile;

    int   mResizeMode;
    QSize mInitialSizeStep;
};




/**
* Used internally by @ref KDialog.
* @internal
*/
class KDialogBaseTile : public QObject
{
  Q_OBJECT

  public:
    KDialogBaseTile( QObject *parent=0, const char *name=0 );
    ~KDialogBaseTile( void );

    void set( const QPixmap *pix );
    const QPixmap *get( void ) const;
  
  public slots:
    void cleanup( void );

  signals:
    void pixmapChanged( void );

  private:
    QPixmap *mPixmap;
};













#endif




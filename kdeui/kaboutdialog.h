/* -*- C++ -*-
   This file is part of the KDE libraries
   Copyright (C) 1999 Mirko Sucker <mirko@kde.org>

   This file declares a class for creating "About ..." dialogs
   in a general way. It provides geometry management and some
   options to connect for, like emailing the author or
   maintainer.
   
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

#ifndef KABOUTDIALOG_H
#define KABOUTDIALOG_H

#include <kdialogbase.h>
#include <qframe.h>
#include <qlabel.h>
#include <qstring.h>
#include <list.h>

class QVBoxLayout;
class KURLLabel;
class KTabCtl;


class KImageTrackLabel : public QLabel
{
  Q_OBJECT

  public:
    enum MouseMode
    {
      MousePress = 1,
      MouseRelease,
      MouseDoubleClick,
      MouseMove
    };

  public:
    KImageTrackLabel( QWidget * parent, const char * name=0, WFlags f=0 );

  signals:
    void mouseTrack( int mode, const QMouseEvent *e );
  
  protected:
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e ); 
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void mouseMoveEvent ( QMouseEvent *e );
};



class KAboutContainer : public QFrame
{
  Q_OBJECT

  public:
    KAboutContainer( QWidget *parent=0, const char *name=0, 
		     int margin=0, int spacing=0,
		     int childAlignment = AlignCenter,
		     int innerAlignment = AlignCenter );

    void addWidget( QWidget *widget );
    void addPerson( const QString &name, const QString &email,
		    const QString &url, const QString &task,
		    bool showHeader = false, bool showframe = false, 
		    bool showBold = false );
    void addTitle(  const QString &title, int alignment=AlignLeft, 
		    bool showframe = false, bool showBold = false );
    void addImage( const char *fileName, int alignment=AlignLeft );

    virtual QSize sizeHint( void ) const;
    virtual QSize minimumSizeHint( void ) const;

  protected:
    virtual void childEvent( QChildEvent *e );

  signals:
    void urlClick( const QString &url );
    void mailClick( const QString &name, const QString &address );


  private:
    QVBoxLayout *mVbox;
    int mAlignment;
};



class KAboutContainerBase : public QWidget
{
  Q_OBJECT

  public:
    enum LayoutType
    {
      AbtPlain         = 0x0001,
      AbtTabbed        = 0x0002,
      AbtTitle         = 0x0004,
      AbtImageLeft     = 0x0008,
      AbtImageRight    = 0x0010,
      AbtImageOnly     = 0x0020,
      AbtProduct       = 0x0040,
      AbtKDEStandard   = AbtTabbed|AbtTitle|AbtImageLeft,
      AbtAppStandard   = AbtTabbed|AbtTitle|AbtProduct,
      AbtImageAndTitle = AbtPlain|AbtTitle|AbtImageOnly 
    };

  public:
    KAboutContainerBase( int layoutType, QWidget *parent = 0, char *name = 0 );
    virtual void show( void );
    virtual QSize sizeHint( void );

    void setTitle( const QString &title );
    void setImage( const char *fileName );
    void setImageBackgroundColor( const QColor &color );
    void setImageFrame( bool state );
    void setProduct( const QString &appName, const QString &version,
		     const QString &author, const QString &year );

    QFrame *addTextPage( const QString &title, const QString &text, 
			 bool richText=false, int numLines=10 );
    KAboutContainer *addContainerPage( const QString &title,
      int childAlignment = AlignCenter, int innerAlignment = AlignCenter );
    QFrame *addEmptyPage( const QString &title );

    KAboutContainer *addContainer( int childAlignment, int innerAlignment ); 

  public slots:
    virtual void slotMouseTrack( int mode, const QMouseEvent *e );
    virtual void slotUrlClick( const QString &url );
    virtual void slotMailClick( const QString &name, const QString &address );

  protected:
    virtual void fontChange( const QFont &oldFont );

  signals:
    void mouseTrack( int mode, const QMouseEvent *e );
    void urlClick( const QString &url );
    void mailClick( const QString &name, const QString &address );

  private:
    QArray<QWidget*>  mContainerList;
   
    QVBoxLayout *mTopLayout; 
    KImageTrackLabel *mImageLabel;
    QLabel  *mTitleLabel;
    QLabel  *mIconLabel;
    QLabel  *mVersionLabel;
    QLabel  *mAuthorLabel;
    QFrame  *mImageFrame;
    KTabCtl *mPageTab;
    QFrame  *mPlainSpace;
};




class KAboutContributor : public QFrame
{
  Q_OBJECT

  public:
    KAboutContributor( QWidget *parent=0, const char *name=0,
		       const QString &name=QString::null,
		       const QString &email=QString::null,
		       const QString &url=QString::null, 
		       const QString &work=QString::null,
		       bool showHeader=false, bool showFrame=true, 
		       bool showBold=false );

    void setName( const QString &text, const QString &header=QString::null,
		  bool update = true );
    void setEmail( const QString &text, const QString &header=QString::null,
		   bool update = true );
    void setURL( const QString &text, const QString &header=QString::null,
		 bool update = true );
    void setWork( const QString &text, const QString &header=QString::null,
		  bool update = true );
    QString getName( void );
    QString getEmail( void );
    QString getURL( void );
    QString getWork( void );

    virtual QSize sizeHint( void );

  protected:
    virtual void fontChange( const QFont &oldFont );

  protected slots:
    void urlClickedSlot( const QString& );
    void emailClickedSlot( const QString& emailaddress );

  private:
    void updateLayout( void );

  signals:
    void sendEmail(const QString& name, const QString& email);
    void openURL(const QString& url);

  private:
    QLabel *mLabel[4];
    QLabel *mText[4];
    bool mShowHeader;
    bool mShowBold;
};





/** 
 * KAboutWidget is the main widget for KAboutDialog.
 * It has a minimum size set. 
 */
class KAboutWidget : public QWidget
{
  // #########################################################################
  Q_OBJECT

  public:

  /** 
   * The Qt constructor. 
   */
  KAboutWidget(QWidget* parent=0, const char* name=0 );

  /** 
   * Adjust the minimum size (after setting the properties of the image and 
   * the labels. 
   */
  void adjust();

  /** 
   * Set the image as the application logo. 
   */
  void setLogo(const QPixmap&);
  /** Set the authors name and email address. */
  void setAuthor(const QString& name, const QString& email,
		 const QString& url, const QString& work);
  /** Set the maintainers name and email address. */
  void setMaintainer(const QString& name, const QString& email,
		     const QString& url, const QString& work);
  /** Show this person as one of the major contributors. */
  void addContributor(const QString& name, const QString& email,
		      const QString& url, const QString& work);
  /** Set the text describing the version. */
  void setVersion(const QString& name);
  // -------------------------------------------------------------------------
protected slots:
  /** Catch the signals from the contributors elements. */
  void sendEmailSlot(const QString& name, const QString& email);
  /** Catch the clicked URLs. */
  void openURLSlot(const QString& url);
  // -------------------------------------------------------------------------
signals:
  /** An email address has been selected by the user. */
  void sendEmail(const QString& name, const QString& email);
  /** An URL has been clicked. */
  void openURL(const QString& url);
  // -------------------------------------------------------------------------
protected:
  // events:
  /** The resize event. */
  void resizeEvent(QResizeEvent*);
  /** The label showing the program version. */
  QLabel *version;
  /** The label showing the text "Other contributors:". */
  QLabel *cont;
  /** The frame showing the logo. */
  QLabel *logo;
  /** The application developer. */
  KAboutContributor *author;
  /** The application maintainer. */
  KAboutContributor *maintainer;
  /** Show the maintainer? */
  bool showMaintainer;
  /** A set of people who contributed to the application. */
  list<KAboutContributor*> contributors;
  // #########################################################################
};

/** 
 * KAboutDialog is a @ref KDialogBase with predefined main widget.
 * Being a KDialogBase it uses your application wide settings for KDialogBase
 * objects (base frame tiles, main frame tiles etc). 
 * To use it, simply create a KAboutDialog object, set all (or some) of its
 * properties and show it. Do not derive it to create your own about dialog 
 * until yoiu need some cool features that are unsupported and you have 
 * contacted me to add them. 
 * 
 * The dialog can be created using two different constructors. The 
 * difference between these constructors is the available components that 
 * can be used to build the contents of the dialog. The first (Constructor I)
 * provides a number of easy to use methods. Basically it allows you
 * to add the components of choice, and the components are placed in a
 * predefined layout. You do not have to plan the layout. Everything is 
 * taken care of.
 *
 * The second constructor (Constructor II) works in quite the same manner
 * as the first, but you have better control on where the components 
 * are postioned in the layout and you have access to an extended number
 * of components you can install such as titles, product information, 
 * a tabbed pages (where you can display rich text with url links) and 
 * a person (developer) information field. The "About KDE" dialog box is 
 * created with Constructor II.
 *
 * For the derived features, see the basic class @ref KDialogBase. 
 * @author Mirko Sucker (mirko@kde.org) and Espen Sand (espensa@online.no)
 * @see KDialogBase 
 */
class KAboutDialog : public KDialogBase
{
  Q_OBJECT

  public:
    enum LayoutType
    {
      AbtPlain         = 0x0001,
      AbtTabbed        = 0x0002,
      AbtTitle         = 0x0004,
      AbtImageLeft     = 0x0008,
      AbtImageRight    = 0x0010,
      AbtImageOnly     = 0x0020,
      AbtProduct       = 0x0040,
      AbtKDEStandard   = AbtTabbed|AbtTitle|AbtImageLeft,
      AbtAppStandard   = AbtTabbed|AbtTitle|AbtProduct,
      AbtImageAndTitle = AbtPlain|AbtTitle|AbtImageOnly 
    };

  public:
    /** 
     * The standard Qt constructor (Constructor I).
     */
    KAboutDialog( QWidget *parent=0, const char *name=0, bool modal=true );

   /**
    * The extended constructor. (Constructor II).
    * 
    * @param dialogLayout Use a mask of @ref LayoutType flags.
    * @param caption The dialog caption. The text you specify is prepended
    *        by i18n("About").
    * @param buttonMask Specifies what buttons will be visible.
    * @param defaultButton Specifies what button we be marked as the default.
    * @param parent Parent of the dialog.
    * @param name Dialog name (for internal use only)
    * @param modal Sets dialog modality. If false, the rest of the 
    *        program interface (example: other dialogs) is accessible while 
    *        the dialog is open.
    * @param separator If true, a separator line is drawn between the action
    *        buttons an the main widget.
    * @param user1 User button1 text.
    * @param user2 User button2 text.
    * @param user3 User button3 text.
    *
    */
    KAboutDialog( int dialogLayout, const QString &caption, int buttonMask, 
		  int defaultButton, QWidget *parent=0, const char *name=0, 
		  bool modal=false, bool separator = false,
		  const QString &user1 = QString::null,
		  const QString &user2 = QString::null, 
		  const QString &user3 = QString::null );

  /** 
   * Adjust the dialog. You can call this method after you have set up all 
   * the contents but it is not required. It is done automaticaly when 
   * @ref show is executed.
   */
  void adjust();

  /**
   * Use this method to make a modless (modal = false in constructor)
   * visible. If you reimplmement this method make sure you run it 
   * in the new method (i.e., KAboutDialog::show()). Reimplemented 
   * from &ref KDialogBase.
   */
  virtual void show( void );

  /**
   * Use this method to make a modless (modal = false in constructor)
   * visible. If you reimplmement this method make sure you run it 
   * in the new method (i.e., KAboutDialog::show( parent )).
   *
   * @param centerParent Center the dialog with respect to this widget.
   */
  virtual void show( QWidget *centerParent );

  /**
   * (Constructor II only)
   * Sets a title (not caption) in the uppermost area of the dialog
   * 
   * @param title Title string.
   */
  void setTitle( const QString &title );

  /**
   * (Constructor II only)
   * Defines an image to be shown in the dialog. The position is dependent
   * on the dialogLayout in the constructor
   * 
   * @param fileName Path to image file.
   */
  void setImage( const char *fileName );

  /**
   * (Constructor II only)
   * The image has a minimum size, but is centered within a an area if the 
   * dialog box is enlarged by the user. You set the background color 
   * of the area with this method.
   *
   * @param color Background color.
   */
  void setImageBackgroundColor( const QColor &color );

  /**
   * (Constructor II only)
   * Enables or disables a frame around the image. The frame is by default
   * enables in the constructor
   *
   * @param state True enables the frame
   */
  void setImageFrame( bool state );
  
  /**
   * (Constructor II only)
   * Prints the application name, KDE version, author, a copyright sign
   * and a year string. To the left of the text the standard application
   * icon is displayed.
   *
   * @param appName The application name
   * @param version Application version
   * @param author One or more authors
   * @param year A string telling when the application was made.
   */
  void setProduct( const QString &appName, const QString &version,
		   const QString &author, const QString &year );

  /**
   * (Constructor II only)
   * Adds a text page to a tab box. The text can be regular text or 
   * rich text. The rich text can contain urls and mail links
   *
   * @param title Tab name
   * @param text The text to display
   * @param richText Set this to True if 'text' is rich text.
   * @param numLines The text area height will be ajusted so that this
   *        is the minimum lines of text that are visible.
   */
  QFrame *addTextPage( const QString &title, const QString &text, 
                       bool richText=false, int numLines=10 );

  /**
   * (Constructor II only)
   * Adds a container to a tab box. You can add text and images to a 
   * container.
   * 
   * @param title Tab name
   * @param childAlignment Specifies how the clildren of the container are
   *        aligned with respect to the container.
   * @param innerAlignment Specifies how the clildren are aligned with 
   *        respect to each other.
   */
  KAboutContainer *addContainerPage( const QString &title,
    int childAlignment = AlignCenter, int innerAlignment = AlignCenter );
  
  /**
   * (Constructor II only)
   * Adds a container. You can add text and images to a container.
   * 
   * @param childAlignment Specifies how the clildren of the container are
   *        aligned with respect to the container.
   * @param innerAlignment Specifies how the clildren are aligned with 
   *        respect to each other.
   */
  KAboutContainer *addContainer( int childAlignment, int innerAlignment ); 

  /**
   * (Constructor II only)
   * Adds an empty page to a tab box.
   * 
   * @param title Tab name
   */
  QFrame *addPage( const QString &title );


  /** 
   * (Constructor I only)
   * Set the image as the application logo. 
   */
  void setLogo(const QPixmap&);

  /**
   * (Constructor I only)
   * Set the authors name and email address. 
   */
  void setAuthor(const QString& name, const QString& email,
		 const QString& url, const QString& work);

  /**
   * (Constructor I only)
   * Set the maintainers name and email address. 
   */
  void setMaintainer(const QString& name, const QString& email,
		     const QString& url, const QString& work);

  /**
   * (Constructor I only)
   * Show this person as one of the major contributors. 
   */
  void addContributor(const QString& name, const QString& email,
		      const QString& url, const QString& work);

  /**
   * (Constructor I only)
   * Set the text describing the version. 
   */
  void setVersion(const QString& name);

  /**
   * Creates a modal dialog with an image in the upper area with an 
   * url link below.
   */
  static void imageURL( QWidget *parent, const QString &caption,
			  const QString &path, const QColor &imageColor,
			  const QString &url );

signals:
  /** 
   * Send an email to this person. The application must provide the
   *  functionality. 
   */
  void sendEmail(const QString& name, const QString& email);

  /** 
   * Open the selected URL. 
   */
  void openURL(const QString& url);
  // ----------------------------------------------------------------------------
protected:
  /** 
   * The main widget (Constructor I) 
   */
  KAboutWidget * about;
  
  /** 
   * The main widget (Constructor II) 
   */
  KAboutContainerBase *mContainerBase;

  // ----------------------------------------------------------------------------
protected slots:

  /** 
   * Connected to widget->sendEmail. 
   */
  void sendEmailSlot(const QString& name, const QString& email);

  /** 
   * Open this URL. 
   */
  void openURLSlot(const QString& url);

  /**
   * (Constructor II only) 
   * Tells the position of the mouse cursor when the left mouse button
   * is pressed above an image 
   */
  virtual void mouseTrackSlot( int mode, const QMouseEvent *e );

  // ############################################################################
};

#endif // defined KABOUTDIALOG_H






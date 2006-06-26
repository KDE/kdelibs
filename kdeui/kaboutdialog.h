/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 * Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file declares a class for creating "About ..." dialogs
 * in a general way. It provides geometry management and some
 * options to connect for, like emailing the author or maintainer.
 */


#ifndef KABOUTDIALOG_H
#define KABOUTDIALOG_H

#include <QFrame>

#include <kdialog.h>

class QLabel;
class QVBoxLayout;
class QTabWidget;
class KUrlLabel;
class KAboutContainer;
class KAboutContainerBase;

/**
 *  KAboutContainer can be used to make a application specific AboutDialog.
 */
class KDEUI_EXPORT KAboutContainer : public QFrame
{
  Q_OBJECT

  public:
    KAboutContainer( QWidget *parent = 0,
                     int margin = 0, int spacing = 0,
                     Qt::Alignment childAlignment = Qt::AlignCenter,
                     Qt::Alignment innerAlignment = Qt::AlignCenter );

    ~KAboutContainer();

    void addWidget( QWidget *widget );

    void addPerson( const QString &name, const QString &email,
                    const QString &url, const QString &task,
                    bool showHeader = false, bool showframe = false,
                    bool showBold = false );

    void addTitle( const QString &title, Qt::Alignment alignment = Qt::AlignLeft,
                   bool showframe = false, bool showBold = false );

    void addImage( const QString &fileName, Qt::Alignment alignment = Qt::AlignLeft );

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

  protected:
    virtual void childEvent( QChildEvent *e );

  Q_SIGNALS:
    /**
     * Emitted when the user clicks on a (highlighted) URL in the
     * aboutbox. The @p url is passed in the signal.
     */
    void urlClick( const QString &url );

    /**
     * Emitted when the user clicks on a mailto: link in the aboutbox.
     * The mail address is passed as @p address; the parameter @p name
     * might be the name of the person being mailed (so that you can
     * build a complete address of the form name \<address\>.
     */
    void mailClick( const QString &name, const QString &address );

  private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY( KAboutContainer )
};

/**
 * Used internally by KAboutWidget
 * @internal
 */
class KDEUI_EXPORT KAboutContributor : public QFrame
{
  Q_OBJECT

  public:
    KAboutContributor( QWidget *parent = 0,
                       const QString &username = QString(),
                       const QString &email = QString(),
                       const QString &url = QString(),
                       const QString &work = QString(),
                       bool showHeader = false, bool showFrame = true,
                       bool showBold = false );

    ~KAboutContributor();

    void setName( const QString &text, const QString &header = QString(),
                  bool update = true );

    void setEmail( const QString &text, const QString &header = QString(),
                   bool update = true );

    void setUrl( const QString &text, const QString &header = QString(),
                 bool update = true );

    void setWork( const QString &text, const QString &header = QString(),
                  bool update = true );

    QString name() const;
    QString email() const;
    QString url() const;
    QString work() const;

    virtual QSize sizeHint() const;

  protected:
    virtual void fontChange( const QFont &oldFont );

  protected Q_SLOTS:
    void urlClickedSlot( const QString& );
    void emailClickedSlot( const QString& emailAddress );


  Q_SIGNALS:
    void sendEmail( const QString& name, const QString& email );
    void openUrl( const QString& url );

  private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY( KAboutContributor )

    virtual void setName(const char *_name) { QObject::setObjectName(_name); }
};

/**
 * KAboutWidget is the main widget for KAboutDialog.
 *
 * It has a minimum size set.
 */
class KDEUI_EXPORT KAboutWidget : public QWidget
{
  Q_OBJECT

  public:

    /**
     * Creates a new about widget.
     */
    KAboutWidget( QWidget* parent=0 );

    /**
     * Adjust the minimum size (after setting the properties of the image and
     * the labels.
     */
    void adjust();

    /**
     * Sets the image as the application logo.
     *
     * @param logo The application logo
     */
    void setLogo( const QPixmap &logo );

    /**
     * Sets the author's name and email address.
     */
    void setAuthor( const QString& name, const QString& email,
                    const QString& url, const QString& work );

    /**
     * Sets the maintainers name and email address.
     */
    void setMaintainer( const QString& name, const QString& email,
                        const QString& url, const QString& work );

    /**
     * Shows this person as one of the major contributors.
     */
    void addContributor( const QString& name, const QString& email,
                         const QString& url, const QString& work );

    /**
     * Sets the text describing the version.
     */
    void setVersion( const QString& name );

  Q_SIGNALS:
    /**
     * An email address has been selected by the user.
     */
    void sendEmail(const QString& name, const QString& email);

    /**
     * An URL has been clicked.
     */
    void openUrl(const QString& url);

  protected Q_SLOTS:
    /**
     * Catches the signals from the contributors elements.
     */
    void sendEmailSlot(const QString& name, const QString& email);

    /**
     * Catches the clicked URLs.
     */
    void openUrlSlot(const QString& url);

  protected:
    /**
     * The resize event.
     */
    void resizeEvent(QResizeEvent*);

    /**
     * The label showing the program version.
     */
    QLabel *version;

    /**
     * The label showing the text "Other contributors:".
     */
    QLabel *cont;

    /**
     * The frame showing the logo.
     */
    QLabel *logo;

    /**
     * The application developer.
     */
    KAboutContributor *author;

    /**
     * The application maintainer.
     */
    KAboutContributor *maintainer;

    /**
     * Show the maintainer?
     */
    bool showMaintainer;

    /**
     * A set of people who contributed to the application.
     */
    QList<KAboutContributor *> contributors;

  private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY( KAboutWidget )
};

/**
 * A KDialog with predefined main widget.
 *
 * As a KDialog it uses your application wide settings
 * for KDialog
 * objects (base frame tiles, main frame tiles etc).
 * To use it, simply create a KAboutDialog object, set all (or some) of its
 * properties and show it. Do not derive it to create your own about dialog
 * until you need some cool features that are unsupported and you have
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
 * For the derived features, see the basic class KDialog.
 * @author Mirko Boehm (mirko@kde.org) and Espen Sand (espensa@online.no)
 * @see KDialog
 */
class KDEUI_EXPORT KAboutDialog : public KDialog
{
  Q_OBJECT

  public:

    /**
     * Layout formats.
     **/
    enum LayoutType
    {
      Plain         = 0x0001,
      Tabbed        = 0x0002,
      Title         = 0x0004,
      ImageLeft     = 0x0008,
      ImageRight    = 0x0010,
      ImageOnly     = 0x0020,
      Product       = 0x0040,
      KDEStandard   = Tabbed | Title | ImageLeft,
      AppStandard   = Tabbed | Title | Product,
      ImageAndTitle = Plain | Title | ImageOnly
    };

  public:
    /**
     * The standard Qt constructor (Constructor I).
     *
     * Add components with the following methods:
     * setLogo(), setAuthor(), setMaintainer(),
     * addContributor(), or setVersion().
     * The dialog will be laid out automatically.
     */
    KAboutDialog( QWidget *parent=0 );

    /**
     * The extended constructor. (Constructor II).
     *
     * Add components with the methods:
     * setTitle(), setImage(), setImageBackgroundColor(),
     * setImageFrame(), setProduct(), addTextPage(),
     * addContainerPage(), addContainer(), or addPage().
     *
     * @param dialogLayout Use a mask of LayoutType flags.
     * @param caption The dialog caption. The text you specify is prepended
     *        by i18n("About").
     * @param parent Parent of the dialog.
     */
    KAboutDialog( int dialogLayout, const QString &caption, QWidget *parent=0 );

    /**
     * Adjusts the dialog.
     *
     * You can call this method after you have set up all
     * the contents but it is not required. It is done automatically when
     * show() is executed.
     */
    void adjust();

    /**
     * Makes a modeless dialog visible.
     *
     * If you reimplement this method make sure you run it
     * in the new method (e.g., show()). Reimplemented
     * from KDialog.
     */
    virtual void show();

    /**
     * Makes a modeless dialog visible.
     *
     * If you reimplmement this method make sure you run it
     * in the new method (i.e., show( parent )).
     *
     * @param centerParent Center the dialog with respect to this widget.
     */
    virtual void show( QWidget *centerParent );

    /**
     * (Constructor II only)
     * Sets a title (not caption) in the uppermost area of the dialog.
     *
     * @param title Title string.
     */
    void setTitle( const QString &title );

    /**
     * (Constructor II only)
     * Define an image to be shown in the dialog. The position is dependent
     * on the @p dialogLayout in the constructor
     *
     * @param fileName Path to image file.
     */
    void setImage( const QString &fileName );

    /**
     * Overloaded version of setProgramLogo(const QPixmap& pixmap).
     */
    void setProgramLogo( const QString &fileName );

    /**
     * (Constructor II only)
     * Define the program logo to be shown in the dialog.  Use this to override the
     * default program logo. For example, use this function if the
     * KAboutDialog is for a panel applet and you want to override the
     * appletproxy logo with your own pixmap.
     *
     * @param pixmap The logo pixmap.
     */
    void setProgramLogo( const QPixmap &pixmap );

    /**
     * (Constructor II only)
     * The image has a minimum size, but is centered within an area if the
     * dialog box is enlarged by the user. You set the background color
     * of the area with this method.
     *
     * @param color Background color.
     */
    void setImageBackgroundColor( const QColor &color );

    /**
     * (Constructor II only)
     * Enables or disables a frame around the image. The frame is, by default,
     * enabled in the constructor
     *
     * @param state A value of @p true enables the frame
     */
    void setImageFrame( bool state );

    /**
     * (Constructor II only)
     * Prints the application name, KDE version, author, a copyright sign
     * and a year string. To the left of the text the standard application
     * icon is displayed.
     *
     * @param appName The application name.
     * @param version Application version.
     * @param author One or more authors.
     * @param year A string telling when the application was made.
     */
    void setProduct( const QString &appName, const QString &version,
                     const QString &author, const QString &year );

    /**
     * (Constructor II only)
     * Adds a text page to a tab box. The text can be regular text or
     * rich text. The rich text can contain URLs and mail links.
     *
     * @param title Tab name.
     * @param text The text to display.
     * @param richText Set this to @p true if 'text' is rich text.
     * @param numLines The text area height will be adjusted so that this
     *        is the minimum number of lines of text that are visible.
     * @return The frame that contains the page.
     */
    QFrame *addTextPage( const QString &title, const QString &text,
                         bool richText=false, int numLines=10 );

    /**
     * (Constructor II only)
     * Adds a license page to a tab box.
     *
     * @param title Tab name.
     * @param text The text to display.
     * @param numLines The text area height will be adjusted so that this
     *        is the minimum number of lines of text that are visible.
     * @return The frame that contains the page.
     */
    QFrame *addLicensePage( const QString &title, const QString &text,
                            int numLines=10 );

    /**
     * (Constructor II only)
     * Adds a container to a tab box. You can add text and images to a
     * container.
     *
     * @param title Tab name.
     * @param childAlignment Specifies how the children of the container are
     *        aligned with respect to the container.
     * @param innerAlignment Specifies how the children are aligned with
     *        respect to each other.
     * @return The new container.
     */
    KAboutContainer *addContainerPage( const QString &title,
                                       Qt::Alignment childAlignment = Qt::AlignCenter,
                                       Qt::Alignment innerAlignment = Qt::AlignCenter );

    /**
     * (Constructor II only)
     * Adds a container inside a QScrollView to a tab box. You can add text
     * and images to a container.
     *
     * @param title Tab name.
     * @param childAlignment Specifies how the children of the container are
     *        aligned with respect to the container.
     * @param innerAlignment Specifies how the children are aligned with
     *        respect to each other.
     * @return The new container.
     */
    KAboutContainer *addScrolledContainerPage( const QString &title,
                                               Qt::Alignment childAlignment = Qt::AlignCenter,
                                               Qt::Alignment innerAlignment = Qt::AlignCenter );

    /**
     * (Constructor II only)
     * Adds a container. You can add text and images to a container.
     *
     * @param childAlignment Specifies how the children of the container are
     *        aligned with respect to the container.
     * @param innerAlignment Specifies how the children are aligned with
     *        respect to each other.
     * @return The new container.
     */
    KAboutContainer *addContainer( Qt::Alignment childAlignment, Qt::Alignment innerAlignment );

    /**
     * (Constructor II only)
     * Adds an empty page to a tab box.
     *
     * @param title Tab name
     * @return The new page.
     */
    QFrame *addPage( const QString &title );

    /**
     * (Constructor I only)
     * Sets the image as the application logo.
     */
    void setLogo( const QPixmap &logo );

    /**
     * (Constructor I only)
     * Sets the author's name and email address.
     */
    void setAuthor( const QString& name, const QString& email,
                    const QString& url, const QString& work );

    /**
     * (Constructor I only)
     * Sets the maintainer's name and email address.
     */
    void setMaintainer( const QString& name, const QString& email,
                        const QString& url, const QString& work );

    /**
     * (Constructor I only)
     * Show this person as one of the major contributors.
     */
    void addContributor( const QString& name, const QString& email,
                         const QString& url, const QString& work );

    /**
     * (Constructor I only)
     * Sets the text describing the version.
     */
    void setVersion( const QString& name );

    /**
     * Create a modal dialog with an image in the upper area with a
     * URL link below.
     */
    static void imageUrl( QWidget *parent, const QString &caption,
                          const QString &path, const QColor &imageColor,
                          const QString &url );

  Q_SIGNALS:
    /**
     * Send an email to this person.
     *
     * The application must provide the
     *  functionality.
     */
    void sendEmail( const QString& name, const QString& email );

    /**
     * Open the selected URL.
     */
    void openUrl( const QString& url );

  protected:
    /**
     * The main widget (Constructor I)
     */
    KAboutWidget *mAbout;

    /**
     * The main widget (Constructor II)
     */
    KAboutContainerBase *mContainerBase;

  protected Q_SLOTS:
    /**
     * Connected to widget->sendEmail.
     */
    void sendEmailSlot( const QString& name, const QString& email );

    /**
     * Open this URL.
     */
    void openUrlSlot( const QString& url );

    /**
     * (Constructor II only)
     * Tells the position of the mouse cursor when the left mouse button
     * is pressed above an image
     */
    virtual void mouseTrackSlot( int mode, const QMouseEvent *e );

  private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY( KAboutDialog )
};

#endif // defined KABOUTDIALOG_H

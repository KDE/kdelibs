/* -*- C++ -*-
 * A dialog class that serves as a common base class for different dialogs.
 * 
 * the K Desktop Environment
 * copyright:  (C) Mirko Sucker, 1998, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko@kde.org>
 * requires:   Qt 1.4x or later
 * $Revision$
 */

#ifndef DialogBase_included
#define DialogBase_included

#include <qstring.h>
#include <qsize.h>
#include <qdialog.h>

class QPixmap;
class QFrame;
class QPushButton;
class KURLLabel;

/** This base class provides basic functionality needed by nearly all dialogs.
 * It offers three standard buttons, \c OK , \c Apply and \c Cancel, a complete
 * geometry management that relies on the minimum size you set for the
 * contents, and the possibility to set background tiles for parts of the
 * window. It manages an outer and an inner frame to split the standard buttons
 * and the contents. 
 * <BR>
 * See the KDE addressbook for some examples how this dialogs look.
 *
 * \par The standard buttons
 * The buttons are configurable in some aspects and are already connected to
 * the appropriate slots: <OL>
 * <LI> OK: accept() (from QDialog) </LI>
 * <LI> Apply: applyPressed() (not defined by Qt, but declared virtual for
 *      overloading), sends signal "apply" to be connected to your code. </LI>
 * <LI> Cancel: reject() (from QDialog) </LI> </OL>
 *
 * \par The main widget
 * The inner frame of a freshly created object is empty. To use it, you need to
 * set a Qt widget as the main widget of the dialog using
 * setMainWidget(). If you do not want to
 * bother with geometry management yourselfes, just set a \e minimum size for
 * the widget. The dialog will automatically take care of it and ajust its own
 * minimum size. The size of the dialog is not fixed. <BR>
 * Often the dialog should be adjusted in its size on startup. The class does
 * not do this by default, as sometimes it is not wanted. After setting the
 * main widget the dialog has a minimum size set, so you may use: <BR>
 * <CODE> YourDialog.resize(YourDialog.minimumSize()); </CODE> <BR>
 * With this, your dialog starts up in its minimum size, but is still resizeable
 * by the user.
 *
 * \par The layout
 * The class inserts a base frame and an inner frame that are 
 * resized automatically if you call the \c resizeEvent function of this class
 * in \e your \c resizeEvent if you derived it.
 * Also the buttons are placed this way. The inner frame is 
 * set inside the base frame above of the buttons.
 * Both the base and the inner frame get background tiles as a static image 
 * provided by the class. This way you can achieve that all your dialogs look
 * the same way all over your application. All dialogs derived from this class 
 * used in one program get the background that is set centrally. <BR>
 * The class has been initially created using QtArchitect. The *Data* sources
 * created by it have been removed for performance reasons. */
 
class DialogBase : public QDialog
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /** The Qt standard constructor. */
  DialogBase(QWidget* parent=0, const char* name=0, bool modal=true);
  /** The destructor. */
  virtual ~DialogBase();
  /** This returns whether the background tile is set or not. */
  static bool haveBackgroundTile();
  /** This returns a const pointer to the background tile if there is one. */
  static const QPixmap* getBackgroundTile();
  /** This method sets the background tile. If it is Null (0), the background
   *  image is deleted. */
  static void setBackgroundTile(const QPixmap*);
  /** The background mode - either showing the selected tile or the standard
   *  background color - can be switched independently for the outer and the
   *  inner frame. This method sets this property for the inner frame. 
   *  If state is \c true, the background tile is displayed. If not, the
   *  standard background color is used. */
  void showBaseFrameTile(bool state);
  /** Select the background mode for the outer frame.
   *  @see setBackgroundTile */
  void showMainFrameTile(bool state);
  /** Enable the OK button if \a state is \c true (default). */
  void enableButtonOK(bool state);
  /** Enable the Apply button if \a state is \c true (default). */
  void enableButtonApply(bool state);
  /** Enable the Cancel button if \a state is \c true (default). */
  void enableButtonCancel(bool state);
  /** Set the text of the OK button. If the default parameters are used (that 
   *  means, if no parameters are given) the standard texts are set:
   *  The button shows \e OK, the tooltip contains "Accept settings." 
   *  (internationalized) and the quickhelp text explains the standard 
   *  behaviour of the OK button in dialogs. */
  void setButtonOKText(const QString& text="", const QString& tooltip="", 
		       const QString& quickhelp="");
  /** Set the text of the Apply button. If the default parameters are used (that 
   *  means, if no parameters are given) the standard texts are set:
   *  The button shows \e Apply, the tooltip contains "Apply settings." 
   *  (internationalized) and the quickhelp text explains the standard 
   *  behaviour of the apply button in dialogs. */
  void setButtonApplyText(const QString& text="", const QString& tooltip="", 
			  const QString& quickhelp="");
  /** Set the text of the Cancel button. If the default parameters are used (that 
   *  means, if no parameters are given) the standard texts are set:
   *  The button shows \e Cancel, the tooltip contains "Cancel settings." 
   *  (internationalized) and the quickhelp text explains the standard 
   *  behaviour of the cancel button in dialogs. */
  void setButtonCancelText(const QString& text="", const QString& tooltip="", 
			   const QString& quickhelp="");
  /** Show the OK button if \a state is \c true (default). */
  void showButtonOK(bool state);
  /** Show the Apply button if \a state is \c true (default). */
  void showButtonApply(bool state);
  /** Show the Cancel button if \a state is \c true (default). */
  void showButtonCancel(bool state);
  /** This method returns the contents rectangle of the inner frame. Place your
   *  widgets inside this rectangle, and use it to set up their geometry.
   *  Be carefull: The rectangle is only valid after resizing the dialog, as it
   *  is a result of the resizing process. If you need the "overhead" the dialog
   *  needs for its elements, use getBorderWidth. */
  QRect getContentsRect();
  /** This method returns the border widths in all directions that the dialog
   *  needs for itselfes. Respect this, or get bad looking results. 
   *  The references are upper left x, upper left y, lower left x and lower left
   *  y. The results are differences in pixels from the dialogs corners. */
  void getBorderWidths(int& ulx, int& uly, int& lrx, int& lry) const;
  /** With this method it is more easier to calculate a size hint for a dialog 
   *  derived from DialogBase if you now the width and height of the elements you
   *  add to the widget. The rectangle returned is calculated so that all 
   *  elements exactly fit into it. Thus, you may set it as a minimum size for
   *  the resulting dialog. */
  QSize calculateSize(int w, int h);
  /** This method is meant to make the management of simple layouts easier. You 
   *  may assign (and remove) a main widget to the dialog. This main widget will 
   *  be placed exactly fitting into the frame rectangle of the inner frame. 
   *  This is usefull if the widget you insert is managed by a Qt layout manager,
   *  for example (or by your own \c resizeEvent handler). Make sure to set the 
   *  minimum size of the dialog using one of the methods above (Note: if the
   *  main widget provided by you has a minimum size the dialogs minimum size is 
   *  \e automatically adapted to this size, no need to bother). <BR>
   *  If you set a main widget and there is already one, this previous main 
   *  widget will NOT be deleted. This way you may switch between different 
   *  widgets for example by hiding the old one and setting the new one as main.
   *  The dialog does not manipulate the visibility state of your widget, only 
   *  its geometry. <BR>
   *  If you replace a previous main widget, the old one is still visible. Hide
   *  it, or place it somewhere else. */
  void setMainWidget(QWidget*);
  /** Returns the main widget or zero if there is none. */
  QWidget* getMainWidget(); 
  // ----------------------------------------------------------------------------
protected:
  // members:
  static QPixmap *tile; /**< The background tile. */
  static const int Grid; /**< The default grid spacing. */
  static DialogBase *dummy; /**< \internal A dummy object for sending signals. */
  QWidget *main; /**< The main widget. */
  bool mainTile; /**< Show the outer frame background tile if \c true. */
  bool baseTile; /**< Show the base frame background tile if \c true. */
  QFrame* frameBase; /**< The outer frame. */
  QFrame* frameMain; /**< The inner frame. */
  QPushButton* buttonOK; /**< The OK button. */
  QPushButton* buttonApply; /**< The apply button. */
  QPushButton* buttonCancel; /**< The cancel button. */
  KURLLabel *kurlHelp; /**< The URL label for launching kdehelp. */
  QString path; /**< The path to the help file. */
  QString topic; /**< The help topic. */
  bool showHelp; /**< Is true if \e kurlHelp is visible and should be. */
  bool init; /**< False until constructor is done. */
  // methods:
  void emitBackgroundChanged(); /**< Emits the signal ::backgroundChanged. */
  void resizeEvent(QResizeEvent*); /**< The resize event. */
  static void cleanup(); /**< Cleanup function to delete static QWidgets. */
  // ----------------------------------------------------------------------------
protected slots:
  void applyPressed(); /**< Called when apply button is pressed. */
  // ----------------------------------------------------------------------------
signals:
  void apply(); /**< The apply button was pressed. */
  void backgroundChanged(); /**< The background tile has changed. */
  // ----------------------------------------------------------------------------
public slots:
  void updateBackground(); /**< Called when the background changed. */
  void initializeGeometry(); /**< Initialize dialog geometry. */
  /** Set the help topic, thereby enabling and showing the help url label.
   *  \e path is the path to the html file, \e text is the text shown by the
   *  label.
   *  If \e text is empty, the text is "Get help...", internationalized. 
   *  WORK_TO_DO: I had to remove the default value ("") for text, since Qt 2.0 
   *  moc shouted a syntax error in this line. Any comments how to fix it? */
  void setHelp(const QString& path,
	       const QString& topic,
	       const QString& text);
  void helpClickedSlot(const QString& url); /**< Connected to kurlHelp. */
  // ############################################################################
};

#endif // DialogBase_included











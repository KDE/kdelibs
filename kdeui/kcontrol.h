/*
  kcontrol - Base for KDE Control Applications

  written 1997 by Matthias Hoelzer

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

#ifndef _K_CONTROL_
#define _K_CONTROL_

#include <qtabbar.h>
#include <qtabdialog.h>
#include <qstrlist.h>
#include <qpushbutton.h>
#include <kapp.h>

// forward declarations
class KControlApplication;

/** 
 * KControlDialog is a QTabDialog that is internally used by KControl
 * Applications.
 *
 *  @see KConfigWidget, KControlApplication 
 */
class KControlDialog : public QTabDialog
{
  Q_OBJECT

  friend KControlApplication;

public:

  /**
   *  Constructor.
   *
   *  Initializes the dialog.
   */
  KControlDialog();

protected slots:

  /**
   *  Closing the dialog will end the application.
   */
  virtual void done( int );

};

/**
 *  The base widget for setup dialog.
 *
 *  It provides methods to load and apply the settings.
 *
 *  @see KControlApplication, KControlDialog
 */
class KConfigWidget : public QWidget
{
  Q_OBJECT

public:

  /**
   *  Constructor.
   *
   *  @param _parent Pointer to the parent widget.
   *  @param _name   Name of the widget.
   */
  KConfigWidget( QWidget* _parent, const char* _name = 0 )
  : QWidget( _parent, _name ) {};

  /**
   *  Loads the settings, usually from an rc-file.
   *
   *  This method is declared pure virtual and has to be overloaded.
   */
  virtual void loadSettings() = 0;

  /**
   *  Applies the settings.
   *
   *  This method is declared pure virtual and has to be overloaded.
   */
  virtual void applySettings() = 0;

  /**
   *  Sets default values.
   *
   *  By default this method does nothing. You have to overload it
   *  to set the default settings into the widgets.
   */
  virtual void defaultSettings() {};
};

/**
 *  KControlApplication is the common base for setup applications.
 *
 *  It provides a tab dialog and functionality common to most setup programs.
 *
 *  @see KControlDialog, KConfigWidget
 *
 *  @author Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 *  @short Common base for setup applications.
 */
class KControlApplication : public KApplication
{
  Q_OBJECT

public:

  /**
   *  Creates the setup application.
   *
   *  The constructor scans the command line arguments. If there is a
   *  single argument, "-init", the function init() is called and the
   *  application terminates.  Otherwise the setup dialog is created
   *  and inkoved.
   *
   *  @param argc  Number of commandline arguments
   *  @param argv  Commandline arguments
   *  @param _name Name of the application 
   */
   KControlApplication( int& argc, char** argv, const QCString& _name );

  /**
   * Destructor. Cleans up.
   */
  ~KControlApplication();

  /**
   *  Sets the title of the dialog.
   *
   *  It's not possible to set the title within the constructor,
   *  because we need the application to get the translator and
   *  it would mean a lot of effort to do it without the one in kapp.
   *
   *  @param _title Text to be shown in the dialogs titlebar
   */
  void setTitle( const QString& _title );

  /**
   *  Determines if the setup dialog has to be run.
   *
   *  The setup dialog has to be run if the application has not been
   *  invoked with a single commandline argument containing "-init".
   *
   *  Due to the fact the QApplication::exec() is not virtual, this
   *  construction has to be used to execute a KControlApplication:
   *
   *  <pre>
   *  KControlApplication app(argc, argv, "name", "title");
   *  app.createPages();
   *
   *  if (app.runGUI())
   *    return app.exec();
   *  else
   *    return 0;
   *  </pre>
   *
   *  Just running app.exec() will fail if "-init" has been requested.  
   */
  bool runGUI() { return !justInit; };

  /** 
   *  Returns the tabbed dialog object, normally a @ref KControlDialog
   *  object.
   *
   *  @return Pointer to a QTabDialog object.  */
  QTabDialog* getDialog() { return dialog; };

  /**
   *  Returns the list of pages to show.
   *
   *  @return Pointer to a QStrList obejct or NULL if no special pages
   *  choosen.  
   */
  QStrList* getPageList() { return pages; };

  /**
   *  Adds a new page to the dialog.
   *
   *  @param _page      Pointer to a widget, that shall be added as page.
   *  @param _name      Name of the page.
   *  @param _help_name Name of the help page for this page.
   */
  void addPage( QWidget* _page, const QString& _name, const QString& _help_name );

public slots:

  /**
   *  This function is called at startup to initialize the settings.
   *   
   *  This function must be overriden by all setup application that
   *  want to have persistent settings.  
   */
  virtual void init() {};

  /** 
   *  This function is called to apply the settings currently selected
   *  in the dialog.
   *
   *  This function must be overriden by all setup applications.  */
  virtual void apply() {};

  /**
   *  This function is called when the help button is pressed.
   *   
   *  The default behaviour is to call
   *
   *  kapp->invokeHTMLHelp("kcontrol/$(appname)/$(help_name).html","");
   */
  virtual void help();

  /**
   *  This function is called when the user presses the default button.
   *
   *  This function must be overriden by all setup application.
   */
  virtual void defaultValues() {};

protected:

  /**
   *  Pointer to the KControlDialog object.
   */
  KControlDialog* dialog;

  /**
   *  Pointer to QStrList object for all pages that shall be shown
   *  or NULL if all shall be shown.
   */
  QStrList*       pages;

  QStrList        helpNames;
  bool            justInit;

private:

  QString        swallowCaption;
};


#endif


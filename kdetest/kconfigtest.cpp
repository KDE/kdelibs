// $Id$

/* $Log$
 *
 * Revision 1.1  1997/04/13 17:49:40  kulow
 * Sources imported
 *
 * Revision 1.7  1997/03/16 22:53:19  kalle
 * KTextStream->QTextStream
 *
 * Revision 1.6  1996/12/14 14:01:53  kalle
 * method names start with small letters
 *
 * Revision 1.5  1996/11/30 19:51:21  kalle
 * open app-specific file read-write
 *
 * Revision 1.4  1996/11/23 21:16:17  kalle
 * KTextStream statt QTextStream
 *
 * Revision 1.3  1996/11/20 20:14:11  kalle
 * ValueEdit instead of ValueLabel, WriteButton added
 *
 * Revision 1.2  1996/11/15 18:22:51  kalle
 * all slots return void
 *
 * Revision 1.1  1996/11/14 20:09:06  kalle
 * Initial revision
 *
 * Revision 1.10  1996/11/03 21:01:25  kalle
 * "found in" line removed (for version 0.2)
 *
 * Revision 1.9  1996/10/26 19:09:47  kalle
 * Show the file the key/value pair was found in.
 *
 * Revision 1.8  1996/10/26 18:55:57  kalle
 * Use HasKey() in KeyEditReturnPressed()
 *
 * Revision 1.7  1996/10/26 18:49:20  kalle
 * Show value "---" if the key was not found
 *
 * Revision 1.6  1996/10/26 18:47:24  kalle
 * info messages about finding a key
 *
 * Revision 1.5  1996/10/26 18:42:44  kalle
 * call default group <default> in group message field
 *
 * Revision 1.4  1996/10/26 18:32:29  kalle
 * check for existence of app config file, open it if necessary
 *
 * Revision 1.3  1996/10/26 18:08:59  kalle
 * KeyEditReturnPressed implemented, works for simple cases
 *
 * Revision 1.2  1996/10/26 09:06:53  kalle
 * AppConfigEditReturnPressed und GroupEditReturnPressed implementiert
 *
 * Revision 1.1  1996/10/25 18:57:35  kalle
 * Initial revision
 *
 *
 */

//
// configtest.cpp: libKDEcore example
//
// demonstrates use of KConfig class
#include <qapp.h>
// adapted from Qt widgets demo


#include <qdialog.h>
#include <qfile.h>
#include <qfileinf.h>
#include <kdebug.h>

// Standard Qt widgets

#include <qlabel.h>
#include <Kconfig.h>
#include <qpushbt.h>

// KDE includes
#include <kconfig.h>

class KConfigTestView : public QDialog
{
  Q_OBJECT
public:
  KConfigTestView( QWidget *parent=0, const char *name=0 );
  ~KConfigTestView();

private slots:
  void appConfigEditReturnPressed();
  void groupEditReturnPressed();
  void keyEditReturnPressed();
  void writeButtonClicked();

private:
  QLabel* pAppFileLabel;
  QLineEdit* pAppFileEdit;
  QLabel* pGroupLabel;
  QLineEdit* pGroupEdit;
  QLineEdit* pKeyEdit;
  QLabel* pEqualsLabel;
  QLineEdit* pValueEdit;
  QPushButton* pWriteButton;
  QLabel* pInfoLabel1, *pInfoLabel2;
  QPushButton* pQuitButton;

  KConfig* pConfig;
  QFile* pFile;
  QTextStream* pStream;
};

//

#include "kconfigtest.h"
#include "kconfigtest.moc"
//
// Construct the KConfigTestView with buttons
      pConfig( NULL ),
      pFile( NULL ),
      pStream( NULL )
    : QDialog( parent, name ),
      pConfig( 0L ),
      pFile( 0L ),
      pStream( 0L )
{
  // Set the window caption/title

  setCaption( "KConfig test" );

  // Label and edit for the app config file
  pAppFileLabel = new QLabel( this, "appconfiglabel" );
  pAppFileLabel->setText( "Application config file:" );
  pAppFileLabel->setGeometry( 20, 20, 200, 20 );

  pAppFileEdit = new QLineEdit( this, "appconfigedit" );
  pAppFileEdit->setGeometry( 240, 20, 160, 20 ); 
  connect( pAppFileEdit, SIGNAL(returnPressed()),
	   SLOT(appConfigEditReturnPressed()));
  
  // Label and edit for the group
  pGroupLabel = new QLabel( this, "grouplabel" );
  pGroupLabel->setText( "Group:" );
  pGroupLabel->setGeometry( 20, 60, 80, 20 );

  pGroupEdit = new QLineEdit( this, "groupedit" );
  pGroupEdit->setGeometry( 120, 60, 100, 20 );
  connect( pGroupEdit, SIGNAL(returnPressed()),
	   SLOT(groupEditReturnPressed()));

  // Edit and label for the key/value pair
  pKeyEdit = new QLineEdit( this, "keyedit" );
  pKeyEdit->setGeometry( 20, 100, 80, 20 );
  connect( pKeyEdit, SIGNAL( returnPressed()),
	   SLOT(keyEditReturnPressed()));

  pEqualsLabel = new QLabel( this, "equalslabel" );
  pEqualsLabel->setGeometry( 105, 100, 20, 20 );
  pEqualsLabel->setText( "=" );

  pValueEdit = new QLineEdit( this, "valueedit" );
  pValueEdit->setGeometry( 120, 100, 100, 20 );
  pValueEdit->setText( "---" );

  pWriteButton = new QPushButton( this, "writebutton" );
  pWriteButton->setGeometry( 20,140, 80, 20 );
  pWriteButton->setText( "Write entry" );
  connect( pWriteButton, SIGNAL(clicked()), SLOT( writeButtonClicked() ) );

  // Labels for the info line
  pInfoLabel1 = new QLabel( this, "infolabel1" );
  pInfoLabel1->setGeometry( 20, 200, 60, 20 );
  pInfoLabel1->setText( "Info:" );

  pInfoLabel2 = new QLabel( this, "infolabel2" );
  pInfoLabel2->setGeometry( 100, 200, 300, 20 );
  pInfoLabel2->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  // Quit button
  pQuitButton = new QPushButton( this, "quitbutton" );
  pQuitButton->setText( "Quit" );
  pConfig = new KConfig();
  connect( pQuitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );

  // create a default KConfig object in order to be able to start right away
  pConfig = new KConfig( 0L );
}

KConfigTestView::~KConfigTestView()
{
  if( pConfig )
      delete pConfig;
  if( pFile )
      delete pFile;
  if( pStream )
      delete pStream;
}  

void KConfigTestView::appConfigEditReturnPressed()
{
      pConfig = NULL;
  if( pConfig )
    {
      delete pConfig;
      pConfig = 0L;
      pFile = NULL;
  if( pFile )
    {
      delete pFile;
      pFile = 0L;
      pStream = NULL;
  if( pStream )
    {
      delete pStream;
      pStream = 0L;
    }
      pFile = new QFile( pAppFileEdit->text() );
	  pFile->open( IO_ReadWrite );
	  pStream = new QTextStream( pFile );
  // create a new config object
  if( strlen( pAppFileEdit->text() ) )
  pConfig = new KConfig( pStream );

    {
	  pConfig = new KConfig( pAppFileEdit->text() );
    }
  
  pInfoLabel2->setText( "New config object created." ); 
}

void KConfigTestView::groupEditReturnPressed()
{
  aText.sprintf( "Group set to %s", pConfig->getGroup().isEmpty() ?
		 "<default>" : (const char*)pConfig->getGroup() );
  // test program this will do
  QString aText;
  aText.sprintf( "Group set to %s", QString( pConfig->group() ).isEmpty() ?
		 "<default>" : pConfig->group() );
  pInfoLabel2->setText( aText );
}

void KConfigTestView::keyEditReturnPressed()
{
  QString aValue = pConfig->readEntry( pKeyEdit->text() );
  // just checking aValue.isNull() would be easier here, but this is
  // to demonstrate the HasKey()-method. Besides, it is better data
  // encapsulation because we do not make any assumption about coding
  // non-values here.
  if( !pConfig->hasKey( pKeyEdit->text() ) )
    {
      pInfoLabel2->setText( "Key not found!" );
      pValueEdit->setText( "---" );
    }
  else
    {
      pInfoLabel2->setText( "Key found!" );
      pValueEdit->setText( aValue );
    }
  pConfig->writeEntry( pKeyEdit->text(), pValueEdit->text() );

  pConfig->writeEntry( pKeyEdit->text(), QString( pValueEdit->text() ) );

//
// Include the meta-object code for our own classes
//

#include "kconfigtest.moc"
  pInfoLabel2->setText( "Entry written" );

  KASSERT1( true, KDEBUG_INFO, 1, "Entry written: %d", 27 );
}

//
// Create and display our KConfigTestView.
    QApplication  a( argc, argv );
    KConfigTestView   *w = new KConfigTestView;
    a.setMainWidget( w );
    w->show();
    return a.exec();
	fprintf( stderr, "Recover file does not exist, use %s\n", pRecoverFile );
  free( pRecoverFile );

  return a.exec();
}


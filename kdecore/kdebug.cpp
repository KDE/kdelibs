#include "kdebugdialog.h"
#include "kdebugdialog.moc"
#include "kdebug.h"
#include "kapp.h"

#include <qfile.h>
#include <qmsgbox.h>

#include <stdlib.h> // abort

void kdebug( unsigned short nLevel, unsigned int nArea, const char* pString )
{
  // FIXME: Check if this is in a desired area, bail out if not

  // Save old group
  QString aOldGroup = kapp->getConfig()->getGroup();
  kapp->getConfig()->setGroup( "KDebug" );

  /* Determine output */
  short nOutput = 0;
  QString aCaption;
  switch( nLevel )
	{
	case KDEBUG_INFO:
	  nOutput = kapp->getConfig()->readNumEntry( "InfoOutput", 1 );
	  fprintf( stderr, "Info output goes to %d\n", nOutput );
	  aCaption = "Info";
	  break;
	case KDEBUG_WARN:
	  nOutput = kapp->getConfig()->readNumEntry( "WarnOutput", 1 );
	  aCaption = "Warning";
	  break;
	case KDEBUG_FATAL:
	  nOutput = kapp->getConfig()->readNumEntry( "FatalOutput", 1 );
	  aCaption = "Fatal Error";
	  break;
	case KDEBUG_ERROR:
	default:
	  /* Programmer error, use "Error" as default */
	  nOutput = kapp->getConfig()->readNumEntry( "ErrorOutput", 1 );
	  aCaption = "Error";
	  break;
	};

  // Output
  switch( nOutput )
	{
	case 0: // File
	  {
		QString aOutputFileName;
		switch( nLevel )
		  {
		  case KDEBUG_INFO:
			aOutputFileName = kapp->getConfig()->readEntry( "InfoFilename",
															"kdebug.dbg" );
			break;
		  case KDEBUG_WARN:
			aOutputFileName = kapp->getConfig()->readEntry( "WarnFilename",
															"kdebug.dbg" );
			break;
		  case KDEBUG_FATAL:
			aOutputFileName = kapp->getConfig()->readEntry( "FatalFilename",
															"kdebug.dbg" );
			break;
		  case KDEBUG_ERROR:
		  default:
			aOutputFileName = kapp->getConfig()->readEntry( "ErrorFilename",
															"kdebug.dbg" );
			break;
		  };
		QFile aOutputFile( aOutputFileName );
		aOutputFile.open( IO_WriteOnly );
		aOutputFile.writeBlock( pString, strlen( pString ));
		aOutputFile.writeBlock( "\n", 1 );
		aOutputFile.close();
		break;
	  }
	case 1: // Message Box
	  {
		// Since we are in kdecore here, we cannot use KMsgBox and use
		// QMessageBox instead 
		QMessageBox::message( aCaption, pString, "OK" );
		break;
	  }
	case 2: // Shell
	  {
		fprintf( stderr, "%s\n", pString );
		break;
	  }
	}

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL ) &&
	  ( kapp->getConfig()->readNumEntry( "AbortFatal", 0 ) ) )
	abort();

  // restore old group
  kapp->getConfig()->setGroup( aOldGroup );
}


KDebugDialog::KDebugDialog() :
  QDialog( NULL, "Debug Settings", true )
{
  pInfoGroup = new QGroupBox( "Information", this );
  pInfoGroup->setGeometry( 5, 10, 140, 185 );
  pInfoGroup->show();
  pInfoLabel1 = new QLabel( "Output to:", this );
  pInfoLabel1->setGeometry( 15, 30, 120, 15 );
  pInfoLabel1->show();
  pInfoCombo = new QComboBox( false, this );
  pInfoCombo->setGeometry( 15, 50, 120, 20 );
  pInfoCombo->insertItem( "File" );
  pInfoCombo->insertItem( "Message Box" );
  pInfoCombo->insertItem( "Shell" );
  pInfoCombo->show();
  pInfoLabel2 = new QLabel( "Filename:", this );
  pInfoLabel2->setGeometry( 15, 85, 120, 15 );
  pInfoLabel2->show();
  pInfoFile = new QLineEdit( this );
  pInfoFile->setGeometry( 15, 105, 120, 20 );
  pInfoFile->show();
  pInfoLabel3 = new QLabel( "Show only area(s):", this );
  pInfoLabel3->setGeometry( 15, 140, 120, 15 );
  pInfoLabel3->show();
  pInfoShow = new QLineEdit( this );
  pInfoShow->setGeometry( 15, 160, 120, 20 );
  pInfoShow->show();
  
  pWarnGroup = new QGroupBox( "Warning", this );
  pWarnGroup->setGeometry( 165, 10, 140, 185 );
  pWarnGroup->show();
  pWarnLabel1 = new QLabel( "Output to:", this );
  pWarnLabel1->setGeometry( 175, 30, 120, 15 );
  pWarnLabel1->show();
  pWarnCombo = new QComboBox( false, this );
  pWarnCombo->setGeometry( 175, 50, 120, 20 );
  pWarnCombo->insertItem( "File" );
  pWarnCombo->insertItem( "Message Box" );
  pWarnCombo->insertItem( "Shell" );
  pWarnCombo->show();
  pWarnLabel2 = new QLabel( "Filename:", this );
  pWarnLabel2->setGeometry( 175, 85, 120, 15 );
  pWarnLabel2->show();
  pWarnFile = new QLineEdit( this );
  pWarnFile->setGeometry( 175, 105, 120, 20 );
  pWarnFile->show();
  pWarnLabel3 = new QLabel( "Show only area(s):", this );
  pWarnLabel3->setGeometry( 175, 140, 120, 15 );
  pWarnLabel3->show();
  pWarnShow = new QLineEdit( this );
  pWarnShow->setGeometry( 175, 160, 120, 20 );
  pWarnShow->show();

  pErrorGroup = new QGroupBox( "Error", this );
  pErrorGroup->setGeometry( 5, 215, 140, 185 );
  pErrorGroup->show();
  pErrorLabel1 = new QLabel( "Output to:", this );
  pErrorLabel1->setGeometry( 15, 235, 120, 15 );
  pErrorLabel1->show();
  pErrorCombo = new QComboBox( false, this );
  pErrorCombo->setGeometry( 15, 255, 120, 20 );
  pErrorCombo->insertItem( "File" );
  pErrorCombo->insertItem( "Message Box" );
  pErrorCombo->insertItem( "Shell" );
  pErrorCombo->show();
  pErrorLabel2 = new QLabel( "Filename:", this );
  pErrorLabel2->setGeometry( 15, 290, 120, 15 );
  pErrorLabel2->show();
  pErrorFile = new QLineEdit( this );
  pErrorFile->setGeometry( 15, 310, 120, 20 );
  pErrorFile->show();
  pErrorLabel3 = new QLabel( "Show only area(s):", this );
  pErrorLabel3->setGeometry( 15, 345, 120, 15 );
  pErrorLabel3->show();
  pErrorShow = new QLineEdit( this );
  pErrorShow->setGeometry( 15, 365, 120, 20 );
  pErrorShow->show();

  pFatalGroup = new QGroupBox( "Fatal error", this );
  pFatalGroup->setGeometry( 165, 215, 140, 185 );
  pFatalGroup->show();
  pFatalLabel1 = new QLabel( "Output to:", this );
  pFatalLabel1->setGeometry( 175, 235, 120, 15 );
  pFatalLabel1->show();
  pFatalCombo = new QComboBox( false, this );
  pFatalCombo->setGeometry( 175, 255, 120, 20 );
  pFatalCombo->insertItem( "File" );
  pFatalCombo->insertItem( "Message Box" );
  pFatalCombo->insertItem( "Shell" );
  pFatalCombo->show();
  pFatalLabel2 = new QLabel( "Filename:", this );
  pFatalLabel2->setGeometry( 175, 290, 120, 15 );
  pFatalLabel2->show();
  pFatalFile = new QLineEdit( this );
  pFatalFile->setGeometry( 175, 310, 100, 20 );
  pFatalFile->show();
  pFatalLabel3 = new QLabel( "Show only area(s):", this );
  pFatalLabel3->setGeometry( 175, 345, 120, 15 );
  pFatalLabel3->show();
  pFatalShow = new QLineEdit( this );
  pFatalShow->setGeometry( 175, 365, 120, 20 );
  pFatalShow->show();

  pAbortFatal = new QCheckBox( "Abort on fatal errors", this );
  pAbortFatal->setGeometry( 15, 420, 200, 15 );
  pAbortFatal->show();
  pOKButton = new QPushButton( "OK", this );
  pOKButton->setGeometry( 15, 460, 80, 20 );
  pOKButton->setDefault( true );
  pOKButton->show();
  connect( pOKButton, SIGNAL( clicked() ), SLOT( accept() ) );
  pCancelButton = new QPushButton( "Cancel", this );
  pCancelButton->setGeometry( 110, 460, 80, 20 );
  pCancelButton->show();
  connect( pCancelButton, SIGNAL( clicked() ), SLOT( reject() ) );
  pHelpButton = new QPushButton( "Help", this );
  pHelpButton->setGeometry( 205, 460, 80, 20 );
  pHelpButton->show();
}

KDebugDialog::~KDebugDialog()
{
  delete pInfoGroup;
  delete pInfoLabel1;
  delete pInfoCombo;
  delete pInfoLabel2;
  delete pInfoFile;
  delete pInfoLabel3;
  delete pInfoShow;
  delete pWarnGroup;
  delete pWarnLabel1;
  delete pWarnCombo;
  delete pWarnLabel2;
  delete pWarnFile;
  delete pWarnLabel3;
  delete pWarnShow;
  delete pErrorGroup;
  delete pErrorLabel1;
  delete pErrorCombo;
  delete pErrorLabel2;
  delete pErrorFile;
  delete pErrorLabel3;
  delete pErrorShow;
  delete pFatalGroup;
  delete pFatalLabel1;
  delete pFatalCombo;
  delete pFatalLabel2;
  delete pFatalFile;
  delete pFatalLabel3;
  delete pFatalShow;
  delete pAbortFatal;
  delete pOKButton;
  delete pCancelButton;
  delete pHelpButton;
}





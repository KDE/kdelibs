#include "kdebugdialog.h"
#include "kdebugdialog.moc"
#include "kdebug.h"
#include "kapp.h"

#include <qfile.h>
#include <qmsgbox.h>
#include <qbitarry.h>

#include <stdlib.h> // abort
#include <stdarg.h> // vararg stuff
#include <syslog.h>
#include <math.h> // pow

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

bool bAreaCalculated = false;
QBitArray *pInfoArray = NULL, *pWarnArray = NULL,
  *pErrorArray = NULL, *pFatalArray = NULL;

static void recalculateAreaBits( QBitArray*, QString* );
static void evalToken( QBitArray*, QString* );

void kdebug( ushort nLevel, ushort nArea, 
			 const char* pFormat, ... )
{
  // Save old group
  QString aOldGroup = kapp->getConfig()->getGroup();
  kapp->getConfig()->setGroup( "KDebug" );
  
  /* The QBitArrays should rather be application-static, but since
	 some stupid platforms do not support that... */
  if( !bAreaCalculated )
	{
	  // check to see if we need to create the bit arrays first
	  static uint size = (uint)rint( pow( 2, (int)(sizeof( ushort ) * 8 )  ) );
	  if( !pInfoArray ) 
		pInfoArray = new QBitArray( size );
	  if( !pWarnArray ) 
		pWarnArray = new QBitArray( size );
	  if( !pErrorArray ) 
		pErrorArray = new QBitArray( size );
	  if( !pFatalArray ) 
		pFatalArray = new QBitArray( size );
	  
	  // (re)calculate the areas from their textual representations
	  pInfoArray->fill( false );
	  QString aInfoArea = kapp->getConfig()->readEntry( "InfoShow", "" );
	  recalculateAreaBits( pInfoArray, &aInfoArea );
	  pWarnArray->fill( false );
	  QString aWarnArea = kapp->getConfig()->readEntry( "WarnShow", "" );
	  recalculateAreaBits( pWarnArray, &aWarnArea );
	  pErrorArray->fill( false );
	  QString aErrorArea = kapp->getConfig()->readEntry( "ErrorShow", "" );
	  recalculateAreaBits( pErrorArray, &aErrorArea );
	  pFatalArray->fill( false );
	  QString aFatalArea = kapp->getConfig()->readEntry( "FatalShow", "" );
	  recalculateAreaBits( pFatalArray, &aFatalArea );
	  
	  bAreaCalculated = true;
	}

  // Check if this is in a desired area, bail out if not
  switch( nLevel )
	{
	case KDEBUG_INFO:
	  if( !pInfoArray->testBit( nArea ) )
		  return;
	  break;
	case KDEBUG_WARN:
	  if( !pWarnArray->testBit( nArea ) )
		return;
	  break;
	case KDEBUG_FATAL:
	  if( !pFatalArray->testBit( nArea ) )
		return;
	  break;
	case KDEBUG_ERROR:
	default:
	  if( !pErrorArray->testBit( nArea ) )
		return;
	};

  va_list arguments; /* Handle variable arguments */

  /* Determine output */
  short nOutput = 0;
  int nPriority = 0; // for syslog
  QString aCaption;
  switch( nLevel )
	{
	case KDEBUG_INFO:
	  nOutput = kapp->getConfig()->readNumEntry( "InfoOutput", 1 );
	  aCaption = "Info";
	  nPriority = LOG_INFO;
	  break;
	case KDEBUG_WARN:
	  nOutput = kapp->getConfig()->readNumEntry( "WarnOutput", 1 );
	  aCaption = "Warning";
	  nPriority = LOG_WARNING;
	  break;
	case KDEBUG_FATAL:
	  nOutput = kapp->getConfig()->readNumEntry( "FatalOutput", 1 );
	  aCaption = "Fatal Error";
	  nPriority = LOG_CRIT;
	  break;
	case KDEBUG_ERROR:
	default:
	  /* Programmer error, use "Error" as default */
	  nOutput = kapp->getConfig()->readNumEntry( "ErrorOutput", 1 );
	  aCaption = "Error";
	  nPriority = LOG_ERR;
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
		char buf[4096];
		QString aAppName = kapp->appName();
		int nPrefix = sprintf( buf, "%s: ", aAppName.data() );
		va_start( arguments, pFormat );
#ifdef HAVE_VSNPRINTF
		// use the more secure version if we have it
		int nSize = vsnprintf( buf, 4095, pFormat, arguments );
#else
		int nSize = vsprintf( buf, pFormat, arguments );
#endif
		if( nSize > (4094-nPrefix) ) nSize = 4094-nPrefix;
		buf[nSize] = '\n';
		buf[nSize+1] = '\0';
		va_end( arguments );
		QFile aOutputFile( aOutputFileName );
		aOutputFile.open( IO_WriteOnly );
		aOutputFile.writeBlock( buf, nSize+2 );
		aOutputFile.close();
		break;
	  }
	case 1: // Message Box
	  {
		// Since we are in kdecore here, we cannot use KMsgBox and use
		// QMessageBox instead 
		char buf[4096]; // constants are evil, but this is evil code anyway
		va_start( arguments, pFormat );
		int nSize = vsprintf( buf, pFormat, arguments );
		if( nSize > 4094 ) nSize = 4094;
		buf[nSize] = '\n';
		buf[nSize+1] = '\0';
		va_end( arguments );
		QMessageBox::message( aCaption, buf, "OK" );
		break;
	  }
	case 2: // Shell
	  {
		va_start( arguments, pFormat );
		fprintf( stderr, "%s: ", kapp->appName().data() );
		vfprintf( stderr, pFormat, arguments );
		fprintf( stderr, "\n" );
		va_end( arguments );
		break;
	  }
	case 3: // syslog
	  {
		char buf[4096];
		QString aAppName = kapp->appName();
		int nPrefix = sprintf( buf, "%s: ", aAppName.data() );
		va_start( arguments, pFormat );
		int nSize = vsprintf( &buf[nPrefix], pFormat, arguments );
		if( nSize > (4094-nPrefix) ) nSize = 4094-nPrefix;
		buf[nSize] = '\n';
		buf[nSize+1] = '\0';
		va_end( arguments );
		syslog( nPriority, buf );
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
  pInfoCombo->insertItem( "Syslog" );
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
  pWarnCombo->insertItem( "Syslog" ); 
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
  pErrorCombo->insertItem( "Syslog" );
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
  pFatalCombo->insertItem( "Syslog" );
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
  connect( pHelpButton, SIGNAL( clicked() ), SLOT( showHelp() ) );
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


void KDebugDialog::showHelp()
{
  kapp->invokeHTMLHelp( "kdelib/kdebug.html", "" );
}

void recalculateAreaBits( QBitArray* pArray, QString* pString )
{
  // string could be empty
  if( pString->isEmpty() )
	{
	  pArray->fill( true ); // all bits are set
	  return;
	}

  // isolate the tokens
  uint pos = 0; uint newpos = 0;
  while( ( newpos = pString->find( ',', pos, false ) ) != (uint)-1 )
	{
	  QString aToken = pString->mid( pos, newpos-pos );
	  evalToken( pArray, &aToken );
	  pos = newpos+1;
	};
  
  // there is probably the last part left
  if( pos != pString->length() )
	{
	  QString aLastToken = pString->right( pString->length() - pos );
	  evalToken( pArray, &aLastToken );
	}
}


void evalToken( QBitArray* pArray, QString* pString )
{
  int dashpos = 0;
  if( ( dashpos = pString->find( '-' ) ) != -1 )
	{
	  // range of areas
	  QString aLeft = pString->mid( 0, dashpos );
	  QString aRight = pString->mid( dashpos+1, pString->length()-1 );
	  for( uint i = aLeft.toUInt(); i <= aRight.toUInt(); i++ )
		{
		  pArray->setBit( i );
		}
	}
  else
	{
	  // single area
	  pArray->setBit( pString->toUInt() );
	}
}

/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

// $Id$

#include "kdebugdialog.h"
#include "kdebug.h"
#include "kapp.h"
#include "kglobal.h"
#include "kstddirs.h"
#include <qmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qlayout.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h>	// abort
#include <stdarg.h>	// vararg stuff
#include <syslog.h>
#include <math.h>
#include <kconfig.h>	// pow

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class KDebugEntry;
template QList<KDebugEntry>;

#ifdef kdebug
#undef kdebug
#endif

class KDebugEntry
{ 
public:
  KDebugEntry (int n, QString d) {number=n; descr=d.copy();}
  KDebugEntry (QString d, int n) {number=n; descr=d.copy();}
  unsigned int number;
  QString descr;
};

static QList<KDebugEntry> *KDebugCache;
#define MAX_CACHE 20

static QString getDescrFromNum(unsigned short _num)
{
  if (!KDebugCache)
      KDebugCache = new QList<KDebugEntry>;

  for ( KDebugEntry *ent = KDebugCache->first(); 
		  ent != 0; ent = KDebugCache->next()) {
	  if (ent->number == _num) {
		  return ent->descr.copy();
	  }
  }

  QString data, filename(locate("config","kdebug.areas"));
  QFile file(filename);
  if (!file.open(IO_ReadOnly)) {
    warning("Couldn't open %s", filename.ascii());
    file.close();
    return "";
  }

  unsigned long number, space;
  bool longOK;

  QTextStream *ts = new QTextStream(&file);
  while (!ts->eof()) {
    data=ts->readLine().stripWhiteSpace().copy();

    if (data.at(0) == '#')
      continue; // It's a comment

    if (data.find("#") != -1) {
      data.remove(data.find("#"), data.length());
      data=data.stripWhiteSpace();
    }

    if (data.isEmpty() || data.isNull())
      continue;

    if ( (data.find(" ") == -1) && (data.find("	") == -1) )
      continue; // It only has one "part", need two

    if (data.find(" ") == -1)
      space=data.find("	");
    else if (data.find("	") == -1)
      space=data.find(" ");
    else if (data.find(" ") < data.find("	"))
      space=data.find(" ");
    else
      space=data.find("	");

    number=data.left(space).toULong(&longOK);
    if (!longOK)
      continue; // The first part wasn't a number

    if (number != _num)
      continue; // Not the number we're looking for

    data.remove(0, space); data=data.stripWhiteSpace();

    if (KDebugCache->count() >= MAX_CACHE)
      KDebugCache->removeFirst();
    KDebugCache->append(new KDebugEntry(number,data.copy()));
    delete ts;
    file.close();
    return data.copy();
  }

  delete ts;
  file.close();
  return "";
}

void kdebug_null( ushort /*nLevel*/, ushort /*nArea*/, 
		  const char* /*pFormat*/, ... )
{
  return;
}

static int getNum (const char *key, int _default)
{
    return KGlobal::config()->readNumEntry(key, _default);
}

static QString getString (const char *key, const char * _default)
{
    return KGlobal::config()->readEntry(key, _default);
}

void kdebug( ushort nLevel, ushort nArea, 
	     const char* pFormat, ... )
{
  // Save old grou
  QString aOldGroup;
  if (kapp) {
    aOldGroup = KGlobal::config()->group();
    KGlobal::config()->setGroup( "KDebug" );
  }
  /* The QBitArrays should rather be application-static, but since
         some stupid platforms do not support that... */
  va_list arguments; /* Handle variable arguments */

  /* Determine output */
  short nOutput = 0;
  int nPriority = 0; // for syslog
  QString aCaption;
  QString aAppName = getDescrFromNum(nArea);
  if (aAppName.isEmpty())
    aAppName=kapp?kapp->name():"unknown";
  switch( nLevel )
        {
        case KDEBUG_INFO:
          nOutput = getNum( "InfoOutput", 2 );
          aCaption = "Info (" + aAppName + ")";
          nPriority = LOG_INFO;
          break;
        case KDEBUG_WARN:
          nOutput = getNum( "WarnOutput", 2 );
          aCaption = "Warning (" + aAppName + ")";
          nPriority = LOG_WARNING;
          break;
        case KDEBUG_FATAL:
          nOutput = getNum( "FatalOutput", 2 );
          aCaption = "Fatal Error (" + aAppName + ")";
          nPriority = LOG_CRIT;
          break;
        case KDEBUG_ERROR:
        default:
          /* Programmer error, use "Error" as default */
          nOutput = getNum( "ErrorOutput", 2 );
          aCaption = "Error (" + aAppName + ")";
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
                        aOutputFileName = getString( "InfoFilename", "kdebug.dbg" );
                        break;
                  case KDEBUG_WARN:
                        aOutputFileName = getString( "WarnFilename", "kdebug.dbg" );
                        break;
                  case KDEBUG_FATAL:
                        aOutputFileName = getString( "FatalFilename", "kdebug.dbg" );
                        break;
                  case KDEBUG_ERROR:
                  default:
                        aOutputFileName = getString( "ErrorFilename", "kdebug.dbg" );
                        break;
                  };
                char buf[4096];
                int nPrefix = sprintf( buf, "%s: ", aAppName.ascii() );
                va_start( arguments, pFormat );
                // use the more secure version if we have it
                unsigned int nSize = vsnprintf( buf, sizeof(buf)-1, pFormat, arguments );
		nSize = QMIN(nSize, sizeof(buf)-2-nPrefix);
                buf[nSize] = '\n';
                buf[nSize+1] = '\0';
                va_end( arguments );
                QFile aOutputFile( aOutputFileName );
                aOutputFile.open( IO_WriteOnly );
                aOutputFile.writeBlock( buf, nSize+1 );
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
                QMessageBox::warning( 0L, aCaption, buf, i18n("&OK") );
                break;
          }
        case 2: // Shell
          {
                va_start( arguments, pFormat );
                fprintf( stderr, "%s: ", aAppName.ascii() );
                vfprintf( stderr, pFormat, arguments );
                fprintf( stderr, "\n" );
                va_end( arguments );
                break;
          }
        case 3: // syslog
          {
                char buf[4096];
                int nPrefix = sprintf( buf, "%s: ", aAppName.ascii() );
                va_start( arguments, pFormat );
                int nSize = vsprintf( &buf[nPrefix], pFormat, arguments );
                if( nSize > (4094-nPrefix) ) nSize = 4094-nPrefix;
                buf[nSize] = '\n';
                buf[nSize+1] = '\0';
                va_end( arguments );
                syslog( nPriority, buf );
          }       
        case 4: // nothing
          {
          }
        }

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL ) &&
          ( getNum( "AbortFatal", 0 ) ) )
        abort();

  // restore old group
  if (kapp)
    KGlobal::config()->setGroup( aOldGroup );
}




KDebugDialog::KDebugDialog( QWidget *parent, const char *name, bool modal )
  : QDialog( parent, name, modal ),
    mMarginHint(6), mSpacingHint(6)
{
  setCaption(kapp->makeStdCaption(i18n("Debug Settings")));
 
  QVBoxLayout *topLayout = new QVBoxLayout( this, mMarginHint, mSpacingHint );
  if( topLayout == 0 ) { return; }
  
  QGridLayout *gbox = new QGridLayout( 2, 2, mSpacingHint );
  if( gbox == 0 ) { return; }
  topLayout->addLayout( gbox );

  QStringList destList;
  destList.append( i18n("File") );
  destList.append( i18n("Message Box") );
  destList.append( i18n("Shell") );
  destList.append( i18n("Syslog") );
  destList.append( i18n("None") );

  //
  // Upper left frame
  //
  pInfoGroup = new QGroupBox( i18n("Information"), this );
  gbox->addWidget( pInfoGroup, 0, 0 );
  QVBoxLayout *vbox = new QVBoxLayout( pInfoGroup, mSpacingHint );
  vbox->addSpacing( fontMetrics().lineSpacing() );
  pInfoLabel1 = new QLabel( i18n("Output to:"), pInfoGroup );
  vbox->addWidget( pInfoLabel1 );
  pInfoCombo = new QComboBox( false, pInfoGroup );
  vbox->addWidget( pInfoCombo );
  pInfoCombo->insertStringList( destList );
  pInfoLabel2 = new QLabel( i18n("Filename:"), pInfoGroup );
  vbox->addWidget( pInfoLabel2 );
  pInfoFile = new QLineEdit( pInfoGroup );
  vbox->addWidget( pInfoFile );
  pInfoLabel3 = new QLabel( i18n("Show only area(s):"), pInfoGroup );
  vbox->addWidget( pInfoLabel3 );
  pInfoShow = new QLineEdit( pInfoGroup );
  vbox->addWidget( pInfoShow );

  //
  // Upper right frame
  //
  pWarnGroup = new QGroupBox( i18n("Warning"), this );
  gbox->addWidget( pWarnGroup, 0, 1 );
  vbox = new QVBoxLayout( pWarnGroup, mSpacingHint );
  vbox->addSpacing( fontMetrics().lineSpacing() );
  pWarnLabel1 = new QLabel( i18n("Output to:"), pWarnGroup );
  vbox->addWidget( pWarnLabel1 );
  pWarnCombo = new QComboBox( false, pWarnGroup );
  vbox->addWidget( pWarnCombo );
  pWarnCombo->insertStringList( destList );
  pWarnLabel2 = new QLabel( i18n("Filename:"), pWarnGroup );
  vbox->addWidget( pWarnLabel2 );
  pWarnFile = new QLineEdit( pWarnGroup );
  vbox->addWidget( pWarnFile );
  pWarnLabel3 = new QLabel( i18n("Show only area(s):"), pWarnGroup );
  vbox->addWidget( pWarnLabel3 );
  pWarnShow = new QLineEdit( pWarnGroup );
  vbox->addWidget( pWarnShow );

  //
  // Lower left frame
  //
  pErrorGroup = new QGroupBox( i18n("Error"), this );
  gbox->addWidget( pErrorGroup, 1, 0 );
  vbox = new QVBoxLayout( pErrorGroup, mSpacingHint );
  vbox->addSpacing( fontMetrics().lineSpacing() );
  pErrorLabel1 = new QLabel( i18n("Output to:"), pErrorGroup );
  vbox->addWidget( pErrorLabel1 );
  pErrorCombo = new QComboBox( false, pErrorGroup );
  vbox->addWidget( pErrorCombo );
  pErrorCombo->insertStringList( destList );
  pErrorLabel2 = new QLabel( i18n("Filename:"), pErrorGroup );
  vbox->addWidget( pErrorLabel2 );
  pErrorFile = new QLineEdit( pErrorGroup );
  vbox->addWidget( pErrorFile );
  pErrorLabel3 = new QLabel( i18n("Show only area(s):"), pErrorGroup );
  vbox->addWidget( pErrorLabel3 );
  pErrorShow = new QLineEdit( pErrorGroup );
  vbox->addWidget( pErrorShow );

  //
  // Lower right frame
  //
  pFatalGroup = new QGroupBox( i18n("Fatal error"), this );
  gbox->addWidget( pFatalGroup, 1, 1 );
  vbox = new QVBoxLayout( pFatalGroup, mSpacingHint );
  vbox->addSpacing( fontMetrics().lineSpacing() );
  pFatalLabel1 = new QLabel( i18n("Output to:"), pFatalGroup );
  vbox->addWidget( pFatalLabel1 );
  pFatalCombo = new QComboBox( false, pFatalGroup );
  vbox->addWidget( pFatalCombo );
  pFatalCombo->insertStringList( destList );
  pFatalLabel2 = new QLabel( i18n("Filename:"), pFatalGroup );
  vbox->addWidget( pFatalLabel2 );
  pFatalFile = new QLineEdit( pFatalGroup );
  vbox->addWidget( pFatalFile );
  pFatalLabel3 = new QLabel( i18n("Show only area(s):"), pFatalGroup );
  vbox->addWidget( pFatalLabel3 );
  pFatalShow = new QLineEdit( pFatalGroup );
  vbox->addWidget( pFatalShow );


  pAbortFatal = new QCheckBox( i18n("Abort on fatal errors"), this );
  topLayout->addWidget(pAbortFatal);

  QFrame *hline = new QFrame( this );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  QHBoxLayout *hbox = new QHBoxLayout( mSpacingHint );
  topLayout->addLayout( hbox );
  pHelpButton = new QPushButton( i18n("&Help"), this );
  hbox->addWidget( pHelpButton );
  hbox->addStretch(10);
  pOKButton = new QPushButton( i18n("&OK"), this );
  hbox->addWidget( pOKButton );
  pCancelButton = new QPushButton( i18n("&Cancel"), this );
  hbox->addWidget( pCancelButton );

  int w1 = pHelpButton->sizeHint().width();
  int w2 = pOKButton->sizeHint().width();
  int w3 = pCancelButton->sizeHint().width();
  int w4 = QMAX( w1, QMAX( w2, w3 ) );

  pHelpButton->setFixedWidth( w4 );
  pOKButton->setFixedWidth( w4 );
  pCancelButton->setFixedWidth( w4 );

  connect( pHelpButton, SIGNAL( clicked() ), SLOT( showHelp() ) );
  connect( pOKButton, SIGNAL( clicked() ), SLOT( accept() ) );
  connect( pCancelButton, SIGNAL( clicked() ), SLOT( reject() ) );
}


KDebugDialog::~KDebugDialog()
{
}



#if 0
KDebugDialog::KDebugDialog() :
  QDialog( 0L, i18n("Debug Settings"), true )
{
  pInfoGroup = new QGroupBox( i18n("Information"), this );
  pInfoCombo = new QComboBox( false, this );
  pInfoGroup->setGeometry( 5, 10, 140, 185 );
  pInfoLabel1 = new QLabel( i18n("Output to:"), this );
  pInfoLabel1->setGeometry( 15, 30, 120, 15 );
  pInfoCombo->setGeometry( 15, 50, 120, 20 );
  pInfoCombo->insertItem( i18n("File") );
  pInfoCombo->insertItem( i18n("Message Box") );
  pInfoCombo->insertItem( i18n("Shell") );
  pInfoCombo->insertItem( i18n("Syslog") );
  pInfoCombo->insertItem( i18n("None") );
  pInfoLabel2 = new QLabel( i18n("Filename:"), this );
  pInfoLabel2->setGeometry( 15, 85, 120, 15 );
  pInfoFile = new QLineEdit( this );
  pInfoFile->setGeometry( 15, 105, 120, 20 );
  pInfoLabel3 = new QLabel( i18n("Show only area(s):"), this );
  pInfoLabel3->setGeometry( 15, 140, 120, 15 );
  pInfoShow = new QLineEdit( this );
  pInfoShow->setGeometry( 15, 160, 120, 20 );

  pWarnGroup = new QGroupBox( i18n("Warning"), this );
  pWarnGroup->setGeometry( 165, 10, 140, 185 );
  pWarnLabel1 = new QLabel( i18n("Output to:"), this );
  pWarnLabel1->setGeometry( 175, 30, 120, 15 );
  pWarnCombo = new QComboBox( false, this );
  pWarnCombo->setGeometry( 175, 50, 120, 20 );
  pWarnCombo->insertItem( i18n("File") );
  pWarnCombo->insertItem( i18n("Message Box") );
  pWarnCombo->insertItem( i18n("Shell") );
  pWarnCombo->insertItem( i18n("Syslog") );
  pWarnCombo->insertItem( i18n("None") );
  pWarnLabel2 = new QLabel( i18n("Filename:"), this );
  pWarnLabel2->setGeometry( 175, 85, 120, 15 );
  pWarnFile = new QLineEdit( this );
  pWarnFile->setGeometry( 175, 105, 120, 20 );
  pWarnLabel3 = new QLabel( i18n("Show only area(s):"), this );
  pWarnLabel3->setGeometry( 175, 140, 120, 15 );
  pWarnShow = new QLineEdit( this );
  pWarnShow->setGeometry( 175, 160, 120, 20 );

  pErrorGroup = new QGroupBox( i18n("Error"), this );
  pErrorGroup->setGeometry( 5, 215, 140, 185 );
  pErrorLabel1 = new QLabel( i18n("Output to:"), this );
  pErrorLabel1->setGeometry( 15, 235, 120, 15 );
  pErrorCombo = new QComboBox( false, this );
  pErrorCombo->setGeometry( 15, 255, 120, 20 );
  pErrorCombo->insertItem( i18n("File") );
  pErrorCombo->insertItem( i18n("Message Box") );
  pErrorCombo->insertItem( i18n("Shell") );
  pErrorCombo->insertItem( i18n("Syslog") );
  pErrorCombo->insertItem( i18n("None") );
  pErrorLabel2 = new QLabel( i18n("Filename:"), this );
  pErrorLabel2->setGeometry( 15, 290, 120, 15 );
  pErrorFile = new QLineEdit( this );
  pErrorFile->setGeometry( 15, 310, 120, 20 );
  pErrorLabel3 = new QLabel( i18n("Show only area(s):"), this );
  pErrorLabel3->setGeometry( 15, 345, 120, 15 );
  pErrorShow = new QLineEdit( this );
  pErrorShow->setGeometry( 15, 365, 120, 20 );

  pFatalGroup = new QGroupBox( i18n("Fatal error"), this );
  pFatalGroup->setGeometry( 165, 215, 140, 185 );
  pFatalLabel1 = new QLabel( i18n("Output to:"), this );
  pFatalLabel1->setGeometry( 175, 235, 120, 15 );
  pFatalCombo = new QComboBox( false, this );
  pFatalCombo->setGeometry( 175, 255, 120, 20 );
  pFatalCombo->insertItem( i18n("File") );
  pFatalCombo->insertItem( i18n("Message Box") );
  pFatalCombo->insertItem( i18n("Shell") );
  pFatalCombo->insertItem( i18n("Syslog") );
  pFatalCombo->insertItem( i18n("None") );
  pFatalLabel2 = new QLabel( i18n("Filename:"), this );
  pFatalLabel2->setGeometry( 175, 290, 120, 15 );
  pFatalFile = new QLineEdit( this );
  pFatalFile->setGeometry( 175, 310, 100, 20 );
  pFatalLabel3 = new QLabel( i18n("Show only area(s):"), this );
  pFatalLabel3->setGeometry( 175, 345, 120, 15 );
  pFatalShow = new QLineEdit( this );
  pFatalShow->setGeometry( 175, 365, 120, 20 );

  pAbortFatal = new QCheckBox( i18n("Abort on fatal errors"), this );
  pAbortFatal->setGeometry( 15, 420, 200, 15 );
  pOKButton = new QPushButton( i18n("&OK"), this );
  pOKButton->setGeometry( 15, 460, 80, 20 );
  pOKButton->setDefault( true );
  connect( pOKButton, SIGNAL( clicked() ), SLOT( accept() ) );
  pCancelButton = new QPushButton( i18n("&Cancel"), this );
  pCancelButton->setGeometry( 110, 460, 80, 20 );
  connect( pCancelButton, SIGNAL( clicked() ), SLOT( reject() ) );
  pHelpButton = new QPushButton( i18n("&Help"), this );
  pHelpButton->setGeometry( 205, 460, 80, 20 );
  connect( pHelpButton, SIGNAL( clicked() ), SLOT( showHelp() ) );

  pInfoGroup->show();
  pInfoLabel1->show();
  pInfoCombo->show();
  pInfoLabel2->show();
  pInfoFile->show();
  pInfoLabel3->show();
  pInfoShow->show();
  pWarnGroup->show();
  pWarnLabel1->show();
  pWarnCombo->show();
  pWarnLabel2->show();
  pWarnFile->show();
  pWarnLabel3->show();
  pWarnShow->show();
  pErrorGroup->show();
  pErrorLabel1->show();
  pErrorCombo->show();
  pErrorLabel2->show();
  pErrorFile->show();
  pErrorLabel3->show();
  pErrorShow->show();
  pFatalGroup->show();
  pFatalLabel1->show();
  pFatalCombo->show();
  pFatalLabel2->show();
  pFatalFile->show();
  pFatalLabel3->show();
  pFatalShow->show();
  pAbortFatal->show();
  pOKButton->show();
  pCancelButton->show();
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
#endif


void KDebugDialog::showHelp()
{
  if (kapp)
    kapp->invokeHTMLHelp( "kdelibs/kdebug.html", "" );
}

#include "kdebugdialog.moc"


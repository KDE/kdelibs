/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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


#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>

#include <kapp.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kurllabel.h>

#include "kbugreport.h"

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

KBugReport::KBugReport( QWidget * parentw, bool modal, const KAboutData *aboutData )
  : KDialogBase( Plain,
                 i18n("Submit a bug report"),
                 Ok | Cancel,
                 Ok,
                 parentw,
                 "KBugReport",
                 modal, // modal
                 true // separator
                 )
{
  // Use supplied aboutdata, otherwise the one from the active instance
  // otherwise the KGlobal one. _activeInstance should neved be 0L in theory.
  m_aboutData = aboutData
    ? aboutData
    : ( KGlobal::_activeInstance ? KGlobal::_activeInstance->aboutData()
                                 : KGlobal::instance()->aboutData() );
  m_process = 0;
  QWidget * parent = plainPage();
  QLabel * tmpLabel;
  QVBoxLayout * lay = new QVBoxLayout( parent, 0, spacingHint() );

  QGridLayout *glay = new QGridLayout( lay, 3, 3 );
  glay->setColStretch( 1, 10 );
  glay->setColStretch( 2, 10 );


  // From
  tmpLabel = new QLabel( i18n("From :"), parent );
  glay->addWidget( tmpLabel, 0,0 );
  m_from = new QLabel( parent );
  glay->addWidget( m_from, 0, 1 );

  // Program name
  tmpLabel = new QLabel( i18n("Application : "), parent );
  glay->addWidget( tmpLabel, 1, 0 );
  tmpLabel = new QLabel( QString::fromLatin1( m_aboutData
					      ? m_aboutData->appName()
					      : kapp->name() ),
			 parent );
  glay->addWidget( tmpLabel, 1, 1 );

  // Version
  tmpLabel = new QLabel( i18n("Version : "), parent );
  glay->addWidget( tmpLabel, 2, 0 );
  if (m_aboutData) m_strVersion = m_aboutData->version();
   else m_strVersion = i18n("no version set (programmer error!)");
  m_strVersion += QString::fromLatin1(" (KDE " KDE_VERSION_STRING ")");
  m_version = new QLabel( m_strVersion, parent );
  //glay->addWidget( m_version, 2, 1 );
  glay->addMultiCellWidget( m_version, 2, 2, 1, 2 );

  // Configure email button
  m_configureEmail = new QPushButton( i18n("Configure E-Mail"),
						  parent );
  connect( m_configureEmail, SIGNAL( clicked() ), this,
	   SLOT( slotConfigureEmail() ) );
  glay->addMultiCellWidget( m_configureEmail, 0, 2, 2, 2, AlignTop|AlignRight );

  // Severity
  m_bgSeverity = new QHButtonGroup( i18n("Severity"), parent );
  const char * sevNames[5] = { "critical", "grave", "normal", "wishlist", "i18n" };
  const QString sevTexts[5] = { i18n("Critical"), i18n("Grave"), i18n("Normal"), i18n("Wishlist"), i18n("Translation") };

  for (int i = 0 ; i < 5 ; i++ )
  {
    // Store the severity string as the name
    QRadioButton *rb = new QRadioButton( sevTexts[i], m_bgSeverity, sevNames[i] );
    if (i==2) rb->setChecked(true); // default : "normal"
  }

  lay->addWidget( m_bgSeverity );

  // Subject
  QHBoxLayout * hlay = new QHBoxLayout( lay );
  tmpLabel = new QLabel( i18n("Subject : "), parent );
  hlay->addWidget( tmpLabel );
  m_subject = new QLineEdit( parent );
  m_subject->setFocus();
  hlay->addWidget( m_subject );

  QString text = i18n(""
    "Enter the text (in English if possible) that you wish to submit for the "
    "bug report.\n"
    "If you press \"OK\", a mail message will be sent to the maintainer of "
    "this program \n"
    "and to the KDE buglist.");
  QLabel * label = new QLabel( parent, "label" );

  /*
    2000-01-15 Espen
    Does not work (yet). The label has no well defined height so the
    dialog can be resized so that the action buttons become obscured

  QString text = i18n(""
    "Enter the text (in English if possible) that you wish to submit for the "
    "bug report. If you press \"OK\", a mail message will be sent to the "
    "maintainer of this program and to the KDE buglist.");
  QLabel * label = new QLabel( parent, "label" );
  label->setTextFormat( RichText );
  */

  label->setText( text );
  lay->addWidget( label );

  // The multiline-edit
  m_lineedit = new QMultiLineEdit( parent, "QMultiLineEdit" );
  m_lineedit->setMinimumHeight( 180 ); // make it big
  lay->addWidget( m_lineedit, 10 /*stretch*/ );


  hlay = new QHBoxLayout( lay, 0 );

  text = i18n("You can access previously reported bugs and wishes at ");
  label = new QLabel( text, parent, "label");
  hlay->addWidget( label, 0, AlignBottom );
  hlay->addSpacing(1); // Looks better :)

  text = "http://bugs.kde.org/";
  KURLLabel *url = new KURLLabel( parent );
  url->setText(text);
  url->setURL(text);
  connect( url, SIGNAL(leftClickedURL(const QString &)),
	   this, SLOT(slotUrlClicked(const QString &)));
  hlay->addWidget( url, 0, AlignBottom );

  hlay->addStretch( 10 );

  // Necessary for vertical label and url alignment.
  label->setFixedHeight( fontMetrics().lineSpacing() );
  url->setFixedHeight( fontMetrics().lineSpacing()-1 );

  slotSetFrom();
}

KBugReport::~KBugReport()
{
}

void KBugReport::slotConfigureEmail()
{
  if (m_process) return;
  m_process = new KProcess;
  *m_process << QString::fromLatin1("kcmshell") << QString::fromLatin1("Personalization/email");
  connect(m_process, SIGNAL(processExited(KProcess *)), this, SLOT(slotSetFrom()));
  if (!m_process->start())
  {
    qDebug("Couldn't start kcmshell..");
    delete m_process;
    m_process = 0;
    return;
  }
  m_configureEmail->setEnabled(false);
}

void KBugReport::slotSetFrom()
{
  delete m_process;
  m_process = 0;
  m_configureEmail->setEnabled(true);
  KConfig emailConf( QString::fromLatin1("emaildefaults") );
  emailConf.setGroup( QString::fromLatin1("UserInfo") );
  QString fromaddr = emailConf.readEntry( QString::fromLatin1("EmailAddress") );
  if (fromaddr.isEmpty()) {
     struct passwd *p;
     p = getpwuid(getuid());  
     fromaddr = QString::fromLatin1(p->pw_name);
  } else {
     QString name = emailConf.readEntry( QString::fromLatin1("FullName"));
     if (!name.isEmpty())
        fromaddr = name + QString::fromLatin1(" <") + fromaddr + QString::fromLatin1(">");
  }
  m_from->setText( fromaddr );
}

void KBugReport::slotUrlClicked(const QString &urlText)
{
  kapp->invokeBrowser( urlText );
}


void KBugReport::slotOk( void )
{
  if( m_lineedit->text().isEmpty() == true ||
      m_subject->text().isEmpty() == true )
  {
    QString msg = i18n(""
      "You must specify both a subject and a description\n"
      "before the report can be sent.");
    KMessageBox::error(this,msg);
    return;
  }

  if( !sendBugReport() )
  {
    QString msg = i18n(""
      "Couldn't send the bug report.\n"
      "Hmmm, submit a bug report manually, sorry...\n"
      "See http://bugs.kde.org/ for instructions.");
    KMessageBox::error(this, msg );
    return;
  }

   KMessageBox::information(this,
			    i18n("Bug report sent, thanks for your input."));
   accept();
}



QString KBugReport::text()
{
    debug(m_bgSeverity->selected()->name());
  // Prepend the pseudo-headers to the contents of the mail
  QString severity = QString::fromLatin1(m_bgSeverity->selected()->name());
  QString appname = QString::fromLatin1( m_aboutData
                                         ? m_aboutData->appName()
                                         : kapp->name());
  if (severity == QString::fromLatin1("i18n"))
    // Case 1 : i18n bug
      return QString::fromLatin1("Package: i18n_") + KGlobal::locale()->language() +
	QString::fromLatin1("\n"
			    "Application: %1\n"
			    // not really i18n's version, so better here IMHO
			    "Version: %2\n"
	  "\n").arg(appname).arg(m_strVersion)+
	  m_lineedit->text();
  else
    // Case 2 : normal bug
    return QString::fromLatin1("Package: %1\n"
      "Version: %2\n"
      "Severity: %3\n"
      "\n").arg(appname).arg(m_strVersion).arg(severity)+
      m_lineedit->text();
}

bool KBugReport::sendBugReport()
{
  QString command = KStandardDirs::findExe( QString::fromLatin1("sendmail"), QString::fromLatin1("/sbin:/usr/sbin:/usr/lib") );
  bool needHeaders = true;

  QString recipient ( m_aboutData ?
    m_aboutData->bugAddress() :
    QString::fromLatin1("submit@bugs.kde.org") );

  if ( command.isNull() )
  {
    command = QString::fromLatin1("mail -s \x22");
    command.append(m_subject->text());
    command.append(QString::fromLatin1("\x22 "));
    command.append(recipient);
    needHeaders = false;
  }
  else
  {
    command += QString::fromLatin1(" -oi -t");
  }
  FILE * fd = popen(command.local8Bit(),"w");
  if (!fd)
  {
    kdError() << "Unable to open a pipe to " << command.local8Bit().data() << endl;
    return false;
  }

  QString textComplete;
  if (needHeaders)
  {
    textComplete += QString::fromLatin1("From: ") + m_from->text() + '\n';
    textComplete += QString::fromLatin1("To: ") + recipient + '\n';
    textComplete += QString::fromLatin1("Subject: ") + m_subject->text() + '\n';
  }
  textComplete += '\n'; // end of headers
  textComplete += text();

  fwrite(textComplete.ascii(),textComplete.length(),1,fd);

  pclose(fd);

  return true;
}

#include "kbugreport.moc"

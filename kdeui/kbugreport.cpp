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
#include <qwhatsthis.h>

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
#include "kdepackages.h"
#include <kcombobox.h>
#include <config.h>
#include <ktempfile.h>
#include <qtextstream.h>
#include <qfile.h>

class KBugReportPrivate {
public:
    KComboBox *appcombo;
    QString lastError;
};

KBugReport::KBugReport( QWidget * parentw, bool modal, const KAboutData *aboutData )
  : KDialogBase( Plain,
                 i18n("Submit a Bug Report"),
                 Ok | Cancel,
                 Ok,
                 parentw,
                 "KBugReport",
                 modal, // modal
                 true // separator
                 )
{
    d = new KBugReportPrivate;

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

  setButtonOKText(i18n("&Send"),
                  i18n("Send bugreport."),
                  i18n("Send this bugreport to the KDE buglist."));

  // From
  QString qwtstr = i18n( "Your e-mail address. If incorrect, use the Configure E-Mail button to change it" );
  tmpLabel = new QLabel( i18n("From :"), parent );
  glay->addWidget( tmpLabel, 0,0 );
  QWhatsThis::add( tmpLabel, qwtstr );
  m_from = new QLabel( parent );
  glay->addWidget( m_from, 0, 1 );
  QWhatsThis::add( m_from, qwtstr );

  // Program name
  qwtstr = i18n( "The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application" );
  tmpLabel = new QLabel( i18n("Application : "), parent );
  glay->addWidget( tmpLabel, 1, 0 );
  QWhatsThis::add( tmpLabel, qwtstr );
  d->appcombo = new KComboBox( false, parent, "app");
  d->appcombo->insertStrList(packages);
  QString appname = QString::fromLatin1( m_aboutData
                                         ? m_aboutData->appName()
                                         : kapp->name() );
  glay->addWidget( d->appcombo, 1, 1 );
  int index = 0;
  for (; index < d->appcombo->count(); index++) {
      if (d->appcombo->text(index) == appname) {
          break;
      }
  }
  if (index == d->appcombo->count()) { // not present
      d->appcombo->insertItem(appname);
  }
  d->appcombo->setCurrentItem(index);

  QWhatsThis::add( tmpLabel, qwtstr );

  // Version
  qwtstr = i18n( "The version of this application - please make sure that no newer version is available before sending a bug report" );
  tmpLabel = new QLabel( i18n("Version : "), parent );
  glay->addWidget( tmpLabel, 2, 0 );
  QWhatsThis::add( tmpLabel, qwtstr );
  if (m_aboutData) m_strVersion = m_aboutData->version();
   else m_strVersion = i18n("no version set (programmer error!)");
  m_strVersion += QString::fromLatin1(" (KDE " KDE_VERSION_STRING ")");
  m_version = new QLabel( m_strVersion, parent );
  //glay->addWidget( m_version, 2, 1 );
  glay->addMultiCellWidget( m_version, 2, 2, 1, 2 );
  QWhatsThis::add( m_version, qwtstr );

  tmpLabel = new QLabel(i18n("OS:"), parent);
  glay->addWidget( tmpLabel, 3, 0 );
  tmpLabel = new QLabel(QString::fromLatin1("%1 (%2)").
                        arg(KDE_COMPILING_OS).
                        arg(KDE_DISTRIBUTION_TEXT), parent);
  glay->addMultiCellWidget( tmpLabel, 3, 3, 1, 2 );

  tmpLabel = new QLabel(i18n("Compiler:"), parent);
  glay->addWidget( tmpLabel, 4, 0 );
  tmpLabel = new QLabel(QString::fromLatin1(KDE_COMPILER_VERSION), parent);
  glay->addMultiCellWidget( tmpLabel, 4, 4, 1, 2 );

  // Configure email button
  m_configureEmail = new QPushButton( i18n("Configure E-Mail"),
						  parent );
  connect( m_configureEmail, SIGNAL( clicked() ), this,
	   SLOT( slotConfigureEmail() ) );
  glay->addMultiCellWidget( m_configureEmail, 0, 2, 2, 2, AlignTop|AlignRight );

  // Severity
  m_bgSeverity = new QHButtonGroup( i18n("Se&verity"), parent );
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
  tmpLabel = new QLabel( i18n("S&ubject : "), parent );
  hlay->addWidget( tmpLabel );
  m_subject = new QLineEdit( parent );
  m_subject->setFocus();
  tmpLabel->setBuddy(m_subject);
  hlay->addWidget( m_subject );

  QString text = i18n(""
    "Enter the text (in English if possible) that you wish to submit for the "
    "bug report.\n"
    "If you press \"Send\", a mail message will be sent to the maintainer of "
    "this program \n"
    "and to the KDE buglist.");
  QLabel * label = new QLabel( parent, "label" );

  /*
    2000-01-15 Espen
    Does not work (yet). The label has no well defined height so the
    dialog can be resized so that the action buttons become obscured

  QString text = i18n(""
    "Enter the text (in English if possible) that you wish to submit for the "
    "bug report. If you press \"Send\", a mail message will be sent to the "
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

  text = i18n("Please check that the bug you are about to report is not already listed at ");
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
  url->setFixedHeight( fontMetrics().lineSpacing());

  slotSetFrom();
}

KBugReport::~KBugReport()
{
    delete d;
}

void KBugReport::slotConfigureEmail()
{
  if (m_process) return;
  m_process = new KProcess;
  *m_process << QString::fromLatin1("kcmshell") << QString::fromLatin1("Personalization/email");
  connect(m_process, SIGNAL(processExited(KProcess *)), this, SLOT(slotSetFrom()));
  if (!m_process->start())
  {
    kdDebug() << "Couldn't start kcmshell.." << endl;
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
        KMessageBox::error(this, msg + "\n\n" + d->lastError);
        return;
    }

    KMessageBox::information(this,
                             i18n("Bug report sent, thanks for your input."));
    accept();
}



QString KBugReport::text()
{
    kdDebug() << m_bgSeverity->selected()->name() << endl;
    // Prepend the pseudo-headers to the contents of the mail
  QString severity = QString::fromLatin1(m_bgSeverity->selected()->name());
  QString appname = d->appcombo->currentText();
  QString os = QString::fromLatin1("OS: %1 (%2)\n").
               arg(KDE_COMPILING_OS).
               arg(KDE_DISTRIBUTION_TEXT);
  if (severity == QString::fromLatin1("i18n"))
      // Case 1 : i18n bug
      return QString::fromLatin1("Package: i18n_") + KGlobal::locale()->language() +
          QString::fromLatin1("\n"
                              "Application: %1\n"
                              // not really i18n's version, so better here IMHO
                              "Version: %2\n")+
          os+
	  QString::fromLatin1("\n").arg(appname).arg(m_strVersion)+
	  m_lineedit->text();
  else
      // Case 2 : normal bug
      return QString::fromLatin1("Package: %1\n"
                                 "Version: %2\n"
                                 "Severity: %3\n")
          .arg(appname).arg(m_strVersion).arg(severity)+
          QString::fromLatin1("Compiler: %1\n").arg(KDE_COMPILER_VERSION)+
          os+QString::fromLatin1("\n")+
          m_lineedit->text();
}

bool KBugReport::sendBugReport()
{
  QString recipient ( m_aboutData ?
    m_aboutData->bugAddress() :
    QString::fromLatin1("submit@bugs.kde.org") );

  QString command;
  command = locate("exe", "ksendbugmail");
  if (command.isEmpty())
      command = KStandardDirs::findExe( QString::fromLatin1("ksendbugmail") );

  KTempFile outputfile;
  outputfile.close();

  command += QString::fromLatin1(" --subject \"%1\" --recipient \"%2\" > %3").arg(m_subject->text()).arg(recipient).arg(outputfile.name());

  FILE * fd = popen(command.local8Bit(), "w");
  if (!fd)
  {
    kdError() << "Unable to open a pipe to " << command << endl;
    return false;
  }

  QString btext = text();
  fwrite(btext.ascii(),btext.length(),1,fd);

  int error = pclose(fd);
  if (error) {
      QFile of(outputfile.name());
      if (of.open(IO_ReadOnly )) {
          QTextStream is(&of);
          is.setEncoding(QTextStream::UnicodeUTF8);
          QString line;
          while (!is.eof())
              line = is.readLine();
          d->lastError = line;
      } else {
          d->lastError = QString::null;
      }
      outputfile.unlink();
      return false;
  }
  outputfile.unlink();
  return true;
}

#include "kbugreport.moc"

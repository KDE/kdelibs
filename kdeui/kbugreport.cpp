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

#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kurllabel.h>

#include "kbugreport.h"

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <sys/utsname.h>

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
    QString kde_version;
    QString appname;
    QString os;
    KURLLabel *webFormLabel;
};

KBugReport::KBugReport( QWidget * parentw, bool modal, const KAboutData *aboutData )
  : KDialogBase( Plain,
                 i18n("Submit Bug Report"),
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
  d->webFormLabel = 0;

  if ( m_aboutData->bugAddress() == QString::fromLatin1("submit@bugs.kde.org") )
  {
    // This is a core KDE application -> redirect to the web form
    d->webFormLabel = new KURLLabel( parent );
    setButtonCancelText(i18n("&Close"));
  }

  QLabel * tmpLabel;
  QVBoxLayout * lay = new QVBoxLayout( parent, 0, spacingHint() );

  QGridLayout *glay = new QGridLayout( lay, 4, 3 );
  glay->setColStretch( 1, 10 );
  glay->setColStretch( 2, 10 );

  int row = 0;

  if ( !d->webFormLabel )
  {
    // From
    QString qwtstr = i18n( "Your email address. If incorrect, use the Configure Email button to change it" );
    tmpLabel = new QLabel( i18n("From:"), parent );
    glay->addWidget( tmpLabel, row,0 );
    QWhatsThis::add( tmpLabel, qwtstr );
    m_from = new QLabel( parent );
    glay->addWidget( m_from, row, 1 );
    QWhatsThis::add( m_from, qwtstr );


    // Configure email button
    m_configureEmail = new QPushButton( i18n("Configure Email..."),
                                        parent );
    connect( m_configureEmail, SIGNAL( clicked() ), this,
             SLOT( slotConfigureEmail() ) );
    glay->addMultiCellWidget( m_configureEmail, 0, 2, 2, 2, AlignTop|AlignRight );

    // To
    qwtstr = i18n( "The email address this bug report is sent to." );
    tmpLabel = new QLabel( i18n("To:"), parent );
    glay->addWidget( tmpLabel, ++row,0 );
    QWhatsThis::add( tmpLabel, qwtstr );
    tmpLabel = new QLabel( m_aboutData->bugAddress(), parent );
    glay->addWidget( tmpLabel, row, 1 );
    QWhatsThis::add( tmpLabel, qwtstr );

    setButtonOKText(i18n("&Send"),
                    i18n("Send bug report."),
                    i18n("Send this bug report to the KDE buglist."));

  }
  else
  {
    m_configureEmail = 0;
    m_from = 0;
    showButtonOK( false );
  }

  // Program name
  QString qwtstr = i18n( "The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application" );
  tmpLabel = new QLabel( i18n("Application: "), parent );
  glay->addWidget( tmpLabel, ++row, 0 );
  QWhatsThis::add( tmpLabel, qwtstr );
  d->appcombo = new KComboBox( false, parent, "app");
  QWhatsThis::add( d->appcombo, qwtstr );
  d->appcombo->insertStrList((const char**)packages);
  connect(d->appcombo, SIGNAL(activated(int)), SLOT(appChanged(int)));
  d->appname = QString::fromLatin1( m_aboutData
                                    ? m_aboutData->appName()
                                    : qApp->name() );
  glay->addWidget( d->appcombo, row, 1 );
  int index = 0;
  for (; index < d->appcombo->count(); index++) {
      if (d->appcombo->text(index) == d->appname) {
          break;
      }
  }
  if (index == d->appcombo->count()) { // not present
      d->appcombo->insertItem(d->appname);
  }
  d->appcombo->setCurrentItem(index);

  QWhatsThis::add( tmpLabel, qwtstr );

  // Version
  qwtstr = i18n( "The version of this application - please make sure that no newer version is available before sending a bug report" );
  tmpLabel = new QLabel( i18n("Version:"), parent );
  glay->addWidget( tmpLabel, ++row, 0 );
  QWhatsThis::add( tmpLabel, qwtstr );
  if (m_aboutData)
      m_strVersion = m_aboutData->version();
  else
      m_strVersion = i18n("no version set (programmer error!)");
  d->kde_version = QString::fromLatin1(" (KDE " KDE_VERSION_STRING ") ("
          KDE_DISTRIBUTION_TEXT ")");
  m_strVersion += d->kde_version;
  m_version = new QLabel( m_strVersion, parent );
  //glay->addWidget( m_version, row, 1 );
  glay->addMultiCellWidget( m_version, row, row, 1, 2 );
  QWhatsThis::add( m_version, qwtstr );

  tmpLabel = new QLabel(i18n("OS:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );

  struct utsname unameBuf;
  uname( &unameBuf );
  d->os = QString::fromLatin1( unameBuf.sysname ) +
					" (" + QString::fromLatin1( unameBuf.machine ) + ") "
          "release " + QString::fromLatin1( unameBuf.release );

  tmpLabel = new QLabel(d->os, parent);
  glay->addMultiCellWidget( tmpLabel, row, row, 1, 2 );

  tmpLabel = new QLabel(i18n("Compiler:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );
  tmpLabel = new QLabel(QString::fromLatin1(KDE_COMPILER_VERSION), parent);
  glay->addMultiCellWidget( tmpLabel, row, row, 1, 2 );

  if ( !d->webFormLabel )
  {
    // Severity
    m_bgSeverity = new QHButtonGroup( i18n("Se&verity"), parent );
    static const char * const sevNames[5] = { "critical", "grave", "normal", "wishlist", "i18n" };
    const QString sevTexts[5] = { i18n("Critical"), i18n("Grave"), i18n("normal severity","Normal"), i18n("Wishlist"), i18n("Translation") };

    for (int i = 0 ; i < 5 ; i++ )
    {
      // Store the severity string as the name
      QRadioButton *rb = new QRadioButton( sevTexts[i], m_bgSeverity, sevNames[i] );
      if (i==2) rb->setChecked(true); // default : "normal"
    }

    lay->addWidget( m_bgSeverity );

    // Subject
    QHBoxLayout * hlay = new QHBoxLayout( lay );
    tmpLabel = new QLabel( i18n("S&ubject: "), parent );
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
    m_lineedit->setWordWrap(QMultiLineEdit::WidgetWidth);
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
  } else {
    // Point to the web form

    lay->addSpacing(20);
    QString text = i18n("To submit a bug report, click on the link below.\n"
                        "This will open a web browser window on http://bugs.kde.org where you'll find a form to fill in.\n"
                        "The information displayed above will be transferred to that server.");
    QLabel * label = new QLabel( text, parent, "label");
    lay->addWidget( label );
    lay->addSpacing(20);

    updateURL();
    d->webFormLabel->setText( "http://bugs.kde.org/wizard/index.php" );
    lay->addWidget( d->webFormLabel );
    lay->addSpacing(20);

    connect( d->webFormLabel, SIGNAL(leftClickedURL(const QString &)),
             this, SLOT(slotUrlClicked(const QString &)));
  }
}

KBugReport::~KBugReport()
{
    delete d;
}

void KBugReport::updateURL()
{
    QString url = QString::fromLatin1("http://bugs.kde.org/wizard/index.php");
    url += "?os=";
    url += KURL::encode_string( d->os );
    url += "&compiler=";
    url += KURL::encode_string( QString::fromLatin1(KDE_COMPILER_VERSION) );
    url += "&version=";
    url += KURL::encode_string( m_strVersion );
    url += "&package=";
    url += KURL::encode_string( d->appcombo->currentText() );
    url += "&kbugreport=1";
    d->webFormLabel->setURL( url );
}

void KBugReport::appChanged(int i)
{
    if (d->appname == d->appcombo->text(i) && m_aboutData)
        m_strVersion = m_aboutData->version();
    else
        m_strVersion = i18n("unknown program name", "unknown");

    m_strVersion += d->kde_version;
    m_version->setText(m_strVersion);
    if ( d->webFormLabel )
        updateURL();
}

void KBugReport::slotConfigureEmail()
{
  if (m_process) return;
  m_process = new KProcess;
  *m_process << QString::fromLatin1("kcmshell") << QString::fromLatin1("email");
  connect(m_process, SIGNAL(processExited(KProcess *)), SLOT(slotSetFrom()));
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

  // ### KDE3: why oh why is KEmailSettings in kio?
  KConfig emailConf( QString::fromLatin1("emaildefaults") );

  // find out the default profile
  emailConf.setGroup( QString::fromLatin1("Defaults") );
  QString profile = QString::fromLatin1("PROFILE_");
  profile += emailConf.readEntry( QString::fromLatin1("Profile"),
                                  QString::fromLatin1("Default") );

  emailConf.setGroup( profile );
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
  if ( kapp )
    kapp->invokeBrowser( urlText );

  // When using the web form, a click can also close the window, as there's
  // not much to do. It also gives the user a direct response to his click:
  if ( d->webFormLabel )
      KDialogBase::slotCancel();
}


void KBugReport::slotOk( void )
{
    if ( d->webFormLabel)
        return;

    if( m_lineedit->text().isEmpty() == true ||
        m_subject->text().isEmpty() == true )
    {
        QString msg = i18n("You must specify both a subject and a description "
                           "before the report can be sent.");
        KMessageBox::error(this,msg);
        return;
    }

    switch ( m_bgSeverity->id( m_bgSeverity->selected() ) )
    {
        case 0: // critical
            if ( KMessageBox::questionYesNo( this, i18n(
                "<p>You chose the severity <b>Critical</b>. "
                "Please note that this severity is intended only for bugs that</p>"
                "<ul><li>break unrelated software on the system (or the whole system)</li>"
                "<li>cause serious data loss</li>"
                "<li>introduce a security hole on the system where the affected package is installed</li></ul>\n"
                "<p>Does the bug you are reporting cause any of the above damage? "
                "If it does not, please select a lower severity. Thank you!</p>" ) ) == KMessageBox::No )
                return;
            break;
        case 1: // grave
            if ( KMessageBox::questionYesNo( this, i18n(
                "<p>You chose the severity <b>Grave</b>. "
                "Please note that this severity is intended only for bugs that</p>"
                "<ul><li>make the package in question unusable or mostly so</li>"
                "<li>cause data loss</li>"
                "<li>introduce a security hole allowing access to the accounts of users who use the affected package</li></ul>\n"
                "<p>Does the bug you are reporting cause any of the above damage? "
                "If it does not, please select a lower severity. Thank you!</p>" ) ) == KMessageBox::No )
                return;
            break;
    }
    if( !sendBugReport() )
    {
        QString msg = i18n(""
                           "Unable to send the bug report.\n"
                           "Please submit a bug report manually...\n"
                           "See http://bugs.kde.org/ for instructions.");
        KMessageBox::error(this, msg + "\n\n" + d->lastError);
        return;
    }

    KMessageBox::information(this,
                             i18n("Bug report sent, thank you for your input."));
    accept();
}

void KBugReport::slotCancel()
{
  if( !d->webFormLabel && ( m_lineedit->edited() || m_subject->edited() ) )
  {
    int rc = KMessageBox::warningYesNo( this,
             i18n( "Close and discard\nedited message?" ),
             i18n( "Close message" ), i18n( "&Discard" ), i18n("&Continue") );
    if( rc == KMessageBox::No )
      return;
  }
  KDialogBase::slotCancel();
}


QString KBugReport::text() const
{
    kdDebug() << m_bgSeverity->selected()->name() << endl;
    // Prepend the pseudo-headers to the contents of the mail
  QString severity = QString::fromLatin1(m_bgSeverity->selected()->name());
  QString appname = d->appcombo->currentText();
  QString os = QString::fromLatin1("OS: %1 (%2)\n").
               arg(KDE_COMPILING_OS).
               arg(KDE_DISTRIBUTION_TEXT);
  QString bodyText;
  for(int i = 0; i < m_lineedit->numLines(); i++)
  {
     QString line = m_lineedit->textLine(i);
     if (line.right(1) != "\n")
        line += '\n';
     bodyText += line;
  }

  if (severity == QString::fromLatin1("i18n") && KGlobal::locale()->language() != KLocale::defaultLanguage()) {
      // Case 1 : i18n bug
      QString package = QString::fromLatin1("i18n_%1").arg(KGlobal::locale()->language());
      package = package.replace(QRegExp("_"), QString::fromLatin1("-"));
      return QString::fromLatin1("Package: %1").arg(package) +
          QString::fromLatin1("\n"
                              "Application: %1\n"
                              // not really i18n's version, so better here IMHO
                              "Version: %2\n").arg(appname).arg(m_strVersion)+
          os+QString::fromLatin1("\n")+bodyText;
  } else {
      appname = appname.replace(QRegExp("_"), QString::fromLatin1("-"));
      // Case 2 : normal bug
      return QString::fromLatin1("Package: %1\n"
                                 "Version: %2\n"
                                 "Severity: %3\n")
          .arg(appname).arg(m_strVersion).arg(severity)+
          QString::fromLatin1("Compiler: %1\n").arg(KDE_COMPILER_VERSION)+
          os+QString::fromLatin1("\n")+bodyText;
  }
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

  QString subject = m_subject->text();
  command += " --subject ";
  command += KProcess::quote(subject);
  command += " --recipient ";
  command += KProcess::quote(recipient);
  command += " > ";
  command += KProcess::quote(outputfile.name());

  fflush(stdin);
  fflush(stderr);

  FILE * fd = popen(QFile::encodeName(command), "w");
  if (!fd)
  {
    kdError() << "Unable to open a pipe to " << command << endl;
    return false;
  }

  QString btext = text();
  fwrite(btext.ascii(),btext.length(),1,fd);
  fflush(fd);

  int error = pclose(fd);
  kdDebug() << "exit status1 " << error << " " << (WIFEXITED(error)) << " " <<  WEXITSTATUS(error) << endl;

  if ((WIFEXITED(error)) && WEXITSTATUS(error) == 1) {
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

void KBugReport::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kbugreport.moc"

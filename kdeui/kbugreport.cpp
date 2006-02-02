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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <QGroupBox>
#include <QHBoxLayout>

#include <kaboutdata.h>
#include "ktoolinvocation.h"
#include <qapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <kinstance.h>
#include <kurl.h>
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
    QPushButton *submitBugButton;
    KUrl url;
    QList<QRadioButton*> severityButtons;
    int currentSeverity() {
    	for (int i=0;i<severityButtons.count();i++)
		if (severityButtons[i]->isChecked()) return i;
	return -1;
    }
};

KBugReport::KBugReport( QWidget * parentw, bool modal, const KAboutData *aboutData )
  : KDialog( parentw,
                 i18n("Submit Bug Report"),
                 Ok | Cancel
                 )
{
  d = new KBugReportPrivate;
  enableButtonSeparator(true);
  setModal(modal);

  // Use supplied aboutdata, otherwise the one from the active instance
  // otherwise the KGlobal one. _activeInstance should neved be 0L in theory.
  m_aboutData = aboutData
    ? aboutData
    : ( KGlobal::_activeInstance ? KGlobal::_activeInstance->aboutData()
                                 : KGlobal::instance()->aboutData() );
  m_process = 0;
  QWidget * parent = new QWidget(this);
  d->submitBugButton = 0;

  if ( m_aboutData->bugAddress() == QString::fromLatin1("submit@bugs.kde.org") )
  {
    // This is a core KDE application -> redirect to the web form
    d->submitBugButton = new QPushButton( parent );
    setButtonGuiItem( Cancel, KStdGuiItem::close() );
  }

  QLabel * tmpLabel;
  QVBoxLayout * lay = new QVBoxLayout( parent);
  lay->setMargin(0);
  lay->setSpacing( spacingHint() );

  QGridLayout *glay = new QGridLayout();
  lay->addItem(glay);
  glay->addItem(new QSpacerItem(10,0),0,1);
  glay->addItem(new QSpacerItem(10,0),0,2);

  int row = 0;

  if ( !d->submitBugButton )
  {
    // From
    QString qwtstr = i18n( "Your email address. If incorrect, use the Configure Email button to change it" );
    tmpLabel = new QLabel( i18n("From:"), parent );
    glay->addWidget( tmpLabel, row,0 );
    tmpLabel->setWhatsThis(qwtstr );
    m_from = new QLabel( parent );
    glay->addWidget( m_from, row, 1 );
    m_from->setWhatsThis(qwtstr );


    // Configure email button
    m_configureEmail = new QPushButton( i18n("Configure Email..."),
                                        parent );
    connect( m_configureEmail, SIGNAL( clicked() ), this,
             SLOT( slotConfigureEmail() ) );
    glay->addWidget( m_configureEmail, 0, 2, 3, 1, Qt::AlignTop|Qt::AlignRight );

    // To
    qwtstr = i18n( "The email address this bug report is sent to." );
    tmpLabel = new QLabel( i18n("To:"), parent );
    glay->addWidget( tmpLabel, ++row,0 );
    tmpLabel->setWhatsThis(qwtstr );
    tmpLabel = new QLabel( m_aboutData->bugAddress(), parent );
    glay->addWidget( tmpLabel, row, 1 );
    tmpLabel->setWhatsThis(qwtstr );

    setButtonGuiItem( Ok,  KGuiItem( i18n("&Send"), "mail_send", i18n( "Send bug report." ),
                    i18n( "Send this bug report to %1." ).arg( m_aboutData->bugAddress() ) ) );

  }
  else
  {
    m_configureEmail = 0;
    m_from = 0;
    showButton(Ok, false );
  }

  // Program name
  QString qwtstr = i18n( "The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application" );
  tmpLabel = new QLabel( i18n("Application: "), parent );
  glay->addWidget( tmpLabel, ++row, 0 );
  tmpLabel->setWhatsThis(qwtstr );
  d->appcombo = new KComboBox( false, parent );
  d->appcombo->setWhatsThis(qwtstr );
  QStringList packageList;
  for (int c = 0; packages[c]; ++c)
    packageList << QString::fromLatin1(packages[c]);
  d->appcombo->addItems(packageList);
  connect(d->appcombo, SIGNAL(activated(int)), SLOT(appChanged(int)));
  d->appname = m_aboutData
                                    ? QString::fromLatin1(m_aboutData->productName())
                                    : qApp->applicationName() ;
  glay->addWidget( d->appcombo, row, 1 );
  int index = 0;
  for (; index < d->appcombo->count(); index++) {
      if (d->appcombo->itemText(index) == d->appname) {
          break;
      }
  }
  if (index == d->appcombo->count()) { // not present
      d->appcombo->addItem(d->appname);
  }
  d->appcombo->setCurrentIndex(index);

  tmpLabel->setWhatsThis(qwtstr );

  // Version
  qwtstr = i18n( "The version of this application - please make sure that no newer version is available before sending a bug report" );
  tmpLabel = new QLabel( i18n("Version:"), parent );
  glay->addWidget( tmpLabel, ++row, 0 );
  tmpLabel->setWhatsThis(qwtstr );
  if (m_aboutData)
      m_strVersion = m_aboutData->version();
  else
      m_strVersion = i18n("no version set (programmer error!)");
  d->kde_version = QString::fromLatin1( KDE_VERSION_STRING );
  d->kde_version += ", " + QString::fromLatin1( KDE_DISTRIBUTION_TEXT );
  if ( !d->submitBugButton )
      m_strVersion += " " + d->kde_version;
  m_version = new QLabel( m_strVersion, parent );
  //glay->addWidget( m_version, row, 1 );
  glay->addWidget( m_version, row, 1, 1, 2 );
  m_version->setWhatsThis(qwtstr );

  tmpLabel = new QLabel(i18n("OS:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );

  struct utsname unameBuf;
  uname( &unameBuf );
  d->os = QString::fromLatin1( unameBuf.sysname ) +
          " (" + QString::fromLatin1( unameBuf.machine ) + ") "
          "release " + QString::fromLatin1( unameBuf.release );

  tmpLabel = new QLabel(d->os, parent);
  glay->addWidget( tmpLabel, row, 1, 1, 2 );

  tmpLabel = new QLabel(i18n("Compiler:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );
  tmpLabel = new QLabel(QString::fromLatin1(KDE_COMPILER_VERSION), parent);
  glay->addWidget( tmpLabel, row, 1, 1, 2 );

  if ( !d->submitBugButton )
  {
    // Severity
    m_bgSeverity = new QGroupBox( i18n("Se&verity"), parent );
    static const char * const sevNames[5] = { "critical", "grave", "normal", "wishlist", "i18n" };
    const QString sevTexts[5] = { i18n("Critical"), i18n("Grave"), i18n("normal severity","Normal"), i18n("Wishlist"), i18n("Translation") };
    QHBoxLayout *severityLayout=new QHBoxLayout(m_bgSeverity);
    for (int i = 0 ; i < 5 ; i++ )
    {
      // Store the severity string as the name
      QRadioButton *rb = new QRadioButton( sevTexts[i], m_bgSeverity);
      rb->setObjectName(sevNames[i] );
      d->severityButtons.append(rb);
      severityLayout->addWidget(rb);
      if (i==2) rb->setChecked(true); // default : "normal"
    }

    lay->addWidget( m_bgSeverity );

    // Subject
    QHBoxLayout * hlay = new QHBoxLayout();
    lay->addItem(hlay);
    tmpLabel = new QLabel( i18n("S&ubject: "), parent );
    hlay->addWidget( tmpLabel );
    m_subject = new KLineEdit( parent );
    m_subject->setFocus();
    tmpLabel->setBuddy(m_subject);
    hlay->addWidget( m_subject );

    QString text = i18n("Enter the text (in English if possible) that you wish to submit for the "
                        "bug report.\n"
                        "If you press \"Send\", a mail message will be sent to the maintainer of "
                        "this program.\n");
    QLabel * label = new QLabel( parent);

    label->setText( text );
    lay->addWidget( label );

    // The multiline-edit
    m_lineedit = new QTextEdit( parent);
    m_lineedit->setMinimumHeight( 180 ); // make it big
    m_lineedit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);//Q3MultiLineEdit::WidgetWidth);
    m_lineedit->setLineWrapMode(QTextEdit::WidgetWidth);
    lay->addWidget( m_lineedit, 10 /*stretch*/ );

    slotSetFrom();
  } else {
    // Point to the web form

    lay->addSpacing(10);
    QString text = i18n("To submit a bug report, click on the button below.\n"
                        "This will open a web browser window on http://bugs.kde.org where you will find a form to fill in.\n"
                        "The information displayed above will be transferred to that server.");
    QLabel * label = new QLabel( text, parent);
    lay->addWidget( label );
    lay->addSpacing(10);

    updateURL();
    d->submitBugButton->setText( i18n("&Launch Bug Report Wizard") );
    d->submitBugButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    lay->addWidget( d->submitBugButton );
    lay->addSpacing(10);

	connect( d->submitBugButton, SIGNAL(clicked()),
			 this, SLOT(accept()));
  }
  setMainWidget(parent);
}

KBugReport::~KBugReport()
{
    delete d;
}

void KBugReport::updateURL()
{
    KUrl url ( "http://bugs.kde.org/wizard.cgi" );
    url.addQueryItem( "os", d->os );
    url.addQueryItem( "compiler", KDE_COMPILER_VERSION );
    url.addQueryItem( "kdeVersion", d->kde_version );
    url.addQueryItem( "appVersion", m_strVersion );
    url.addQueryItem( "package", d->appcombo->currentText() );
    url.addQueryItem( "kbugreport", "1" );
    d->url = url;
}

void KBugReport::appChanged(int i)
{
    QString appName = d->appcombo->itemText(i);
    int index = appName.indexOf( '/' );
    if ( index > 0 )
        appName = appName.left( index );
    kDebug() << "appName " << appName << endl;

    if (d->appname == appName && m_aboutData)
        m_strVersion = m_aboutData->version();
    else
        m_strVersion = i18n("unknown program name", "unknown");

    if ( !d->submitBugButton )
        m_strVersion += d->kde_version;

    m_version->setText(m_strVersion);
    if ( d->submitBugButton )
        updateURL();
}

void KBugReport::slotConfigureEmail()
{
  if (m_process) return;
  m_process = new KProcess;
  *m_process << QString::fromLatin1("kcmshell") << QString::fromLatin1("kcm_useraccount");
  connect(m_process, SIGNAL(processExited(KProcess *)), SLOT(slotSetFrom()));
  if (!m_process->start())
  {
    kDebug() << "Couldn't start kcmshell.." << endl;
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

  // ### KDE4: why oh why is KEmailSettings in kio?
  KConfig emailConf( QString::fromLatin1("emaildefaults") );

  // find out the default profile
  emailConf.setGroup( QString::fromLatin1("Defaults") );
  QString profile = QString::fromLatin1("PROFILE_");
  profile += emailConf.readEntry( QString::fromLatin1("Profile"),
                                  QString::fromLatin1("Default") );

  emailConf.setGroup( profile );
  QString fromaddr = emailConf.readEntry( "EmailAddress" );
  if (fromaddr.isEmpty()) {
     struct passwd *p;
     p = getpwuid(getuid());
     fromaddr = QString::fromLatin1(p->pw_name);
  } else {
     QString name = emailConf.readEntry( "FullName" );
     if (!name.isEmpty())
        fromaddr = name + QString::fromLatin1(" <") + fromaddr + QString::fromLatin1(">");
  }
  m_from->setText( fromaddr );
}

void KBugReport::accept()
{
    if ( d->submitBugButton ) {
            KToolInvocation::invokeBrowser( d->url.url() );
        return;
    }

    if( m_lineedit->toPlainText().isEmpty() ||
        m_subject->text().isEmpty() )
    {
        QString msg = i18n("You must specify both a subject and a description "
                           "before the report can be sent.");
        KMessageBox::error(this,msg);
        return;
    }

	switch ( d->currentSeverity())
    {
        case 0: // critical
            if ( KMessageBox::questionYesNo( this, i18n(
                "<p>You chose the severity <b>Critical</b>. "
                "Please note that this severity is intended only for bugs that</p>"
                "<ul><li>break unrelated software on the system (or the whole system)</li>"
                "<li>cause serious data loss</li>"
                "<li>introduce a security hole on the system where the affected package is installed</li></ul>\n"
                "<p>Does the bug you are reporting cause any of the above damage? "
                "If it does not, please select a lower severity. Thank you!</p>" ),QString(),KStdGuiItem::cont(),KStdGuiItem::cancel() ) == KMessageBox::No )
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
                "If it does not, please select a lower severity. Thank you!</p>" ),QString(),KStdGuiItem::cont(),KStdGuiItem::cancel() ) == KMessageBox::No )
                return;
            break;
	default:
	    break;
    }
    if( !sendBugReport() )
    {
        QString msg = i18n("Unable to send the bug report.\n"
                           "Please submit a bug report manually...\n"
                           "See http://bugs.kde.org/ for instructions.");
        KMessageBox::error(this, msg + "\n\n" + d->lastError);
        return;
    }

    KMessageBox::information(this,
                             i18n("Bug report sent, thank you for your input."));
	KDialog::accept();
}

void KBugReport::closeEvent( QCloseEvent * e)
{
  if( !d->submitBugButton && ( (m_lineedit->toPlainText().length()>0) || m_subject->isModified() ) )
  {
    int rc = KMessageBox::warningYesNo( this,
             i18n( "Close and discard\nedited message?" ),
             i18n( "Close Message" ), KStdGuiItem::discard(), KStdGuiItem::cont() );
    if( rc == KMessageBox::No )
	{
		e->ignore();
      return;
	}
  }
  KDialog::closeEvent( e);
}


QString KBugReport::text() const
{
    kDebug() << d->severityButtons[d->currentSeverity()]->objectName() << endl;
    // Prepend the pseudo-headers to the contents of the mail
  QString severity = d->severityButtons[d->currentSeverity()]->objectName();
  QString appname = d->appcombo->currentText();
  QString os = QString::fromLatin1("OS: %1 (%2)\n").
               arg(KDE_COMPILING_OS).
               arg(KDE_DISTRIBUTION_TEXT);
  QString bodyText;
/*  for(int i = 0; i < m_lineedit->numLines(); i++)
  {
     QString line = m_lineedit->textLine(i);
     if (!line.endsWith("\n"))
        line += '\n';
     bodyText += line;
  }
*/
  bodyText=m_lineedit->toPlainText();
  if (bodyText.length()>0)
  	if (bodyText[bodyText.length()-1]!='\n') bodyText+="\n";
  if (severity == QString::fromLatin1("i18n") && KGlobal::locale()->language() != KLocale::defaultLanguage()) {
      // Case 1 : i18n bug
      QString package = QString::fromLatin1("i18n_%1").arg(KGlobal::locale()->language());
      package = package.replace(QString::fromLatin1("_"), QString::fromLatin1("-"));
      return QString::fromLatin1("Package: %1").arg(package) +
          QString::fromLatin1("\n"
                              "Application: %1\n"
                              // not really i18n's version, so better here IMHO
                              "Version: %2\n").arg(appname).arg(m_strVersion)+
          os+QString::fromLatin1("\n")+bodyText;
  } else {
      appname = appname.replace(QString::fromLatin1("_"), QString::fromLatin1("-"));
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
    kError() << "Unable to open a pipe to " << command << endl;
    return false;
  }

  QByteArray btext = text().toAscii();
  fwrite(btext.data(),btext.length(),1,fd);
  fflush(fd);

  int error = pclose(fd);
  kDebug() << "exit status1 " << error << " " << (WIFEXITED(error)) << " " <<  WEXITSTATUS(error) << endl;

  if ((WIFEXITED(error)) && WEXITSTATUS(error) == 1) {
      QFile of(outputfile.name());
      if (of.open(QIODevice::ReadOnly )) {
          QTextStream is(&of);
          //is.setEncoding(QTextStream::UnicodeUTF8);
          is.setCodec(QTextCodec::codecForName("UTF-8"));
          QString line;
          while (!is.atEnd())
              line = is.readLine();
          d->lastError = line;
      } else {
          d->lastError.clear();
      }
      outputfile.unlink();
      return false;
  }
  outputfile.unlink();
  return true;
}

void KBugReport::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }

#include "kbugreport.moc"

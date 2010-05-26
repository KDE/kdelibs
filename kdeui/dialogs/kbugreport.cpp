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

#include "kbugreport.h"

#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QGroupBox>
#include <QtGui/QCloseEvent>

#include <kaboutdata.h>
#include <kcombobox.h>
#include <ktoolinvocation.h>
#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kurllabel.h>
#include <ktextedit.h>
#include <ktitlewidget.h>

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "kdepackages.h"
#include "kdeversion.h"

#include <config-compiler.h>

class KBugReportPrivate {
public:
    KBugReportPrivate(KBugReport *q): q(q) {}

    void _k_slotConfigureEmail();
    void _k_slotSetFrom();
    void _k_appChanged(int);
    void _k_updateUrl();

    KBugReport *q;
    QProcess * m_process;
    const KAboutData * m_aboutData;

    KTextEdit * m_lineedit;
    KLineEdit * m_subject;
    QLabel * m_from;
    QLabel * m_version;
    QString m_strVersion;
    QGroupBox * m_bgSeverity;
    QPushButton * m_configureEmail;

    KComboBox *appcombo;
    QString lastError;
    QString kde_version;
    QString appname;
    QString os;
    KUrl url;
    QList<QRadioButton*> severityButtons;
    int currentSeverity() {
        for (int i=0;i<severityButtons.count();i++)
            if (severityButtons[i]->isChecked()) return i;
        return -1;
    }
    bool submitBugWeb;
};

KBugReport::KBugReport( QWidget * _parent, bool modal, const KAboutData *aboutData )
  : KDialog( _parent ), d( new KBugReportPrivate(this) )
{
  setCaption( i18n("Submit Bug Report") );
  setButtons( Ok | Cancel );
  setModal(modal);

  // Use supplied aboutdata, otherwise the one from the active componentData
  // otherwise the KGlobal one. _activeInstance should neved be 0L in theory.
  d->m_aboutData = aboutData ? aboutData
      : (KGlobal::activeComponent().isValid() ? KGlobal::activeComponent().aboutData()
                                  : KGlobal::mainComponent().aboutData());
  d->m_process = 0;
  QWidget * parent = new QWidget(this);
  d->submitBugWeb = false;

  if ( d->m_aboutData->bugAddress() == QLatin1String("submit@bugs.kde.org") )
  {
    // This is a core KDE application -> redirect to the web form
    d->submitBugWeb = true;
    setButtonGuiItem( Cancel, KStandardGuiItem::close() );
  }

  QLabel * tmpLabel;
  QVBoxLayout * lay = new QVBoxLayout( parent);

  KTitleWidget *title = new KTitleWidget( this );
  title->setText(i18n( "Submit Bug Report" ) );
  title->setPixmap( KIcon( "tools-report-bug" ).pixmap( 32 ) );
  lay->addWidget( title );

  QGridLayout *glay = new QGridLayout();
  lay->addLayout(glay);

  int row = 0;

  if ( !d->submitBugWeb )
  {
    // From
    QString qwtstr = i18n( "Your email address. If incorrect, use the Configure Email button to change it" );
    tmpLabel = new QLabel( i18nc("Email sender address", "From:"), parent );
    glay->addWidget( tmpLabel, row,0 );
    tmpLabel->setWhatsThis(qwtstr );
    d->m_from = new QLabel( parent );
    glay->addWidget( d->m_from, row, 1 );
    d->m_from->setWhatsThis(qwtstr );


    // Configure email button
    d->m_configureEmail = new QPushButton( i18n("Configure Email..."),
                                        parent );
    connect( d->m_configureEmail, SIGNAL( clicked() ), this,
             SLOT( _k_slotConfigureEmail() ) );
    glay->addWidget( d->m_configureEmail, 0, 2, 3, 1, Qt::AlignTop|Qt::AlignRight );

    // To
    qwtstr = i18n( "The email address this bug report is sent to." );
    tmpLabel = new QLabel( i18nc("Email receiver address", "To:"), parent );
    glay->addWidget( tmpLabel, ++row,0 );
    tmpLabel->setWhatsThis(qwtstr );
    tmpLabel = new QLabel( d->m_aboutData->bugAddress(), parent );
    tmpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    glay->addWidget( tmpLabel, row, 1 );
    tmpLabel->setWhatsThis(qwtstr );

    setButtonGuiItem( Ok,  KGuiItem( i18n("&Send"), "mail-send", i18n( "Send bug report." ),
                    i18n( "Send this bug report to %1." ,  d->m_aboutData->bugAddress() ) ) );
    row++;
  }
  else
  {
    d->m_configureEmail = 0;
    d->m_from = 0;
  }

  // Program name
  QString qwtstr = i18n( "The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application" );
  tmpLabel = new QLabel( i18n("Application: "), parent );
  glay->addWidget( tmpLabel, row, 0 );
  tmpLabel->setWhatsThis(qwtstr );
  d->appcombo = new KComboBox( false, parent );
  d->appcombo->setWhatsThis(qwtstr );
  QStringList packageList;
  for (int c = 0; packages[c]; ++c)
    packageList << QString::fromLatin1(packages[c]);
  d->appcombo->addItems(packageList);
  connect(d->appcombo, SIGNAL(activated(int)), SLOT(_k_appChanged(int)));
  d->appname = d->m_aboutData
                                    ? d->m_aboutData->productName()
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
  if (d->m_aboutData)
      d->m_strVersion = d->m_aboutData->version();
  else
      d->m_strVersion = i18n("no version set (programmer error)");
  d->kde_version = QString::fromLatin1( KDE_VERSION_STRING );
  d->kde_version += ", " + QString::fromLatin1( KDE_DISTRIBUTION_TEXT );
  if ( !d->submitBugWeb )
      d->m_strVersion += ' ' + d->kde_version;
  d->m_version = new QLabel( d->m_strVersion, parent );
  d->m_version->setTextInteractionFlags(Qt::TextBrowserInteraction);
  //glay->addWidget( d->m_version, row, 1 );
  glay->addWidget( d->m_version, row, 1, 1, 2 );
  d->m_version->setWhatsThis(qwtstr );

  tmpLabel = new QLabel(i18n("OS:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );

  struct utsname unameBuf;
  uname( &unameBuf );
  d->os = QString::fromLatin1( unameBuf.sysname ) +
          " (" + QString::fromLatin1( unameBuf.machine ) + ") "
          "release " + QString::fromLatin1( unameBuf.release );

  tmpLabel = new QLabel(d->os, parent);
  tmpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  glay->addWidget( tmpLabel, row, 1, 1, 2 );

  tmpLabel = new QLabel(i18n("Compiler:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );
  tmpLabel = new QLabel(QString::fromLatin1(KDE_COMPILER_VERSION), parent);
  tmpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  glay->addWidget( tmpLabel, row, 1, 1, 2 );

  if ( !d->submitBugWeb )
  {
    // Severity
    d->m_bgSeverity = new QGroupBox( i18n("Se&verity"), parent );
    static const char * const sevNames[5] = { "critical", "grave", "normal", "wishlist", "i18n" };
    const QString sevTexts[5] = { i18n("Critical"), i18n("Grave"), i18nc("normal severity","Normal"), i18n("Wishlist"), i18n("Translation") };
    QHBoxLayout *severityLayout=new QHBoxLayout(d->m_bgSeverity);
    for (int i = 0 ; i < 5 ; i++ )
    {
      // Store the severity string as the name
      QRadioButton *rb = new QRadioButton( sevTexts[i], d->m_bgSeverity);
      rb->setObjectName(sevNames[i] );
      d->severityButtons.append(rb);
      severityLayout->addWidget(rb);
      if (i==2) rb->setChecked(true); // default : "normal"
    }

    lay->addWidget( d->m_bgSeverity );

    // Subject
    QHBoxLayout * hlay = new QHBoxLayout();
    lay->addItem(hlay);
    tmpLabel = new QLabel( i18n("S&ubject: "), parent );
    hlay->addWidget( tmpLabel );
    d->m_subject = new KLineEdit( parent );
    d->m_subject->setClearButtonShown(true);
    d->m_subject->setFocus();
    tmpLabel->setBuddy( d->m_subject );
    hlay->addWidget( d->m_subject );

    QString text = i18n("Enter the text (in English if possible) that you wish to submit for the "
                        "bug report.\n"
                        "If you press \"Send\", a mail message will be sent to the maintainer of "
                        "this program.\n");
    QLabel * label = new QLabel( parent);

    label->setText( text );
    lay->addWidget( label );

    // The multiline-edit
    d->m_lineedit = new KTextEdit( parent);
    d->m_lineedit->setMinimumHeight( 180 ); // make it big
    d->m_lineedit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    d->m_lineedit->setLineWrapMode(QTextEdit::WidgetWidth);
    d->m_lineedit->setCheckSpellingEnabled(true);
    d->m_lineedit->setSpellCheckingLanguage("en");
    lay->addWidget( d->m_lineedit, 10 /*stretch*/ );

    d->_k_slotSetFrom();
  } else {
    // Point to the web form

    lay->addSpacing(10);
    QString text = i18n("<qt>To submit a bug report, click on the button below. This will open a web browser "
                        "window on <a href=\"http://bugs.kde.org\">http://bugs.kde.org</a> where you will find "
                        "a form to fill in. The information displayed above will be transferred to that server.</qt>");
    QLabel * label = new QLabel( text, parent);
    label->setOpenExternalLinks( true );
    label->setTextInteractionFlags( Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard );
    label->setWordWrap( true );
    lay->addWidget( label );
    lay->addSpacing(10);

    d->appcombo->setFocus();

    d->_k_updateUrl();

    setButtonText(Ok, i18n("&Launch Bug Report Wizard"));
    setButtonIcon(Ok, KIcon("tools-report-bug"));
  }
  parent->setMinimumHeight( parent->sizeHint().height() + 20 ); // WORKAROUND: prevent "cropped" kcombobox
  setMainWidget(parent);
}

KBugReport::~KBugReport()
{
    delete d;
}

QString KBugReport::messageBody() const
{
  if ( !d->submitBugWeb )
    return d->m_lineedit->toPlainText();
  else
    return QString();
}

void KBugReport::setMessageBody(const QString &messageBody)
{
  if ( !d->submitBugWeb )
    d->m_lineedit->setPlainText(messageBody);
}

void KBugReportPrivate::_k_updateUrl()
{
    url = KUrl( "https://bugs.kde.org/wizard.cgi" );
    url.addQueryItem( "os", os );
    url.addQueryItem( "compiler", KDE_COMPILER_VERSION );
    url.addQueryItem( "kdeVersion", kde_version );
    url.addQueryItem( "appVersion", m_strVersion );
    url.addQueryItem( "package", appcombo->currentText() );
    url.addQueryItem( "kbugreport", "1" );
}

void KBugReportPrivate::_k_appChanged(int i)
{
    QString appName = appcombo->itemText(i);
    int index = appName.indexOf( '/' );
    if ( index > 0 )
        appName = appName.left( index );
    kDebug() << "appName " << appName;

    QString strDisplayVersion; //Version string to show in the UI
    if (appname == appName && m_aboutData) {
        m_strVersion = m_aboutData->version();
        strDisplayVersion = m_strVersion;
    } else {
        m_strVersion = QLatin1String("unknown"); //English string to put in the bug report
        strDisplayVersion = i18nc("unknown program name", "unknown");
    }

    if ( !submitBugWeb ) {
        m_strVersion += ' ' + kde_version;
        strDisplayVersion += ' ' + kde_version;
    }

    m_version->setText(strDisplayVersion);
    if ( submitBugWeb )
        _k_updateUrl();
}

void KBugReportPrivate::_k_slotConfigureEmail()
{
  if (m_process) return;
  m_process = new QProcess;
  QObject::connect( m_process, SIGNAL(finished( int, QProcess::ExitStatus )), q, SLOT(_k_slotSetFrom()) );
  m_process->start( QString::fromLatin1("kcmshell4"), QStringList() << QString::fromLatin1("kcm_useraccount") );
  if ( !m_process->waitForStarted() )
  {
    kDebug() << "Couldn't start kcmshell4..";
    delete m_process;
    m_process = 0;
    return;
  }
  m_configureEmail->setEnabled(false);
}

void KBugReportPrivate::_k_slotSetFrom()
{
  delete m_process;
  m_process = 0;
  m_configureEmail->setEnabled(true);

  // ### KDE4: why oh why is KEmailSettings in kio?
  KConfig emailConf( QString::fromLatin1("emaildefaults") );

  KConfigGroup cg(&emailConf, "Defaults");
  // find out the default profile
  QString profile = QString::fromLatin1("PROFILE_");
  profile += cg.readEntry( QString::fromLatin1("Profile"),
                           QString::fromLatin1("Default") );

  KConfigGroup profileGrp(&emailConf, profile );
  QString fromaddr = profileGrp.readEntry( "EmailAddress" );
  if (fromaddr.isEmpty()) {
     struct passwd *p;
     p = getpwuid(getuid());
     fromaddr = QString::fromLatin1(p->pw_name);
  } else {
     QString name = profileGrp.readEntry( "FullName" );
     if (!name.isEmpty())
        fromaddr = name + QString::fromLatin1(" <") + fromaddr + QString::fromLatin1(">");
  }
  m_from->setText( fromaddr );
}

void KBugReport::accept()
{
    if ( d->submitBugWeb ) {
        KToolInvocation::invokeBrowser( d->url.url() );
        return;
    }

    if( d->m_lineedit->toPlainText().isEmpty() ||
        d->m_subject->text().isEmpty() )
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
                "Please note that this severity is intended only for bugs that:</p>"
                "<ul><li>break unrelated software on the system (or the whole system)</li>"
                "<li>cause serious data loss</li>"
                "<li>introduce a security hole on the system where the affected package is installed</li></ul>\n"
                "<p>Does the bug you are reporting cause any of the above damage? "
                "If it does not, please select a lower severity. Thank you.</p>" ),QString(),KStandardGuiItem::cont(),KStandardGuiItem::cancel() ) == KMessageBox::No )
                return;
            break;
        case 1: // grave
            if ( KMessageBox::questionYesNo( this, i18n(
                "<p>You chose the severity <b>Grave</b>. "
                "Please note that this severity is intended only for bugs that:</p>"
                "<ul><li>make the package in question unusable or mostly so</li>"
                "<li>cause data loss</li>"
                "<li>introduce a security hole allowing access to the accounts of users who use the affected package</li></ul>\n"
                "<p>Does the bug you are reporting cause any of the above damage? "
                "If it does not, please select a lower severity. Thank you.</p>" ),QString(),KStandardGuiItem::cont(),KStandardGuiItem::cancel() ) == KMessageBox::No )
                return;
            break;
        default:
            break;
    }
    if( !sendBugReport() )
    {
        QString msg = i18n("Unable to send the bug report.\n"
                           "Please submit a bug report manually....\n"
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
  if( !d->submitBugWeb && ( (d->m_lineedit->toPlainText().length()>0) || d->m_subject->isModified() ) )
  {
    int rc = KMessageBox::warningYesNo( this,
             i18n( "Close and discard\nedited message?" ),
             i18n( "Close Message" ), KStandardGuiItem::discard(), KStandardGuiItem::cont() );
    if( rc == KMessageBox::No )
    {
        e->ignore();
        return;
    }
  }
  KDialog::closeEvent(e);
}


QString KBugReport::text() const
{
    kDebug() << d->severityButtons[d->currentSeverity()]->objectName();
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
  bodyText=d->m_lineedit->toPlainText();
  if (bodyText.length()>0)
        if (bodyText[bodyText.length()-1]!='\n') bodyText+='\n';
  if (severity == QLatin1String("i18n") && KGlobal::locale()->language() != KLocale::defaultLanguage()) {
      // Case 1 : i18n bug
      QString package = QString::fromLatin1("i18n_%1").arg(KGlobal::locale()->language());
      package = package.replace('_', '-');
      return QString::fromLatin1("Package: %1").arg(package) +
          QString::fromLatin1("\n"
                              "Application: %1\n"
                              // not really i18n's version, so better here IMHO
                              "Version: %2\n").arg(appname).arg(d->m_strVersion)+
          os+QString::fromLatin1("\n")+bodyText;
  } else {
      appname = appname.replace('_', '-');
      // Case 2 : normal bug
      return QString::fromLatin1("Package: %1\n"
                                 "Version: %2\n"
                                 "Severity: %3\n")
          .arg(appname).arg(d->m_strVersion).arg(severity)+
          QString::fromLatin1("Compiler: %1\n").arg(KDE_COMPILER_VERSION)+
          os+QString::fromLatin1("\n")+bodyText;
  }
}

bool KBugReport::sendBugReport()
{
  QString recipient ( d->m_aboutData ?
    d->m_aboutData->bugAddress() :
    QString::fromLatin1("submit@bugs.kde.org") );

  QString command;
  command = KStandardDirs::locate("exe", "ksendbugmail");
  if (command.isEmpty())
      command = KStandardDirs::findExe( QString::fromLatin1("ksendbugmail") );

  QProcess proc;
  QStringList args;
  args << "--subject" << d->m_subject->text() << "--recipient" << recipient;
  proc.start( command, args );
  //kDebug() << command << args;
  if (!proc.waitForStarted())
  {
    kError() << "Unable to open a pipe to " << command << endl;
    return false;
  }
  proc.write( text().toUtf8() );
  proc.closeWriteChannel();

  proc.waitForFinished();
  kDebug() << "kbugreport: sendbugmail exit, status " << proc.exitStatus() << " code " << proc.exitCode();

  QByteArray line;
  if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() != 0) {
      // XXX not stderr?
      while (!proc.atEnd())
          line = proc.readLine();
      d->lastError = QString::fromUtf8( line );
      return false;
  }
  return true;
}


#include "kbugreport.moc"

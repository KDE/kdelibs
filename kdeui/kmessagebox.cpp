/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcheckbox.h>
#include <qguardedptr.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qstylesheet.h>
#include <qsimplerichtext.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kguiitem.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kstdguiitem.h>
#include <kactivelabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>

#include <X11/Xlib.h>

 /**
  * Easy MessageBox Dialog.
  *
  * Provides convenience functions for some i18n'ed standard dialogs,
  * as well as audible notification via @ref KNotifyClient
  *
  * @author Waldo Bastian (bastian@kde.org)
  */

static bool KMessageBox_queue = false;

static QPixmap themedMessageBoxIcon(QMessageBox::Icon icon)
{
    QString icon_name;

    switch(icon)
    {
    case QMessageBox::NoIcon:
        return QPixmap();
        break;
    case QMessageBox::Information:
        icon_name = "messagebox_info";
        break;
    case QMessageBox::Warning:
        icon_name = "messagebox_warning";
        break;
    case QMessageBox::Critical:
        icon_name = "messagebox_critical";
        break;
    default:
        break;
    }

   QPixmap ret = KApplication::kApplication()->iconLoader()->loadIcon(icon_name, KIcon::NoGroup, KIcon::SizeMedium, KIcon::DefaultState, 0, true);

   if (ret.isNull())
       return QMessageBox::standardIcon(icon);
   else
       return ret;
}

static void sendNotification( QString message, 
                              const QStringList& strlist, 
                              QMessageBox::Icon icon,
                              WId parent_id )
{
    // create the message for KNotify
    QString messageType;
    switch ( icon )
    {
        case QMessageBox::Warning:
            messageType = "messageWarning";
            break;
        case QMessageBox::Critical:
            messageType = "messageCritical";
            break;
#if QT_VERSION >= 0x030200
        case QMessageBox::Question:
            messageType = "messageQuestion";
            break;
#endif
        default:
            messageType = "messageInformation";
            break;
    }

    if ( !strlist.isEmpty() )
    {
        for ( QStringList::ConstIterator it = strlist.begin(); it != strlist.end(); ++it )
            message += "\n" + *it;
    }

    if ( !message.isEmpty() )
        KNotifyClient::event( parent_id, messageType, message );
}

static QString qrichtextify( const QString& text )
{
  if ( text.isEmpty() || text[0] == '<' )
    return text;

  QStringList lines = QStringList::split('\n', text);
  for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    *it = QStyleSheet::convertFromPlainText( *it, QStyleSheetItem::WhiteSpaceNormal );
  }

  return lines.join(QString::null);
}

static int createKMessageBox(KDialogBase *dialog, QMessageBox::Icon icon,
                             const QString &text, const QStringList &strlist,
                             const QString &ask, bool *checkboxReturn,
                             int options, const QString &details=QString::null)
{
    QVBox *topcontents = new QVBox (dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint());

    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint());

    QLabel *label1 = new QLabel( contents);

    if (icon != QMessageBox::NoIcon)
        label1->setPixmap(themedMessageBoxIcon(icon));

    lay->addWidget( label1, 0, Qt::AlignCenter );
    lay->addSpacing(KDialog::spacingHint());
    // Enforce <p>text</p> otherwise the word-wrap doesn't work well
    QString qt_text = qrichtextify( text );

    int pref_width = 0;
    int pref_height = 0;
    // Calculate a proper size for the text.
    {
       QSimpleRichText rt(qt_text, dialog->font());
       QRect d = KGlobalSettings::desktopGeometry(dialog);

       pref_width = d.width() / 3;
       rt.setWidth(pref_width);
       int used_width = rt.widthUsed();
       pref_height = rt.height();
       if (used_width <= pref_width)
       {
          while(true)
          {
             int new_width = (used_width * 9) / 10;
             rt.setWidth(new_width);
             int new_height = rt.height();
             if (new_height > pref_height)
                break;
             used_width = rt.widthUsed();
             if (used_width > new_width)
                break;
          }
          pref_width = used_width;
       }
       else
       {
          if (used_width > (pref_width *2))
             pref_width = pref_width *2;
          else
             pref_width = used_width;
       }
    }
    KActiveLabel *label2 = new KActiveLabel( qt_text, contents );
    if ((options & KMessageBox::AllowLink) == 0)
    {
       QObject::disconnect(label2, SIGNAL(linkClicked(const QString &)),
                  label2, SLOT(openLink(const QString &)));
    }

    // We add 10 pixels extra to compensate for some KActiveLabel margins.
    // TODO: find out why this is 10.
    label2->setFixedSize(QSize(pref_width+10, pref_height));
    lay->addWidget( label2 );
    lay->addStretch();

    KListBox *listbox = 0;
    if (!strlist.isEmpty())
    {
       listbox=new KListBox( topcontents );
       listbox->insertStringList( strlist );
       listbox->setSelectionMode( QListBox::NoSelection );
       topcontents->setStretchFactor(listbox, 1);
    }

    QGuardedPtr<QCheckBox> checkbox = 0;
    if (!ask.isEmpty())
    {
       checkbox = new QCheckBox(ask, topcontents);
    }

    if (!details.isEmpty())
    {
       QVGroupBox *detailsGroup = new QVGroupBox( i18n("Details"), dialog);
       if ( details.length() < 512 ) {
         KActiveLabel *label3 = new KActiveLabel(qrichtextify(details),
                                                 detailsGroup);
         label3->setMinimumSize(label3->sizeHint());
         if ((options & KMessageBox::AllowLink) == 0)
         {
           QObject::disconnect(label3, SIGNAL(linkClicked(const QString &)),
                               label3, SLOT(openLink(const QString &)));
         }
       } else {
         QTextEdit* te = new QTextEdit(details, QString::null, detailsGroup);
         te->setReadOnly( true );
         te->setMinimumHeight( te->fontMetrics().lineSpacing() * 11 );
       }
       dialog->setDetailsWidget(detailsGroup);
    }

    dialog->setMainWidget(topcontents);
    dialog->enableButtonSeparator(false);
    if (!listbox)
       dialog->disableResize();

    const KDialogBase::ButtonCode buttons[] = {
        KDialogBase::Help,
        KDialogBase::Default,
        KDialogBase::Ok,
        KDialogBase::Apply,
        KDialogBase::Try,
        KDialogBase::Cancel,
        KDialogBase::Close,
        KDialogBase::User1,
        KDialogBase::User2,
        KDialogBase::User3,
        KDialogBase::No,
        KDialogBase::Yes,
        KDialogBase::Details };
    for( unsigned int i = 0;
	 i < sizeof( buttons )/sizeof( buttons[ 0 ] );
	 ++i )
	if( QPushButton* btn = dialog->actionButton( buttons[ i ] ))
	    if( btn->isDefault())
		btn->setFocus();

    if ( (options & KMessageBox::Notify) != 0 )
    sendNotification( text, strlist, icon, dialog->topLevelWidget()->winId());
    
    if (KMessageBox_queue)
    {
       KDialogQueue::queueDialog(dialog);
       return KMessageBox::Cancel; // We have to return something.
    }
    
    // We use a QGuardedPtr because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the guarded ptr will reset to 0.
    QGuardedPtr<KDialogBase> guardedDialog = dialog;

    int result = guardedDialog->exec();
    if (checkbox && checkboxReturn)
       *checkboxReturn = checkbox->isChecked();
    delete (KDialogBase *) guardedDialog;
    return result;
}

int
KMessageBox::questionYesNo(QWidget *parent, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           int options)
{
   return questionYesNoList(parent, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::questionYesNoWId(WId parent_id, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           int options)
{
   return questionYesNoListWId(parent_id, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

bool
KMessageBox::shouldBeShownYesNo(const QString &dontShowAgainName,
                                ButtonCode &result)
{
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    if ( dontShowAgainName.isEmpty() ) return true;
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroupSaver saver( config, grpNotifMsgs );
    QString dontAsk = config->readEntry(dontShowAgainName).lower();
    if (dontAsk == "yes") {
        result = Yes;
        return false;
    }
    if (dontAsk == "no") {
        result = No;
        return false;
    }
    return true;
}

bool
KMessageBox::shouldBeShownContinue(const QString &dontShowAgainName)
{
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    if ( dontShowAgainName.isEmpty() ) return true;
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroupSaver saver( config, grpNotifMsgs );
    return config->readBoolEntry(dontShowAgainName,  true);
}

void
KMessageBox::saveDontShowAgainYesNo(const QString &dontShowAgainName,
                                    ButtonCode result)
{
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    if ( dontShowAgainName.isEmpty() ) return;
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroupSaver saver( config, grpNotifMsgs );
    config->writeEntry( dontShowAgainName, result==Yes ? "yes" : "no");
    config->sync();
}

void
KMessageBox::saveDontShowAgainContinue(const QString &dontShowAgainName)
{
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    if ( dontShowAgainName.isEmpty() ) return;
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroupSaver saver( config, grpNotifMsgs );
    config->writeEntry( dontShowAgainName, false);
    config->sync();
}

KConfig* KMessageBox::againConfig = NULL;
void
KMessageBox::setDontShowAskAgainConfig(KConfig* cfg)
{
  againConfig = cfg;
}

int
KMessageBox::questionYesNoList(QWidget *parent, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           int options)
{ // in order to avoid code duplication, convert to WId, it will be converted back
    return questionYesNoListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::questionYesNoListWId(WId parent_id, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           int options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Question") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "questionYesNo", true, true,
                       buttonYes, buttonNo);
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Information, text, strlist,
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::questionYesNoCancel(QWidget *parent,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          int options)
{
    return questionYesNoCancelWId( parent ? parent->winId() : 0, text, caption, buttonYes, buttonNo,
        dontAskAgainName, options );
}

int
KMessageBox::questionYesNoCancelWId(WId parent_id,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          int options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Question") : caption,
                       KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                       KDialogBase::Yes, KDialogBase::Cancel,
                       parent, "questionYesNoCancel", true, true,
                       buttonYes, buttonNo);
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Information,
                       text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);
    if ( result==KDialogBase::Cancel ) return Cancel;
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          int options)
{
   return warningYesNoList(parent, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoWId(WId parent_id, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          int options)
{
   return warningYesNoListWId(parent_id, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoList(QWidget *parent, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const QString &dontAskAgainName,
                              int options)
{
    return warningYesNoListWId( parent ? parent->winId() : 0, text, strlist, caption,
        buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::warningYesNoListWId(WId parent_id, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const QString &dontAskAgainName,
                              int options)
{ 
    // warningYesNo and warningYesNoList are always "dangerous"
    // ### Remove this line for KDE 4, when the 'options' default parameter
    // takes effects.
    options |= Dangerous;

    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::No, KDialogBase::No,
                       parent, "warningYesNoList", true, true,
                       buttonYes, buttonNo);
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::warningContinueCancel(QWidget *parent,
                                   const QString &text,
                                   const QString &caption,
                                   const KGuiItem &buttonContinue,
                                   const QString &dontAskAgainName,
                                   int options)
{
   return warningContinueCancelList(parent, text, QStringList(), caption,
                                buttonContinue, dontAskAgainName, options);
}

int
KMessageBox::warningContinueCancelWId(WId parent_id,
                                   const QString &text,
                                   const QString &caption,
                                   const KGuiItem &buttonContinue,
                                   const QString &dontAskAgainName,
                                   int options)
{
   return warningContinueCancelListWId(parent_id, text, QStringList(), caption,
                                buttonContinue, dontAskAgainName, options);
}

int
KMessageBox::warningContinueCancelList(QWidget *parent, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue,
                             const QString &dontAskAgainName,
                             int options)
{
    return warningContinueCancelListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonContinue, dontAskAgainName, options );
}

int
KMessageBox::warningContinueCancelListWId(WId parent_id, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue,
                             const QString &dontAskAgainName,
                             int options)
{
    if ( !shouldBeShownContinue(dontAskAgainName) )
        return Continue;

    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "warningYesNo", true, true,
                       buttonContinue, KStdGuiItem::cancel() );
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);

    if ( result==KDialogBase::No )
        return Cancel;
    if (checkboxResult)
        saveDontShowAgainContinue(dontAskAgainName);
    return Continue;
}

int
KMessageBox::warningYesNoCancel(QWidget *parent, const QString &text,
                                const QString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const QString &dontAskAgainName,
                                int options)
{
   return warningYesNoCancelList(parent, text, QStringList(), caption,
                      buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoCancelWId(WId parent_id, const QString &text,
                                const QString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const QString &dontAskAgainName,
                                int options)
{
   return warningYesNoCancelListWId(parent_id, text, QStringList(), caption,
                      buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoCancelList(QWidget *parent, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const QString &dontAskAgainName,
                                    int options)
{
    return warningYesNoCancelListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::warningYesNoCancelListWId(WId parent_id, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const QString &dontAskAgainName,
                                    int options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                       KDialogBase::Yes, KDialogBase::Cancel,
                       parent, "warningYesNoCancel", true, true,
                       buttonYes, buttonNo);
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);
    if ( result==KDialogBase::Cancel ) return Cancel;
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

void
KMessageBox::error(QWidget *parent,  const QString &text,
                   const QString &caption, int options)
{
    return errorWId( parent ? parent->winId() : 0, text, caption, options );
}

void
KMessageBox::errorWId(WId parent_id,  const QString &text,
                   const QString &caption, int options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString::null, 0, options);
}

void
KMessageBox::detailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption, int options)
{
    return detailedErrorWId( parent ? parent->winId() : 0, text, details, caption, options );
}

void
KMessageBox::detailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption, int options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString::null, 0, options, details);
}

void
KMessageBox::queuedDetailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
    return queuedDetailedErrorWId( parent ? parent->winId() : 0, text, details, caption );
}

void
KMessageBox::queuedDetailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
   KMessageBox_queue = true;
   (void) detailedErrorWId(parent_id, text, details, caption);
   KMessageBox_queue = false;
}


void
KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption, int options)
{
    return sorryWId( parent ? parent->winId() : 0, text, caption, options );
}

void
KMessageBox::sorryWId(WId parent_id, const QString &text,
                   const QString &caption, int options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString::null, 0, options);
}

void
KMessageBox::detailedSorry(QWidget *parent, const QString &text,
                   const QString &details,
                   const QString &caption, int options)
{
    return detailedSorryWId( parent ? parent->winId() : 0, text, details, caption, options );
}

void
KMessageBox::detailedSorryWId(WId parent_id, const QString &text,
                   const QString &details,
                   const QString &caption, int options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString::null, 0, options, details);
}

void
KMessageBox::information(QWidget *parent,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, int options)
{
  informationList(parent, text, QStringList(), caption, dontShowAgainName, options);
}

void
KMessageBox::informationWId(WId parent_id,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, int options)
{
  informationListWId(parent_id, text, QStringList(), caption, dontShowAgainName, options);
}

void
KMessageBox::informationList(QWidget *parent,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, int options)
{
    return informationListWId( parent ? parent->winId() : 0, text, strlist, caption,
        dontShowAgainName, options );
}

void
KMessageBox::informationListWId(WId parent_id,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, int options)
{
    if ( !shouldBeShownContinue(dontShowAgainName) )
        return;

    QWidget* parent = QWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Information") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "information", true, true,
                       KStdGuiItem::ok() );
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( qt_xdisplay(), dialog->winId(), parent_id );

    bool checkboxResult = false;

    createKMessageBox(dialog, QMessageBox::Information, text, strlist,
		dontShowAgainName.isEmpty() ? QString::null : i18n("&Do not show this message again"),
                &checkboxResult, options);

    if (checkboxResult)
        saveDontShowAgainContinue(dontShowAgainName);
}

void
KMessageBox::enableAllMessages()
{
   KConfig *config = againConfig ? againConfig : KGlobal::config();
   QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
   if (!config->hasGroup(grpNotifMsgs))
      return;

   KConfigGroupSaver saver( config, grpNotifMsgs );

   typedef QMap<QString, QString> configMap;

   configMap map = config->entryMap(grpNotifMsgs);

   configMap::Iterator it;
   for (it = map.begin(); it != map.end(); ++it)
      config->deleteEntry( it.key() );
   config->sync();
}

void
KMessageBox::enableMessage(const QString &dontShowAgainName)
{
   KConfig *config = againConfig ? againConfig : KGlobal::config();
   QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
   if (!config->hasGroup(grpNotifMsgs))
      return;

   KConfigGroupSaver saver( config, grpNotifMsgs );

   config->deleteEntry(dontShowAgainName);
   config->sync();
}

void
KMessageBox::about(QWidget *parent, const QString &text,
                   const QString &caption, int /* options */)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("About %1").arg(kapp->caption());

    QMessageBox *box = new QMessageBox( _caption, text,
              QMessageBox::Information,
              QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
              0, 0,
              parent, "about" );

    box->setButtonText(QMessageBox::Ok, i18n("&OK"));
    box->setIconPixmap(kapp->icon());
    box->adjustSize();
    box->setFixedSize(box->size());

    box->exec();
    delete box;
    return;
}

int KMessageBox::messageBox( QWidget *parent, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const QString &dontShowAskAgainName,
                             int options )
{
    return messageBoxWId( parent ? parent->winId() : 0, type, text, caption,
        buttonYes, buttonNo, dontShowAskAgainName, options );
}

int KMessageBox::messageBox( QWidget *parent, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, int options )
{
    return messageBoxWId( parent ? parent->winId() : 0, type, text, caption,
        buttonYes, buttonNo, QString::null, options );
}

int KMessageBox::messageBoxWId( WId parent_id, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const QString &dontShow,
                             int options )
{
    switch (type) {
        case QuestionYesNo:
            return KMessageBox::questionYesNoWId( parent_id,
                                               text, caption, buttonYes, buttonNo, dontShow, options );
        case QuestionYesNoCancel:
            return KMessageBox::questionYesNoCancelWId( parent_id,
                                               text, caption, buttonYes, buttonNo, dontShow, options );
        case WarningYesNo:
            return KMessageBox::warningYesNoWId( parent_id,
                                              text, caption, buttonYes, buttonNo, dontShow, options );
        case WarningContinueCancel:
            return KMessageBox::warningContinueCancelWId( parent_id,
                                              text, caption, buttonYes.text(), dontShow, options );
        case WarningYesNoCancel:
            return KMessageBox::warningYesNoCancelWId( parent_id,
                                              text, caption, buttonYes, buttonNo, dontShow, options );
        case Information:
            KMessageBox::informationWId( parent_id,
                                      text, caption, dontShow, options );
            return KMessageBox::Ok;

        case Error:
            KMessageBox::errorWId( parent_id, text, caption, options );
            return KMessageBox::Ok;

        case Sorry:
            KMessageBox::sorryWId( parent_id, text, caption, options );
            return KMessageBox::Ok;
    }
    return KMessageBox::Cancel;
}

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption, int options )
{
    return queuedMessageBoxWId( parent ? parent->winId() : 0, type, text, caption, options );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption, int options )
{
   KMessageBox_queue = true;
   (void) messageBoxWId(parent_id, type, text, caption, KStdGuiItem::yes(),
                     KStdGuiItem::no(), QString::null, options);
   KMessageBox_queue = false;
}

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption )
{
    return queuedMessageBoxWId( parent ? parent->winId() : 0, type, text, caption );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption )
{
   KMessageBox_queue = true;
   (void) messageBoxWId(parent_id, type, text, caption);
   KMessageBox_queue = false;
}

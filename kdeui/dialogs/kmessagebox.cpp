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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kmessagebox.h"

#include <QtCore/QPointer>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QScrollArea>
#include <QtGui/QTextDocumentFragment>

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kdialogqueue_p.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <knotification.h>
#include <kiconloader.h>
#include <kconfiggroup.h>
#include <ktextedit.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

// Some i18n filters, that standard button texts are piped through
// (the new KGuiItem object with filtered text is created from the old one).

// i18n: Filter for the Yes-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_YES(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-yes", "%1", src.text() ) );

// i18n: Filter for the No-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_NO(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-no", "%1", src.text() ) );

// i18n: Filter for the Continue-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_CONTINUE(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-continue", "%1", src.text() ) );

// i18n: Filter for the Cancel-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_CANCEL(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-cancel", "%1", src.text() ) );

// i18n: Called after the button texts in standard message dialogs
// have been filtered by the messages above. Not visible to user.
#define I18N_POST_BUTTON_FILTER \
    i18nc( "@action:button post-filter", "." );

static bool KMessageBox_queue = false;
KConfig* KMessageBox_againConfig = 0;


static QIcon themedMessageBoxIcon(QMessageBox::Icon icon)
{
    QString icon_name;

    switch (icon) {
    case QMessageBox::NoIcon:
        return QIcon();
        break;
    case QMessageBox::Information:
        icon_name = "dialog-information";
        break;
    case QMessageBox::Warning:
        icon_name = "dialog-warning";
        break;
    case QMessageBox::Critical:
        icon_name = "dialog-error";
        break;
    default:
        break;
    }

   QIcon ret = KIconLoader::global()->loadIcon(icon_name, KIconLoader::NoGroup, KIconLoader::SizeHuge, KIconLoader::DefaultState, QStringList(), 0, true);

   if (ret.isNull()) {
       return QMessageBox::standardIcon(icon);
   } else {
       return ret;
   }
}

static void sendNotification( QString message, //krazy:exclude=passbyvalue
                              const QStringList& strlist,
                              QMessageBox::Icon icon,
                              WId parent_id )
{
    // create the message for KNotify
    QString messageType;
    switch (icon) {
    case QMessageBox::Warning:
        messageType = "messageWarning";
        break;
    case QMessageBox::Critical:
        messageType = "messageCritical";
        break;
    case QMessageBox::Question:
        messageType = "messageQuestion";
        break;
    default:
        messageType = "messageInformation";
        break;
    }

    if ( !strlist.isEmpty() ) {
        for ( QStringList::ConstIterator it = strlist.begin(); it != strlist.end(); ++it ) {
            message += '\n' + *it;
        }
    }

    if ( !message.isEmpty() ) {
        KNotification::event( messageType, message , QPixmap() , QWidget::find( parent_id ) );
    }
}


int KMessageBox::createKMessageBox(KDialog *dialog, QMessageBox::Icon icon,
                             const QString &text, const QStringList &strlist,
                             const QString &ask, bool *checkboxReturn,
                             Options options, const QString &details)
{
    return createKMessageBox(dialog, themedMessageBoxIcon(icon), text, strlist,
                      ask, checkboxReturn, options, details, icon);
}

static int longest_line( const QFontMetrics & fm, const QString & text ) {
    const QStringList lines = QTextDocumentFragment::fromHtml( text ).toPlainText().split( QLatin1String( "\n" ) );
    int len = 0;
    Q_FOREACH( const QString & line, lines )
        len = qMax( len, fm.width( line ) );
    return len;
}

int KMessageBox::createKMessageBox(KDialog *dialog, const QIcon &icon,
                             const QString &text, const QStringList &strlist,
                             const QString &ask, bool *checkboxReturn, Options options,
                             const QString &details, QMessageBox::Icon notifyType)
{
    QWidget *mainWidget = new QWidget(dialog);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setSpacing(KDialog::spacingHint() * 2);
    mainLayout->setMargin(0);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(KDialog::spacingHint());
    mainLayout->addLayout(hLayout);
    mainLayout->addStretch();

    QLabel *iconLabel = new QLabel(mainWidget);

    if (!icon.isNull()) {
       iconLabel->setPixmap(icon.pixmap(KIconLoader::SizeHuge));
    }

    QVBoxLayout *iconLayout = new QVBoxLayout();
    iconLayout->addStretch(1);
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch(5);

    hLayout->addLayout(iconLayout);
    hLayout->addSpacing(KDialog::spacingHint());

    QLabel *messageLabel = new QLabel(text, mainWidget);
    messageLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    QPalette messagePal(messageLabel->palette());
    messagePal.setColor(QPalette::Window, Qt::transparent);
    messageLabel->setPalette(messagePal);

    QRect desktop = KGlobalSettings::desktopGeometry(dialog);
    if (desktop.width() / 3 < longest_line(messageLabel->fontMetrics(), text)) {
        // do only enable automatic wrapping of messages which are longer than one third of the current screen
        messageLabel->setWordWrap(true);
    }

    QScrollArea* messageScrollArea = new QScrollArea(mainWidget);
    messageScrollArea->setWidget(messageLabel);
    messageScrollArea->setFrameShape(QFrame::NoFrame);
    messageScrollArea->setWidgetResizable(true);
    QPalette scrollPal(messageScrollArea->palette());
    scrollPal.setColor(QPalette::Window, Qt::transparent);
    messageScrollArea->setPalette(scrollPal);

    hLayout->addWidget(messageScrollArea);

    QListWidget *listWidget = 0;
    if (!strlist.isEmpty()) {
        // enable automatic wrapping since the listwidget has already a good initial width
        messageLabel->setWordWrap(true);
        listWidget = new QListWidget(mainWidget);
        mainLayout->addWidget(listWidget);
        listWidget->addItems(strlist);
        listWidget->setSelectionMode(QListWidget::NoSelection);
        mainLayout->setStretchFactor(listWidget, 1);
    }

    QPointer<QCheckBox> checkbox = 0;
    if (!ask.isEmpty()) {
        checkbox = new QCheckBox(ask, mainWidget);
        mainLayout->addWidget(checkbox);
        if (checkboxReturn) {
            checkbox->setChecked(*checkboxReturn);
        }
    }

    if (!details.isEmpty()) {
        QGroupBox *detailsGroup = new QGroupBox(i18n("Details"));
        QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);
        if (details.length() < 512) {
            QLabel *detailsLabel = new QLabel(details);
            detailsLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
            detailsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
            detailsLabel->setWordWrap(true);
            detailsLayout->addWidget(detailsLabel);
        } else {
            KTextEdit *detailTextEdit = new KTextEdit(details);
            detailTextEdit->setReadOnly(true);
            detailTextEdit->setMinimumHeight(detailTextEdit->fontMetrics().lineSpacing() * 11);
            detailsLayout->addWidget(detailTextEdit);
        }
        dialog->setDetailsWidget(detailsGroup);
    }

    dialog->setMainWidget(mainWidget);
    dialog->showButtonSeparator(true);
    if (!listWidget) {
        int hfw = messageLabel->heightForWidth(messageScrollArea->sizeHint().width() - 2);
        if (hfw != messageScrollArea->sizeHint().height() && hfw < desktop.height() / 2) {
            messageScrollArea->setMinimumHeight(hfw);
        }
        if (details.isEmpty())
            dialog->setFixedSize(dialog->sizeHint() + QSize( 10, 10 ));
    }

    if ((options & KMessageBox::Dangerous)) {
        if (dialog->isButtonEnabled(KDialog::Cancel))
            dialog->setDefaultButton(KDialog::Cancel);
        else if (dialog->isButtonEnabled(KDialog::No))
            dialog->setDefaultButton(KDialog::No);
    }

    KDialog::ButtonCode defaultCode = dialog->defaultButton();
    if (defaultCode != KDialog::NoDefault) {
        dialog->setButtonFocus(defaultCode);
    }

#ifndef Q_WS_WIN // FIXME problems with KNotify on Windows
    if ((options & KMessageBox::Notify)) {
        sendNotification(text, strlist, notifyType, dialog->topLevelWidget()->winId());
    }
#endif

    if (KMessageBox_queue) {
        KDialogQueue::queueDialog(dialog);
        return KMessageBox::Cancel; // We have to return something.
    }

    if ((options & KMessageBox::NoExec)) {
        return KMessageBox::Cancel; // We have to return something.
    }

    // We use a QPointer because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the QPointer will reset to 0.
    QPointer<KDialog> guardedDialog = dialog;

    int result = guardedDialog->exec();
    if (checkbox && checkboxReturn) {
        *checkboxReturn = checkbox->isChecked();
    }

    delete (KDialog *) guardedDialog;
    return result;
}

int KMessageBox::questionYesNo(QWidget *parent, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    return questionYesNoList(parent, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

int KMessageBox::questionYesNoWId(WId parent_id, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    return questionYesNoListWId(parent_id, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

bool KMessageBox::shouldBeShownYesNo(const QString &dontShowAgainName,
                                ButtonCode &result)
{
    if ( dontShowAgainName.isEmpty() ) {
        return true;
    }
    KConfigGroup cg( KMessageBox_againConfig ? KMessageBox_againConfig : KGlobal::config().data(), "Notification Messages" );
    QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
    if (dontAsk == "yes" || dontAsk == "true") {
        result = Yes;
        return false;
    }
    if (dontAsk == "no" || dontAsk == "false") {
        result = No;
        return false;
    }
    return true;
}

bool KMessageBox::shouldBeShownContinue(const QString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) {
        return true;
    }
    KConfigGroup cg( KMessageBox_againConfig ? KMessageBox_againConfig : KGlobal::config().data(), "Notification Messages" );
    return cg.readEntry(dontShowAgainName, true);
}

void KMessageBox::saveDontShowAgainYesNo(const QString &dontShowAgainName,
                                    ButtonCode result)
{
    if ( dontShowAgainName.isEmpty() ) {
        return;
    }
    KConfigGroup::WriteConfigFlags flags = KConfig::Persistent;
    if (dontShowAgainName[0] == ':') {
        flags |= KConfigGroup::Global;
    }
    KConfigGroup cg( KMessageBox_againConfig? KMessageBox_againConfig : KGlobal::config().data(), "Notification Messages" );
    cg.writeEntry( dontShowAgainName, result==Yes, flags );
    cg.sync();
}

void KMessageBox::saveDontShowAgainContinue(const QString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) {
        return;
    }
    KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
    if (dontShowAgainName[0] == ':') {
        flags |= KConfigGroup::Global;
    }
    KConfigGroup cg( KMessageBox_againConfig? KMessageBox_againConfig: KGlobal::config().data(), "Notification Messages" );
    cg.writeEntry( dontShowAgainName, false, flags );
    cg.sync();
}

void KMessageBox::setDontShowAskAgainConfig(KConfig* cfg)
{
    KMessageBox_againConfig = cfg;
}

int KMessageBox::questionYesNoList(QWidget *parent, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{ // in order to avoid code duplication, convert to WId, it will be converted back
    return questionYesNoListWId( parent ? parent->effectiveWinId() : 0, text, strlist,
        caption, buttonYes, buttonNo, dontAskAgainName, options );
}

int KMessageBox::questionYesNoListWId(WId parent_id, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes_,
                           const KGuiItem &buttonNo_,
                           const QString &dontAskAgainName,
                           Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) ) {
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_POST_BUTTON_FILTER

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Question") : caption );
    dialog->setButtons( KDialog::Yes | KDialog::No );
    dialog->setObjectName( "questionYesNo" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::No );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Information, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialog::Yes ? Yes : No);

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

int KMessageBox::questionYesNoCancel(QWidget *parent,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const KGuiItem &buttonCancel,
                          const QString &dontAskAgainName,
                          Options options)
{
    return questionYesNoCancelWId( parent ? parent->effectiveWinId() : 0, text, caption, buttonYes, buttonNo, buttonCancel,
        dontAskAgainName, options );
}

int KMessageBox::questionYesNoCancelWId(WId parent_id,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes_,
                          const KGuiItem &buttonNo_,
                          const KGuiItem &buttonCancel_,
                          const QString &dontAskAgainName,
                          Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) ) {
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_FILTER_BUTTON_CANCEL(buttonCancel_, buttonCancel)
    I18N_POST_BUTTON_FILTER

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Question") : caption );
    dialog->setButtons( KDialog::Yes | KDialog::No | KDialog::Cancel );
    dialog->setObjectName( "questionYesNoCancel" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setButtonGuiItem( KDialog::Cancel, buttonCancel );
    dialog->setDefaultButton( KDialog::Yes );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Information,
                       text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result == KDialog::Yes ) {
        res = Yes;
    } else if ( result == KDialog::No ) {
        res = No;
    } else {
        return Cancel;
    }

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

int KMessageBox::warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
    return warningYesNoList(parent, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int KMessageBox::warningYesNoWId(WId parent_id, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
    return warningYesNoListWId(parent_id, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int KMessageBox::warningYesNoList(QWidget *parent, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const QString &dontAskAgainName,
                              Options options)
{
    return warningYesNoListWId( parent ? parent->effectiveWinId() : 0, text, strlist, caption,
        buttonYes, buttonNo, dontAskAgainName, options );
}

int KMessageBox::warningYesNoListWId(WId parent_id, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes_,
                              const KGuiItem &buttonNo_,
                              const QString &dontAskAgainName,
                              Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) ) {
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_POST_BUTTON_FILTER

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Warning") : caption );
    dialog->setButtons( KDialog::Yes | KDialog::No );
    dialog->setObjectName( "warningYesNoList" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setDefaultButton( KDialog::No );
    dialog->setEscapeButton( KDialog::No );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialog::Yes ? Yes : No);

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

int KMessageBox::warningContinueCancel(QWidget *parent,
                                   const QString &text,
                                   const QString &caption,
                                   const KGuiItem &buttonContinue,
                                   const KGuiItem &buttonCancel,
                                   const QString &dontAskAgainName,
                                   Options options)
{
    return warningContinueCancelList(parent, text, QStringList(), caption,
                                buttonContinue, buttonCancel, dontAskAgainName, options);
}

int KMessageBox::warningContinueCancelWId(WId parent_id,
                                   const QString &text,
                                   const QString &caption,
                                   const KGuiItem &buttonContinue,
                                   const KGuiItem &buttonCancel,
                                   const QString &dontAskAgainName,
                                   Options options)
{
    return warningContinueCancelListWId(parent_id, text, QStringList(), caption,
                                buttonContinue, buttonCancel, dontAskAgainName, options);
}

int KMessageBox::warningContinueCancelList(QWidget *parent, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue,
                             const KGuiItem &buttonCancel,
                             const QString &dontAskAgainName,
                             Options options)
{
    return warningContinueCancelListWId( parent ? parent->effectiveWinId() : 0, text, strlist,
        caption, buttonContinue, buttonCancel, dontAskAgainName, options );
}

int KMessageBox::warningContinueCancelListWId(WId parent_id, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue_,
                             const KGuiItem &buttonCancel_,
                             const QString &dontAskAgainName,
                             Options options)
{
    if ( !shouldBeShownContinue(dontAskAgainName) )
        return Continue;

    I18N_FILTER_BUTTON_CONTINUE(buttonContinue_, buttonContinue)
    I18N_FILTER_BUTTON_CANCEL(buttonCancel_, buttonCancel)
    I18N_POST_BUTTON_FILTER

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Warning") : caption );
    dialog->setButtons( KDialog::Yes | KDialog::No );
    dialog->setObjectName( "warningYesNo" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonContinue );
    dialog->setButtonGuiItem( KDialog::No, buttonCancel );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::No );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result != KDialog::Yes ) {
        return Cancel;
    }
    if (checkboxResult) {
        saveDontShowAgainContinue(dontAskAgainName);
    }
    return Continue;
}

int KMessageBox::warningYesNoCancel(QWidget *parent, const QString &text,
                                const QString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const KGuiItem &buttonCancel,
                                const QString &dontAskAgainName,
                                Options options)
{
    return warningYesNoCancelList(parent, text, QStringList(), caption,
                      buttonYes, buttonNo, buttonCancel, dontAskAgainName, options);
}

int KMessageBox::warningYesNoCancelWId(WId parent_id, const QString &text,
                                const QString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const KGuiItem &buttonCancel,
                                const QString &dontAskAgainName,
                                Options options)
{
    return warningYesNoCancelListWId(parent_id, text, QStringList(), caption,
                      buttonYes, buttonNo, buttonCancel, dontAskAgainName, options);
}

int KMessageBox::warningYesNoCancelList(QWidget *parent, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const KGuiItem &buttonCancel,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    return warningYesNoCancelListWId( parent ? parent->effectiveWinId() : 0, text, strlist,
        caption, buttonYes, buttonNo, buttonCancel, dontAskAgainName, options );
}

int KMessageBox::warningYesNoCancelListWId(WId parent_id, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes_,
                                    const KGuiItem &buttonNo_,
                                    const KGuiItem &buttonCancel_,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) ) {
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_FILTER_BUTTON_CANCEL(buttonCancel_, buttonCancel)
    I18N_POST_BUTTON_FILTER

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Warning") : caption );
    dialog->setButtons( KDialog::Yes | KDialog::No | KDialog::Cancel );
    dialog->setObjectName( "warningYesNoCancel" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setButtonGuiItem( KDialog::Cancel, buttonCancel );
    dialog->setDefaultButton( KDialog::Yes );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result == KDialog::Yes ) {
        res = Yes;
    } else if ( result == KDialog::No ) {
        res = No;
    } else {
        return Cancel;
    }

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

void KMessageBox::error(QWidget *parent,  const QString &text,
                   const QString &caption, Options options)
{
    return errorListWId( parent ? parent->effectiveWinId() : 0, text, QStringList(), caption, options );
}

void KMessageBox::errorWId(WId parent_id, const QString &text,
                      const QString &caption, Options options)
{
    errorListWId( parent_id, text, QStringList(), caption, options );
}

void KMessageBox::errorList(QWidget *parent, const QString &text, const QStringList &strlist,
                       const QString &caption, Options options)
{
    return errorListWId( parent ? parent->effectiveWinId() : 0, text, strlist, caption, options );
}

void KMessageBox::errorListWId(WId parent_id,  const QString &text, const QStringList &strlist,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Error") : caption );
    dialog->setButtons( KDialog::Yes );
    dialog->setObjectName( "error" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStandardGuiItem::ok().text() );
    dialog->setButtonToolTip( KDialog::Yes, QString() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    createKMessageBox(dialog, QMessageBox::Critical, text, strlist, QString(), 0, options);
}

void
KMessageBox::detailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    return detailedErrorWId( parent ? parent->effectiveWinId() : 0, text, details, caption, options );
}

void KMessageBox::detailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Error") : caption );
    dialog->setButtons( KDialog::Yes | KDialog::Details );
    dialog->setObjectName( "error" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStandardGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString(), 0, options, details);
}

void KMessageBox::queuedDetailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
    return queuedDetailedErrorWId( parent ? parent->effectiveWinId() : 0, text, details, caption );
}

void KMessageBox::queuedDetailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
   KMessageBox_queue = true;
   (void) detailedErrorWId(parent_id, text, details, caption);
   KMessageBox_queue = false;
}


void KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption, Options options)
{
    return sorryWId( parent ? parent->effectiveWinId() : 0, text, caption, options );
}

void KMessageBox::sorryWId(WId parent_id, const QString &text,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Sorry") : caption );
    dialog->setButtons( KDialog::Yes );
    dialog->setObjectName( "sorry" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStandardGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString(), 0, options);
}

void KMessageBox::detailedSorry(QWidget *parent, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    return detailedSorryWId( parent ? parent->effectiveWinId() : 0, text, details, caption, options );
}

void KMessageBox::detailedSorryWId(WId parent_id, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Sorry") : caption );
    dialog->setButtons( KDialog::Yes | KDialog::Details );
    dialog->setObjectName( "sorry" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStandardGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString(), 0, options, details);
}

void KMessageBox::information(QWidget *parent,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, Options options)
{
    informationList(parent, text, QStringList(), caption, dontShowAgainName, options);
}

void KMessageBox::informationWId(WId parent_id,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, Options options)
{
    informationListWId(parent_id, text, QStringList(), caption, dontShowAgainName, options);
}

void KMessageBox::informationList(QWidget *parent,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    return informationListWId( parent ? parent->effectiveWinId() : 0, text, strlist, caption,
        dontShowAgainName, options );
}

void KMessageBox::informationListWId(WId parent_id,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    if ( !shouldBeShownContinue(dontShowAgainName) ) {
        return;
    }

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption.isEmpty() ? i18n("Information") : caption );
    dialog->setButtons( KDialog::Yes );
    dialog->setObjectName( "information" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStandardGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if ( options & PlainCaption ) {
        dialog->setPlainCaption( caption );
    }
#ifdef Q_WS_X11
    if ( parent == NULL && parent_id ) {
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
    }
#endif

    bool checkboxResult = false;

    createKMessageBox(dialog, QMessageBox::Information, text, strlist,
		dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                &checkboxResult, options);

    if (checkboxResult) {
        saveDontShowAgainContinue(dontShowAgainName);
    }
}

void KMessageBox::enableAllMessages()
{
   KConfig *config = KMessageBox_againConfig ? KMessageBox_againConfig : KGlobal::config().data();
   if (!config->hasGroup("Notification Messages")) {
      return;
   }

   KConfigGroup cg(config, "Notification Messages" );

   typedef QMap<QString, QString> configMap;

   const configMap map = cg.entryMap();

   configMap::ConstIterator it;
   for (it = map.begin(); it != map.end(); ++it) {
      cg.deleteEntry( it.key() );
   }
}

void KMessageBox::enableMessage(const QString &dontShowAgainName)
{
   KConfig *config = KMessageBox_againConfig ? KMessageBox_againConfig : KGlobal::config().data();
   if (!config->hasGroup("Notification Messages")) {
      return;
   }

   KConfigGroup cg( config, "Notification Messages" );

   cg.deleteEntry(dontShowAgainName);
   config->sync();
}

void KMessageBox::about(QWidget *parent, const QString &text,
                   const QString &caption, Options options)
{
    QString _caption = caption;
    if (_caption.isEmpty()) {
        _caption = i18n("About %1", KGlobal::caption());
    }

    KDialog *dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption( caption );
    dialog->setButtons( KDialog::Yes );
    dialog->setObjectName( "about" );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStandardGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if (qApp->windowIcon().isNull()) {
        QPixmap ret = QMessageBox::standardIcon(QMessageBox::Information);
        dialog->setWindowIcon(ret);
    }

    createKMessageBox(dialog, qApp->windowIcon(), text, QStringList(), QString(), 0, options);
    return;
}

int KMessageBox::messageBox( QWidget *parent, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const KGuiItem &buttonCancel,
                             const QString &dontShowAskAgainName, Options options )
{
    return messageBoxWId( parent ? parent->effectiveWinId() : 0, type, text, caption,
        buttonYes, buttonNo, buttonCancel, dontShowAskAgainName, options );
}

int KMessageBox::messageBoxWId( WId parent_id, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const KGuiItem &buttonCancel,
                             const QString &dontShow, Options options )
{
    switch (type) {
    case QuestionYesNo:
        return KMessageBox::questionYesNoWId( parent_id,
                                            text, caption, buttonYes, buttonNo, dontShow, options );
    case QuestionYesNoCancel:
        return KMessageBox::questionYesNoCancelWId( parent_id,
                                            text, caption, buttonYes, buttonNo, buttonCancel, dontShow, options );
    case WarningYesNo:
        return KMessageBox::warningYesNoWId( parent_id,
                                            text, caption, buttonYes, buttonNo, dontShow, options );
    case WarningContinueCancel:
        return KMessageBox::warningContinueCancelWId( parent_id,
                                            text, caption, KGuiItem(buttonYes.text()), buttonCancel, dontShow, options );
    case WarningYesNoCancel:
        return KMessageBox::warningYesNoCancelWId( parent_id,
                                            text, caption, buttonYes, buttonNo, buttonCancel, dontShow, options );
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

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption, Options options )
{
    return queuedMessageBoxWId( parent ? parent->effectiveWinId() : 0, type, text, caption, options );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption, Options options )
{
    KMessageBox_queue = true;
    (void) messageBoxWId(parent_id, type, text, caption, KStandardGuiItem::yes(),
                     KStandardGuiItem::no(), KStandardGuiItem::cancel(), QString(), options);
    KMessageBox_queue = false;
}

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption )
{
    return queuedMessageBoxWId( parent ? parent->effectiveWinId() : 0, type, text, caption );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption )
{
    KMessageBox_queue = true;
    (void) messageBoxWId(parent_id, type, text, caption);
    KMessageBox_queue = false;
}

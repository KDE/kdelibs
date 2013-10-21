/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
    Copyright 2012 David Faure <faure+bluesystems@kde.org>

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
#include "kmessagebox_p.h"

#include <QtCore/QPointer>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QDesktopWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextDocumentFragment>
#include <QTextBrowser>
#include <QWindow>

#include <qapplication.h>
#if 0
    // NOTE waiting for the notification framework plan
#include <knotification.h>
#endif
#include <ksqueezedtextlabel.h>

static const QString i18n(const char *a) { return QApplication::translate("KMessageBox", a); }
static const QString i18nc(const char *comment, const char *a) { return QApplication::translate("KMessageBox", a, comment); }

// Some i18n filters, that standard button texts are piped through
// (the new KGuiItem object with filtered text is created from the old one).

// i18n: Filter for the Yes-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_YES(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-yes", src.text().toUtf8().constData() ) );

// i18n: Filter for the No-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_NO(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-no", src.text().toUtf8().constData() ) );

// i18n: Filter for the Continue-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_CONTINUE(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-continue", src.text().toUtf8().constData() ) );

// i18n: Filter for the Cancel-button text in standard message dialogs,
// after the message caption/text have been translated.
#define I18N_FILTER_BUTTON_CANCEL(src, dst) \
    KGuiItem dst(src); \
    dst.setText( i18nc( "@action:button filter-cancel", src.text().toUtf8().constData() ) );

// i18n: Called after the button texts in standard message dialogs
// have been filtered by the messages above. Not visible to user.
#define I18N_POST_BUTTON_FILTER \
    i18nc( "@action:button post-filter", "." );

namespace KMessageBox {

/*
 * this static is used by the createKMessageBox function to enqueue dialogs
 * FIXME what should we do about this static?
 */
QDialogButtonBox::StandardButton KWIDGETSADDONS_EXPORT (*KMessageBox_exec_hook)(QDialog*) = 0;

static QIcon themedMessageBoxIcon(QMessageBox::Icon icon)
{
    QString icon_name;

    switch (icon) {
    case QMessageBox::NoIcon:
        return QIcon();
        break;
    case QMessageBox::Information:
        icon_name = QStringLiteral("dialog-information");
        break;
    case QMessageBox::Warning:
        icon_name = QStringLiteral("dialog-warning");
        break;
    case QMessageBox::Critical:
        icon_name = QStringLiteral("dialog-error");
        break;
    default:
        break;
    }

   QIcon ret = QIcon::fromTheme(icon_name);

   if (ret.isNull()) {
       return QMessageBox::standardIcon(icon);
   } else {
       return ret;
   }
}

static void applyOptions( QDialog* dialog, KMessageBox::Options options )
{
    if ( options & KMessageBox::WindowModal ) {
        dialog->setWindowModality( Qt::WindowModal );
    }
    dialog->setModal( true );
}

// This method has been copied from KWindowSystem to avoid depending on it
static void setMainWindow( QWidget* subWidget, WId mainWindowId )
{
    // Set the WA_NativeWindow attribute to force the creation of the QWindow.
    // Without this QWidget::windowHandle() returns 0.
    subWidget->setAttribute(Qt::WA_NativeWindow, true);
    QWindow *subWindow = subWidget->windowHandle();
    Q_ASSERT(subWindow);

    QWindow *mainWindow = QWindow::fromWinId(mainWindowId);
    // mainWindow is not the child of any object, so make sure it gets deleted at some point
    QObject::connect(subWidget, &QObject::destroyed, mainWindow, &QObject::deleteLater);
    subWindow->setTransientParent(mainWindow);
}

/**
 * Create a QDialog whose parent is a foreign window
 */
static QDialog *createWIdDialog(WId parent_id)
{
    QWidget* parent = QWidget::find(parent_id);
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    if (!parent && parent_id) {
        setMainWindow(dialog, parent_id);
    }
    return dialog;
}

class DialogButtonsHelper : public QObject
{
    Q_OBJECT
public:
    DialogButtonsHelper(QDialog *dialog, QDialogButtonBox *buttons)
        : QObject(dialog),
          m_dialog(dialog),
          m_buttons(buttons),
          m_details(0)
    {
        connect(m_buttons, SIGNAL(clicked(QAbstractButton*)),
                this, SLOT(onButtonClicked(QAbstractButton*)));
    }

    void setDetailsWidget(QWidget *widget)
    {
        m_details = widget;
    }

public Q_SLOTS:
    void onButtonClicked(QAbstractButton *button)
    {
        QDialogButtonBox::StandardButton code = m_buttons->standardButton(button);
        if (code != QDialogButtonBox::NoButton) {
            m_dialog->done(code);
        } else if (m_details && (button->objectName() == QStringLiteral("detailsButton"))) {
            button->setText(i18n("&Details") + (m_details->isVisible() ? QStringLiteral(" >>") : QStringLiteral(" <<")));
            m_details->setVisible(!m_details->isVisible());
        }
    }

private:
    QDialog * const m_dialog;
    QDialogButtonBox * const m_buttons;
    QWidget *m_details;
};

QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog, QDialogButtonBox *buttons, QMessageBox::Icon icon,
                             const QString &text, const QStringList &strlist,
                             const QString &ask, bool *checkboxReturn,
                             Options options, const QString &details)
{
    return createKMessageBox(dialog, buttons, themedMessageBoxIcon(icon), text, strlist,
                      ask, checkboxReturn, options, details, icon);
}


QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog, QDialogButtonBox *buttons, const QIcon &icon,
                             const QString &text, const QStringList &strlist,
                             const QString &ask, bool *checkboxReturn, Options options,
                             const QString &details, QMessageBox::Icon notifyType)
{
    DialogButtonsHelper *buttonsHelper = new DialogButtonsHelper(dialog, buttons);

    QWidget *mainWidget = new QWidget(dialog);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    const int spacingHint = mainWidget->style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    mainLayout->setSpacing(spacingHint * 2); // provide extra spacing
    mainLayout->setMargin(0);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(-1); // use default spacing
    mainLayout->addLayout(hLayout,5);

    QLabel *iconLabel = new QLabel(mainWidget);

    if (!icon.isNull()) {
        QStyleOption option;
        option.initFrom(mainWidget);
        iconLabel->setPixmap(icon.pixmap(mainWidget->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, mainWidget)));
    }

    QVBoxLayout *iconLayout = new QVBoxLayout();
    iconLayout->addStretch(1);
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch(5);

    hLayout->addLayout(iconLayout,0);
    hLayout->addSpacing(spacingHint);

    QLabel *messageLabel = new QLabel(text, mainWidget);
    messageLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
    Qt::TextInteractionFlags flags = Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard;
    if (options & KMessageBox::AllowLink) {
        flags |= Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    }
    messageLabel->setTextInteractionFlags(flags);

    QRect desktop = QApplication::desktop()->screenGeometry(dialog);
    bool usingSqueezedTextLabel = false;
    if (messageLabel->sizeHint().width() > desktop.width() * 0.5) {
        // enable automatic wrapping of messages which are longer than 50% of screen width
        messageLabel->setWordWrap(true);
        // use a squeezed label if text is still too wide
        usingSqueezedTextLabel = messageLabel->sizeHint().width() > desktop.width() * 0.85;
        if (usingSqueezedTextLabel) {
            delete messageLabel;
            messageLabel = new KSqueezedTextLabel(text, mainWidget);
            messageLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
            messageLabel->setTextInteractionFlags(flags);
        }
    }

    QPalette messagePal(messageLabel->palette());
    messagePal.setColor(QPalette::Window, Qt::transparent);
    messageLabel->setPalette(messagePal);


    bool usingScrollArea=desktop.height() / 3 < messageLabel->sizeHint().height();
    if (usingScrollArea)
    {
        QScrollArea* messageScrollArea = new QScrollArea(mainWidget);
        messageScrollArea->setWidget(messageLabel);
        messageScrollArea->setFrameShape(QFrame::NoFrame);
        messageScrollArea->setWidgetResizable(true);
        QPalette scrollPal(messageScrollArea->palette());
        scrollPal.setColor(QPalette::Window, Qt::transparent);
        messageScrollArea->viewport()->setPalette(scrollPal);
        hLayout->addWidget(messageScrollArea,5);
    }
    else
        hLayout->addWidget(messageLabel,5);


    const bool usingListWidget=!strlist.isEmpty();
    if (usingListWidget) {
        // enable automatic wrapping since the listwidget has already a good initial width
        messageLabel->setWordWrap(true);
        QListWidget *listWidget = new QListWidget(mainWidget);
        listWidget->addItems(strlist);

        QStyleOptionViewItem styleOption;
        styleOption.initFrom(listWidget);
        QFontMetrics fm(styleOption.font);
        int w = listWidget->width();
        Q_FOREACH(const QString &str, strlist) {
            w = qMax(w, fm.width(str));
        }
        const int borderWidth = listWidget->width() - listWidget->viewport()->width() + listWidget->verticalScrollBar()->height();
        w += borderWidth;
        if (w > desktop.width() * 0.85) { // limit listWidget size to 85% of screen width
            w = qRound(desktop.width() * 0.85);
        }
        listWidget->setMinimumWidth(w);

        mainLayout->addWidget(listWidget,usingScrollArea?10:50);
        listWidget->setSelectionMode(QListWidget::NoSelection);
        messageLabel->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
    }
    else if (!usingScrollArea)
        mainLayout->addStretch(15);


    QPointer<QCheckBox> checkbox = 0;
    if (!ask.isEmpty()) {
        checkbox = new QCheckBox(ask, mainWidget);
        mainLayout->addWidget(checkbox);
        if (checkboxReturn) {
            checkbox->setChecked(*checkboxReturn);
        }
    }

    QVBoxLayout *topLayout = new QVBoxLayout;
    dialog->setLayout(topLayout);
    topLayout->addWidget(mainWidget);

    if (!details.isEmpty()) {
        QGroupBox *detailsGroup = new QGroupBox(i18n("Details"));
        QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);
        if (details.length() < 512) {
            QLabel *detailsLabel = new QLabel(details);
            detailsLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
            Qt::TextInteractionFlags flags = Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard;
            if ( options & KMessageBox::AllowLink )
                flags |= Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;;
            detailsLabel->setTextInteractionFlags(flags);
            detailsLabel->setWordWrap(true);
            detailsLayout->addWidget(detailsLabel,50);
        } else {
            QTextBrowser *detailTextEdit = new QTextBrowser();
            detailTextEdit->setText(details);
            detailTextEdit->setReadOnly(true);
            detailTextEdit->setMinimumHeight(detailTextEdit->fontMetrics().lineSpacing() * 11);
            detailsLayout->addWidget(detailTextEdit,50);
        }
        if (!usingListWidget)
            mainLayout->setStretchFactor(hLayout,10);
        topLayout->addWidget(detailsGroup);
        buttonsHelper->setDetailsWidget(detailsGroup);
        detailsGroup->hide();
    }

    buttons->setParent(dialog);
    topLayout->addWidget(buttons);

    if (!usingListWidget && !usingScrollArea && !usingSqueezedTextLabel && details.isEmpty())
        dialog->setFixedSize(dialog->sizeHint() + QSize( 10, 10 ));
    else if (!details.isEmpty() && dialog->minimumHeight()<iconLabel->sizeHint().height()*2)//strange bug...
    {
        if (!usingScrollArea)
            dialog->setMinimumSize(300,qMax(150,qMax(iconLabel->sizeHint().height(),messageLabel->sizeHint().height())));
        else
            dialog->setMinimumSize(300,qMax(150,iconLabel->sizeHint().height()));
    }


    if ((options & KMessageBox::Dangerous)) {
        QPushButton *cancelButton = buttons->button(QDialogButtonBox::Cancel);
        QPushButton *noButton = buttons->button(QDialogButtonBox::No);

        if (cancelButton && cancelButton->isEnabled()) {
            cancelButton->setDefault(true);
            cancelButton->setFocus();
        } else if (noButton && noButton->isEnabled()) {
            noButton->setDefault(true);
            noButton->setFocus();
        }
    }

#ifndef Q_OS_WIN // FIXME problems with KNotify on Windows
    if ((options & KMessageBox::Notify)) {
        QString message = text;
        if (!strlist.isEmpty()) {
            message += QLatin1Char('\n') + strlist.join(QLatin1Char('\n'));
        }
        notifyInterface()->sendNotification(notifyType, message, dialog->topLevelWidget());
    }
#endif

    if (KMessageBox_exec_hook) {
        return KMessageBox_exec_hook(dialog);
    }

    if ((options & KMessageBox::NoExec)) {
        return QDialogButtonBox::NoButton; // We have to return something.
    }

    // We use a QPointer because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the QPointer will reset to 0.
    QPointer<QDialog> guardedDialog = dialog;

    const QDialogButtonBox::StandardButton result = QDialogButtonBox::StandardButton(guardedDialog->exec());
    if (checkbox && checkboxReturn) {
        *checkboxReturn = checkbox->isChecked();
    }

    delete guardedDialog;
    return result;
}

ButtonCode questionYesNo(QWidget *parent, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    return questionYesNoList(parent, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

bool shouldBeShownYesNo(const QString &dontShowAgainName,
                                     ButtonCode &result)
{
    if ( dontShowAgainName.isEmpty() ) {
        return true;
    }
    return dontAskAgainInterface()->shouldBeShownYesNo(dontShowAgainName, result);
}

bool shouldBeShownContinue(const QString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) {
        return true;
    }
    return dontAskAgainInterface()->shouldBeShownContinue(dontShowAgainName);
}

void saveDontShowAgainYesNo(const QString &dontShowAgainName,
                                         ButtonCode result)
{
    if ( dontShowAgainName.isEmpty() ) {
        return;
    }
    dontAskAgainInterface()->saveDontShowAgainYesNo(dontShowAgainName, result);
}

void saveDontShowAgainContinue(const QString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) {
        return;
    }
    dontAskAgainInterface()->saveDontShowAgainContinue(dontShowAgainName);
}

void enableAllMessages()
{
    dontAskAgainInterface()->enableAllMessages();
}

void enableMessage(const QString &dontShowAgainName)
{
    dontAskAgainInterface()->enableMessage(dontShowAgainName);
}

void setDontShowAgainConfig(KConfig* cfg)
{
    dontAskAgainInterface()->setConfig(cfg);
}

static ButtonCode questionYesNoListInternal(QDialog *dialog, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes_,
                           const KGuiItem &buttonNo_,
                           const QString &dontAskAgainName,
                           Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) ) {
        delete dialog;
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_POST_BUTTON_FILTER

    dialog->setWindowTitle(caption.isEmpty() ? i18n("Question") : caption);
    dialog->setObjectName(QStringLiteral("questionYesNo"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);

    applyOptions( dialog, options );

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog, buttonBox, QMessageBox::Information, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==QDialogButtonBox::Yes ? Yes : No);

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

ButtonCode questionYesNoList(QWidget *parent, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    return questionYesNoListInternal(new QDialog(parent), text, strlist, caption, buttonYes, buttonNo,
        dontAskAgainName, options);
}

static ButtonCode questionYesNoCancelInternal(QDialog *dialog,
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
        delete dialog;
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_FILTER_BUTTON_CANCEL(buttonCancel_, buttonCancel)
    I18N_POST_BUTTON_FILTER

    dialog->setWindowTitle(caption.isEmpty() ? i18n("Question") : caption);
    dialog->setObjectName(QStringLiteral("questionYesNoCancel"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), buttonCancel);

    applyOptions( dialog, options );

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog, buttonBox, QMessageBox::Information,
                       text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result == QDialogButtonBox::Yes ) {
        res = Yes;
    } else if ( result == QDialogButtonBox::No ) {
        res = No;
    } else {
        return Cancel;
    }

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

ButtonCode questionYesNoCancel(QWidget *parent,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const KGuiItem &buttonCancel,
                          const QString &dontAskAgainName,
                          Options options)
{
    return questionYesNoCancelInternal(new QDialog(parent), text, caption, buttonYes, buttonNo, buttonCancel,
        dontAskAgainName, options);
}

ButtonCode warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
    return warningYesNoList(parent, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

static ButtonCode warningYesNoListInternal(QDialog *dialog, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes_,
                              const KGuiItem &buttonNo_,
                              const QString &dontAskAgainName,
                              Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) ) {
        delete dialog;
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_POST_BUTTON_FILTER

    dialog->setWindowTitle(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setObjectName(QStringLiteral("warningYesNoList"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);

    applyOptions( dialog, options );

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==QDialogButtonBox::Yes ? Yes : No);

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

ButtonCode warningYesNoList(QWidget *parent, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const QString &dontAskAgainName,
                              Options options)
{
    return warningYesNoListInternal(new QDialog(parent), text, strlist, caption, buttonYes, buttonNo,
        dontAskAgainName, options);
}

ButtonCode warningContinueCancel(QWidget *parent,
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

static ButtonCode warningContinueCancelListInternal(QDialog *dialog, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue_,
                             const KGuiItem &buttonCancel_,
                             const QString &dontAskAgainName,
                             Options options)
{
    if (!shouldBeShownContinue(dontAskAgainName)) {
        delete dialog;
        return Continue;
    }

    I18N_FILTER_BUTTON_CONTINUE(buttonContinue_, buttonContinue)
    I18N_FILTER_BUTTON_CANCEL(buttonCancel_, buttonCancel)
    I18N_POST_BUTTON_FILTER

    dialog->setWindowTitle(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setObjectName(QStringLiteral("warningYesNo"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonContinue);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonCancel);

    applyOptions( dialog, options );

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result != QDialogButtonBox::Yes ) {
        return Cancel;
    }
    if (checkboxResult) {
        saveDontShowAgainContinue(dontAskAgainName);
    }
    return Continue;
}

ButtonCode warningContinueCancelList(QWidget *parent, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue,
                             const KGuiItem &buttonCancel,
                             const QString &dontAskAgainName,
                             Options options)
{
    return warningContinueCancelListInternal(new QDialog(parent), text, strlist, caption, buttonContinue, buttonCancel,
        dontAskAgainName, options);
}

ButtonCode warningYesNoCancel(QWidget *parent, const QString &text,
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

static ButtonCode warningYesNoCancelListInternal(QDialog *dialog, const QString &text,
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
        delete dialog;
        return res;
    }

    I18N_FILTER_BUTTON_YES(buttonYes_, buttonYes)
    I18N_FILTER_BUTTON_NO(buttonNo_, buttonNo)
    I18N_FILTER_BUTTON_CANCEL(buttonCancel_, buttonCancel)
    I18N_POST_BUTTON_FILTER

    dialog->setWindowTitle(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setObjectName(QStringLiteral("warningYesNoCancel"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), buttonCancel);

    applyOptions( dialog, options );

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result == QDialogButtonBox::Yes ) {
        res = Yes;
    } else if ( result == QDialogButtonBox::No ) {
        res = No;
    } else {
        return Cancel;
    }

    if (checkboxResult) {
        saveDontShowAgainYesNo(dontAskAgainName, res);
    }
    return res;
}

ButtonCode warningYesNoCancelList(QWidget *parent, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const KGuiItem &buttonCancel,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    return warningYesNoCancelListInternal(new QDialog(parent), text, strlist, caption, buttonYes, buttonNo, buttonCancel,
        dontAskAgainName, options);
}

void error(QWidget *parent,  const QString &text,
                   const QString &caption, Options options)
{
    return errorList( parent, text, QStringList(), caption, options );
}

static void errorListInternal(QDialog *dialog, const QString &text, const QStringList &strlist,
                       const QString &caption, Options options)
{
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Error") : caption);
    dialog->setObjectName(QStringLiteral("error"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );

    createKMessageBox(dialog, buttonBox, QMessageBox::Critical, text, strlist, QString(), 0, options);
}

void errorList(QWidget *parent, const QString &text, const QStringList &strlist,
                       const QString &caption, Options options)
{
    errorListInternal(new QDialog(parent), text, strlist, caption, options);
}

static void detailedErrorInternal(QDialog *dialog, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Error") : caption);
    dialog->setObjectName(QStringLiteral("error"));

    QPushButton *detailsButton = new QPushButton;
    detailsButton->setObjectName(QStringLiteral("detailsButton"));
    detailsButton->setText(i18n("&Details") + QStringLiteral(" >>"));
    detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->addButton(detailsButton, QDialogButtonBox::HelpRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    applyOptions( dialog, options );

    createKMessageBox(dialog, buttonBox, QMessageBox::Critical, text, QStringList(), QString(), 0, options, details);
}

void detailedError(QWidget *parent, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    detailedErrorInternal(new QDialog(parent), text, details, caption, options);
}

static void sorryInternal(QDialog *dialog, const QString &text,
                   const QString &caption, Options options)
{
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Sorry") : caption);
    dialog->setObjectName(QStringLiteral("sorry"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );

    createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, QStringList(), QString(), 0, options);
}

void sorry(QWidget *parent, const QString &text,
                   const QString &caption, Options options)
{
    sorryInternal(new QDialog(parent), text, caption, options);
}

static void detailedSorryInternal(QDialog *dialog, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Sorry") : caption);
    dialog->setObjectName(QStringLiteral("sorry"));

    QPushButton *detailsButton = new QPushButton;
    detailsButton->setObjectName(QStringLiteral("detailsButton"));
    detailsButton->setText(i18n("&Details") + QStringLiteral(" >>"));
    detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->addButton(detailsButton, QDialogButtonBox::HelpRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    applyOptions( dialog, options );

    createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, QStringList(), QString(), 0, options, details);
}

void detailedSorry(QWidget *parent, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    detailedSorryInternal(new QDialog(parent), text, details, caption, options);
}

void information(QWidget *parent,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, Options options)
{
    informationList(parent, text, QStringList(), caption, dontShowAgainName, options);
}

static void informationListInternal(QDialog *dialog, const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    if ( !shouldBeShownContinue(dontShowAgainName) ) {
        delete dialog;
        return;
    }

    dialog->setWindowTitle(caption.isEmpty() ? i18n("Information") : caption);
    dialog->setObjectName(QStringLiteral("information"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );

    bool checkboxResult = false;

    createKMessageBox(dialog, buttonBox, QMessageBox::Information, text, strlist,
        dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                &checkboxResult, options);

    if (checkboxResult) {
        saveDontShowAgainContinue(dontShowAgainName);
    }
}

void informationList(QWidget *parent,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    informationListInternal(new QDialog(parent), text, strlist, caption, dontShowAgainName, options);
}

void about(QWidget *parent, const QString &text,
                   const QString &caption, Options options)
{
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    if (!caption.isEmpty())
        dialog->setWindowTitle(caption);
    dialog->setObjectName(QStringLiteral("about"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );

    if (qApp->windowIcon().isNull()) {
        QPixmap ret = QMessageBox::standardIcon(QMessageBox::Information);
        dialog->setWindowIcon(ret);
    }

    createKMessageBox(dialog, buttonBox, qApp->windowIcon(), text, QStringList(), QString(), 0, options);
}

static ButtonCode messageBoxInternal(QDialog *dialog, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const KGuiItem &buttonCancel,
                             const QString &dontShow, Options options)
{
    switch (type) {
    case QuestionYesNo:
        return questionYesNoListInternal(dialog, text, QStringList(), caption,
            buttonYes, buttonNo, dontShow, options);
    case QuestionYesNoCancel:
        return questionYesNoCancelInternal(dialog, text, caption,
            buttonYes, buttonNo, buttonCancel, dontShow, options);
    case WarningYesNo:
        return warningYesNoListInternal(dialog, text, QStringList(), caption,
            buttonYes, buttonNo, dontShow, options);
    case WarningContinueCancel:
        return warningContinueCancelListInternal(dialog, text, QStringList(), caption,
            KGuiItem(buttonYes.text()), buttonCancel, dontShow, options);
    case WarningYesNoCancel:
        return warningYesNoCancelListInternal(dialog, text, QStringList(), caption,
            buttonYes, buttonNo, buttonCancel, dontShow, options);
    case Information:
        informationListInternal(dialog, text, QStringList(), caption, dontShow, options);
        return KMessageBox::Ok;

    case Error:
        errorListInternal(dialog, text, QStringList(), caption, options);
        return KMessageBox::Ok;

    case Sorry:
        sorryInternal(dialog, text, caption, options);
        return KMessageBox::Ok;
    }
    return KMessageBox::Cancel;
}

ButtonCode messageBox( QWidget *parent, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const KGuiItem &buttonCancel,
                             const QString &dontShow, Options options )
{
    return messageBoxInternal(new QDialog(parent), type, text, caption,
        buttonYes, buttonNo, buttonCancel, dontShow, options);
}

ButtonCode questionYesNoWId(WId parent_id, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    return questionYesNoListWId(parent_id, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

ButtonCode questionYesNoListWId(WId parent_id, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    return questionYesNoListInternal(createWIdDialog(parent_id), text, strlist, caption, buttonYes, buttonNo,
        dontAskAgainName, options);
}

ButtonCode questionYesNoCancelWId(WId parent_id,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const KGuiItem &buttonCancel,
                          const QString &dontAskAgainName,
                          Options options)
{
    return questionYesNoCancelInternal(createWIdDialog(parent_id), text, caption, buttonYes, buttonNo, buttonCancel,
        dontAskAgainName, options);
}

ButtonCode warningYesNoWId(WId parent_id, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
    return warningYesNoListWId(parent_id, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

ButtonCode warningYesNoListWId(WId parent_id, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const QString &dontAskAgainName,
                              Options options)
{
    return warningYesNoListInternal(createWIdDialog(parent_id), text, strlist, caption, buttonYes, buttonNo,
        dontAskAgainName, options);
}

ButtonCode warningContinueCancelWId(WId parent_id,
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

ButtonCode warningContinueCancelListWId(WId parent_id, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue,
                             const KGuiItem &buttonCancel,
                             const QString &dontAskAgainName,
                             Options options)
{
    return warningContinueCancelListInternal(createWIdDialog(parent_id), text, strlist, caption, buttonContinue, buttonCancel,
        dontAskAgainName, options);
}

ButtonCode warningYesNoCancelWId(WId parent_id, const QString &text,
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

ButtonCode warningYesNoCancelListWId(WId parent_id, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const KGuiItem &buttonCancel,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    return warningYesNoCancelList(createWIdDialog(parent_id), text, strlist, caption, buttonYes, buttonNo, buttonCancel,
        dontAskAgainName, options);
}

void errorWId(WId parent_id, const QString &text,
                      const QString &caption, Options options)
{
    errorListWId( parent_id, text, QStringList(), caption, options );
}

void errorListWId(WId parent_id,  const QString &text, const QStringList &strlist,
                   const QString &caption, Options options)
{
    errorListInternal(createWIdDialog(parent_id), text, strlist, caption, options);
}

void detailedErrorWId(WId parent_id, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    detailedErrorInternal(createWIdDialog(parent_id), text, details, caption, options);
}

void sorryWId(WId parent_id, const QString &text,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Sorry") : caption);
    dialog->setObjectName(QStringLiteral("sorry"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        setMainWindow( dialog, parent_id );
    }

    createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, QStringList(), QString(), 0, options);
}

void detailedSorryWId(WId parent_id, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    detailedSorryInternal(createWIdDialog(parent_id), text, details, caption, options);
}

void informationWId(WId parent_id,const QString &text,
             const QString &caption, const QString &dontShowAgainName, Options options)
{
    informationListWId(parent_id, text, QStringList(), caption, dontShowAgainName, options);
}

void informationListWId(WId parent_id,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    informationListInternal(createWIdDialog(parent_id), text, strlist, caption, dontShowAgainName, options);
}

ButtonCode messageBoxWId(WId parent_id, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const KGuiItem &buttonCancel,
                             const QString &dontShow, Options options)
{
    return messageBoxInternal(createWIdDialog(parent_id), type, text, caption,
        buttonYes, buttonNo, buttonCancel, dontShow, options);
}

} // namespace

#include "kmessagebox.moc"

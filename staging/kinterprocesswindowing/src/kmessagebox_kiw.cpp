
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

#include "kmessagebox_kiw.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <kwindowsystem.h>
#include <klocalizedstring.h>

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

namespace KMessageBox {

/*
 * this static is used by the createKMessageBox function to enqueue dialogs
 * it's exported by the kwidgets framework
 * FIXME what should we do about this static?
 */
extern bool KMessageBox_queue;

/** @note This function has a duplicate in kmessagebox.cpp 
 *  FIXME What should be done about this?
 **/
static void applyOptions(QDialog *dialog, KMessageBox::Options options)
{
    if ( options & KMessageBox::WindowModal ) {
        dialog->setWindowModality( Qt::WindowModal );
    }
    dialog->setModal( true );
}

int questionYesNoWId(WId parent_id, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    return questionYesNoListWId(parent_id, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

int questionYesNoListWId(WId parent_id, const QString &text,
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
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Question") : caption);
    dialog->setObjectName( "questionYesNo" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

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

int questionYesNoCancelWId(WId parent_id,
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
    QDialog *dialog= new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Question") : caption);
    dialog->setObjectName( "questionYesNoCancel" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), buttonCancel);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

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

int warningYesNoWId(WId parent_id, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
    return warningYesNoListWId(parent_id, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int warningYesNoListWId(WId parent_id, const QString &text,
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
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setObjectName( "warningYesNoList" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

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

int warningContinueCancelWId(WId parent_id,
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

int warningContinueCancelListWId(WId parent_id, const QString &text,
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
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setObjectName( "warningYesNo" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonContinue);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonCancel);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

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

int warningYesNoCancelWId(WId parent_id, const QString &text,
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

int warningYesNoCancelListWId(WId parent_id, const QString &text,
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
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setObjectName( "warningYesNoCancel" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonYes);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonNo);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), buttonCancel);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

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

void errorWId(WId parent_id, const QString &text,
                      const QString &caption, Options options)
{
    errorListWId( parent_id, text, QStringList(), caption, options );
}

void errorListWId(WId parent_id,  const QString &text, const QStringList &strlist,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Error") : caption);
    dialog->setObjectName( "error" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

    createKMessageBox(dialog, buttonBox, QMessageBox::Critical, text, strlist, QString(), 0, options);
}

void detailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Error") : caption);
    dialog->setObjectName( "error" );

    QPushButton *detailsButton = new QPushButton;
    detailsButton->setObjectName("detailsButton");
    detailsButton->setText(i18n("&Details") + " >>");
    detailsButton->setIcon(QIcon::fromTheme("help-about"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->addButton(detailsButton, QDialogButtonBox::HelpRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

    createKMessageBox(dialog, buttonBox, QMessageBox::Critical, text, QStringList(), QString(), 0, options, details);
}

void sorryWId(WId parent_id, const QString &text,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Sorry") : caption);
    dialog->setObjectName( "sorry" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

    createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, QStringList(), QString(), 0, options);
}

void detailedSorryWId(WId parent_id, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Sorry") : caption);
    dialog->setObjectName( "sorry" );

    QPushButton *detailsButton = new QPushButton;
    detailsButton->setObjectName("detailsButton");
    detailsButton->setText(i18n("&Details") + " >>");
    detailsButton->setIcon(QIcon::fromTheme("help-about"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->addButton(detailsButton, QDialogButtonBox::HelpRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

    createKMessageBox(dialog, buttonBox, QMessageBox::Warning, text, QStringList(), QString(), 0, options, details);
}

void informationWId(WId parent_id,const QString &text,
             const QString &caption, const QString &dontShowAgainName, Options options)
{
    informationListWId(parent_id, text, QStringList(), caption, dontShowAgainName, options);
}

void informationListWId(WId parent_id,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    if ( !shouldBeShownContinue(dontShowAgainName) ) {
        return;
    }

    QWidget* parent = QWidget::find( parent_id );
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Information") : caption);
    dialog->setObjectName( "information" );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions( dialog, options );
    if ( parent == NULL && parent_id ) {
        KWindowSystem::setMainWindow( dialog, parent_id );
    }

    bool checkboxResult = false;

    createKMessageBox(dialog, buttonBox, QMessageBox::Information, text, strlist,
        dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                &checkboxResult, options);

    if (checkboxResult) {
        saveDontShowAgainContinue(dontShowAgainName);
    }
}

int messageBoxWId( WId parent_id, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const KGuiItem &buttonCancel,
                             const QString &dontShow, Options options )
{
    switch (type) {
    case QuestionYesNo:
        return questionYesNoWId( parent_id,
                                            text, caption, buttonYes, buttonNo, dontShow, options );
    case QuestionYesNoCancel:
        return questionYesNoCancelWId( parent_id,
                                            text, caption, buttonYes, buttonNo, buttonCancel, dontShow, options );
    case WarningYesNo:
        return warningYesNoWId( parent_id,
                                            text, caption, buttonYes, buttonNo, dontShow, options );
    case WarningContinueCancel:
        return warningContinueCancelWId( parent_id,
                                            text, caption, KGuiItem(buttonYes.text()), buttonCancel, dontShow, options );
    case WarningYesNoCancel:
        return warningYesNoCancelWId( parent_id,
                                            text, caption, buttonYes, buttonNo, buttonCancel, dontShow, options );
    case Information:
        informationWId( parent_id,
                                    text, caption, dontShow, options );
        return KMessageBox::Ok;

    case Error:
        errorWId( parent_id, text, caption, options );
        return KMessageBox::Ok;

    case Sorry:
        sorryWId( parent_id, text, caption, options );
        return KMessageBox::Ok;
    }
    return KMessageBox::Cancel;
}


} // namespace

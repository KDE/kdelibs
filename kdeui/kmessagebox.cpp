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
/*
 * $Id$
 *
 */
#include <qcheckbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qvbox.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlayout.h>

 /**
  * Easy MessageBox Dialog.
  *
  * Provides convenience functions for some i18n'ed standard dialogs,
  * as well as audible notification via @ref KNotifyClient
  *
  * @author Waldo Bastian (bastian@kde.org)
  * @version $Id$
  */

#ifdef __GNUC__
#warning FIXME - Implement Notification
#endif

int
KMessageBox::questionYesNo(QWidget *parent, const QString &text,
                           const QString &caption,
                           const QString &buttonYes,
                           const QString &buttonNo, bool /*notify*/)
{
    return questionYesNoList( parent, text, QStringList(), caption,
	       	              buttonYes, buttonNo );
}

int
KMessageBox::questionYesNoList(QWidget *parent, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const QString &buttonYes,
                               const QString &buttonNo, bool /*notify*/)
{
    KDialogBase dialog(caption.isEmpty() ? i18n("Question") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "questionYesNo", true, true,
                       buttonYes, buttonNo);

    QVBox *topcontents = new QVBox (&dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint()*2);
    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents );
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Information, kapp->style().guiStyle()));
    lay->add( label1 );
    lay->add( new QLabel( text, contents ) );
    lay->addStretch(1);

    if (!strlist.isEmpty())
    {
      KListBox *listbox=new KListBox( topcontents );
      listbox->insertStringList( strlist );
    }

    dialog.setMainWidget(topcontents);
    dialog.enableButtonSeparator(false);

    int result = dialog.exec();

    switch( result )
    {
      case KDialogBase::Yes:
         return Yes;

      case KDialogBase::No:
         return No;

      default: // Huh?
         break;
    }

    return Yes; // Default
}

int
KMessageBox::warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const QString &buttonYes,
                          const QString &buttonNo, bool /*notify*/)
{
    KDialogBase dialog(caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::No, KDialogBase::No,
                       parent, "warningYesNo", true, true,
                       buttonYes, buttonNo);

    QWidget *contents = new QWidget(&dialog);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);
    lay->setMargin(KDialog::marginHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents);
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()));
    lay->add( label1 );
    lay->add( new QLabel(text, contents) );
    lay->addStretch(1);

    dialog.setMainWidget(contents);
    dialog.enableButtonSeparator(false);

    int result = dialog.exec();

    switch( result )
    {
      case KDialogBase::Yes:
         return Yes;

      case KDialogBase::No:
         return No;

      default: // Huh?
         break;
    }

    return No; // Default
}

int
KMessageBox::warningContinueCancel(QWidget *parent, const QString &text,
                          const QString &caption,
                                   const QString &buttonContinue, bool /*notify*/)
{
    KDialogBase dialog(caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "warningYesNo", true, true,
                       buttonContinue, i18n("&Cancel"));

    QWidget *contents = new QWidget(&dialog);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);
    lay->setMargin(KDialog::marginHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents);
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()));
    lay->add( label1 );
    lay->add( new QLabel(text, contents) );
    lay->addStretch(1);

    dialog.setMainWidget(contents);
    dialog.enableButtonSeparator(false);

    int result = dialog.exec();

    switch( result )
    {
      case KDialogBase::Yes:
         return Continue;

      case KDialogBase::No:
         return Cancel;

      default: // Huh?
         break;
    }

    return Cancel; // Default
}


int
KMessageBox::warningYesNoCancel(QWidget *parent, const QString &text,
                                const QString &caption,
                                const QString &buttonYes,
                                const QString &buttonNo, bool /*notify*/)
{
    KDialogBase dialog(caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                       KDialogBase::Yes, KDialogBase::Cancel,
                       parent, "warningYesNoCancel", true, true,
                       buttonYes, buttonNo);

    QWidget *contents = new QWidget(&dialog);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);
    lay->setMargin(KDialog::marginHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents);
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()));
    lay->add( label1 );
    lay->add( new QLabel(text, contents) );
    lay->addStretch(1);

    dialog.setMainWidget(contents);
    dialog.enableButtonSeparator(false);

    int result = dialog.exec();

    switch( result )
    {
      case KDialogBase::Yes:
         return Yes;

      case KDialogBase::No:
         return No;

      case KDialogBase::Cancel:
         return Cancel;

      default: // Huh?
         break;
    }

    return Cancel; // Default
}

void
KMessageBox::error(QWidget *parent,  const QString &text,
                   const QString &caption, bool /*notify*/)
{
    KDialogBase dialog(caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       i18n("&OK"));

    QWidget *contents = new QWidget(&dialog);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);
    lay->setMargin(KDialog::marginHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents);
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Critical, kapp->style().guiStyle()));
    lay->add( label1 );
    lay->add( new QLabel(text, contents) );
    lay->addStretch(1);

    dialog.setMainWidget(contents);
    dialog.enableButtonSeparator(false);

    dialog.exec();
}

void
KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption, bool /*notify*/)
{
    KDialogBase dialog(caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       i18n("&OK"));

    QWidget *contents = new QWidget(&dialog);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);
    lay->setMargin(KDialog::marginHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents);
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()));
    lay->add( label1 );
    lay->add( new QLabel(text, contents) );
    lay->addStretch(1);

    dialog.setMainWidget(contents);
    dialog.enableButtonSeparator(false);

    dialog.exec();
}

void
KMessageBox::information(QWidget *parent,const QString &text,
                         const QString &caption, const QString &dontShowAgainName, bool /*notify*/)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    bool showMsg = true;
    QCheckBox *checkbox = 0;

    if (!dontShowAgainName.isEmpty())
    {
       config = kapp->config();
       KConfigGroupSaver saver( config, grpNotifMsgs );
       showMsg = config->readBoolEntry( dontShowAgainName, true);
       if (!showMsg)
       {
          return;
       }
    }

    KDialogBase dialog(caption.isEmpty() ? i18n("Information") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "information", true, true,
                       i18n("&OK"));

    QVBox *contents = new QVBox(&dialog);
    contents->setSpacing(KDialog::spacingHint()*2);
    contents->setMargin(KDialog::marginHint()*2);

    QWidget *info = new QWidget(contents);
    QHBoxLayout * lay = new QHBoxLayout(info);
    lay->setSpacing(KDialog::spacingHint()*2);
    lay->setMargin(0);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( info);
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Information, kapp->style().guiStyle()));
    lay->add( label1 );
    lay->add( new QLabel(text, info) );
    lay->addStretch(1);

    if (!dontShowAgainName.isEmpty())
    {
       checkbox = new QCheckBox(i18n("Do not show this message again"), contents);
    }

    dialog.setMainWidget(contents);
    dialog.enableButtonSeparator(false);

    dialog.exec();

    if (!dontShowAgainName.isEmpty())
    {
       showMsg = !checkbox->isChecked();
       if (!showMsg)
       {
          KConfigGroupSaver saver( config, grpNotifMsgs );
          config->writeEntry( dontShowAgainName, showMsg);
       }
       config->sync();
    }

    return;
}

void
KMessageBox::enableAllMessages()
{
   KConfig *config = kapp->config();
   QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
   if (!config->hasGroup(grpNotifMsgs))
      return;

   KConfigGroupSaver saver( config, grpNotifMsgs );

   typedef QMap<QString, QString> configMap;

   configMap map = config->entryMap(grpNotifMsgs);

   configMap::Iterator it;
   for (it = map.begin(); it != map.end(); ++it)
      config->writeEntry( it.key(), true);
   config->sync();
}

void
KMessageBox::about(QWidget *parent, const QString &text,
                   const QString &caption, bool /*notify*/)
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


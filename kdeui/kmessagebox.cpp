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
#include <qvgroupbox.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <kstdguiitem.h>

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

static bool KMessageBox_queue = false;

static int createKMessageBox(KDialogBase *dialog, QMessageBox::Icon icon, const QString &text, const QStringList &strlist, const QString &ask, bool *checkboxReturn, const QString &details=QString::null)
{
    QVBox *topcontents = new QVBox (dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint()*2);

    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents);
#if QT_VERSION < 300
    label1->setPixmap(QMessageBox::standardIcon(icon, kapp->style().guiStyle()));
#else
    label1->setPixmap(QMessageBox::standardIcon(icon));
#endif
    lay->add( label1 );
    QLabel *label2 = new QLabel( text, contents);
    label2->setAlignment( Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak );
    label2->setMinimumSize(label2->sizeHint());
    lay->add( label2 );
    lay->addStretch(1);

    QSize extraSize = QSize(50,30);
    if (!strlist.isEmpty())
    {
       KListBox *listbox=new KListBox( topcontents );
       listbox->insertStringList( strlist );
    }

    QCheckBox *checkbox = 0;
    if (!ask.isEmpty())
    {
       checkbox = new QCheckBox(ask, topcontents);
       extraSize = QSize(50,0);
    }

    if (!details.isEmpty())
    {
       QVGroupBox *detailsGroup = new QVGroupBox( i18n("Details:"), dialog);
       QLabel *label3 = new QLabel(details, detailsGroup);
       label3->setMinimumSize(label3->sizeHint());
       dialog->setDetailsWidget(detailsGroup);
    }

    dialog->setMainWidget(topcontents);
    dialog->enableButtonSeparator(false);
    dialog->incInitialSize( extraSize );

    if (KMessageBox_queue)
    {
       KDialogQueue::queueDialog(dialog);
       return KMessageBox::Cancel; // We have to return something.
    }

    int result = dialog->exec();
    if (checkbox && checkboxReturn)
       *checkboxReturn = checkbox->isChecked();
    delete dialog;
    return result;
}

int
KMessageBox::questionYesNo(QWidget *parent, const QString &text,
                           const QString &caption,
                           const QString &buttonYes,
                           const QString &buttonNo,
                           const QString &dontAskAgainName,
                           bool notify)
{
   return questionYesNoList(parent, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, notify);
}


int
KMessageBox::questionYesNoList(QWidget *parent, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const QString &buttonYes,
                           const QString &buttonNo,
                           const QString &dontAskAgainName,
                           bool /*notify*/)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = kapp->config();
       KConfigGroupSaver saver( config, grpNotifMsgs );
       QString dontAsk = config->readEntry( dontAskAgainName).lower();
       if (dontAsk == "yes")
       {
          return Yes;
       }
       if (dontAsk == "no")
       {
          return No;
       }
    }
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Question") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "questionYesNo", true, true,
                       buttonYes, buttonNo);

    bool checkboxResult;
    int result = createKMessageBox(dialog, QMessageBox::Information, text, strlist,
                       dontAskAgainName.isEmpty() ? QString::null : i18n("Do not ask again"),
                       &checkboxResult);

    switch( result )
    {
      case KDialogBase::Yes:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "Yes");
            }
            config->sync();
         }
         return Yes;

      case KDialogBase::No:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "No");
            }
            config->sync();
         }
         return No;

      default: // Huh?
         break;
    }

    return Yes; // Default
}
int
KMessageBox::questionYesNoCancel(QWidget *parent,
                          const QString &text,
                          const QString &caption,
                          const QString &buttonYes,
                          const QString &buttonNo,
                          const QString &dontAskAgainName,
                          bool /*notify*/)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = kapp->config();
       KConfigGroupSaver saver( config, grpNotifMsgs );
       QString dontAsk = config->readEntry( dontAskAgainName).lower();
       if (dontAsk == "yes")
       {
          return Yes;
       }
       if (dontAsk == "no")
       {
          return No;
       }
    }
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Question") : caption,
                       KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                       KDialogBase::Yes, KDialogBase::Cancel,
                       parent, "questionYesNoCancel", true, true,
                       buttonYes, buttonNo);

    bool checkboxResult;
    int result = createKMessageBox(dialog, QMessageBox::Information
, text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString::null : i18n("Do not ask again"),
                       &checkboxResult);

    switch( result )
    {
      case KDialogBase::Yes:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "Yes");
            }
            config->sync();
         }
         return Yes;

      case KDialogBase::No:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "No");
            }
            config->sync();
         }
         return No;

      case KDialogBase::Cancel:
         return Cancel;

      default: // Huh?
         break;
    }

    return Cancel; // Default
}

int
KMessageBox::warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const QString &buttonYes,
                          const QString &buttonNo,
                          const QString &dontAskAgainName,
                          bool /*notify*/)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = kapp->config();
       KConfigGroupSaver saver( config, grpNotifMsgs );
       QString dontAsk = config->readEntry( dontAskAgainName).lower();
       if (dontAsk == "yes")
       {
          return Yes;
       }
       if (dontAsk == "no")
       {
          return No;
       }
    }
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::No, KDialogBase::No,
                       parent, "warningYesNo", true, true,
                       buttonYes, buttonNo);

    bool checkboxResult;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString::null : i18n("Do not ask again"),
                       &checkboxResult);

    switch( result )
    {
      case KDialogBase::Yes:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "Yes");
            }
            config->sync();
         }
         return Yes;

      case KDialogBase::No:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "No");
            }
            config->sync();
         }
         return No;

      default: // Huh?
         break;
    }

    return No; // Default
}

int
KMessageBox::warningContinueCancel(QWidget *parent,
                                   const QString &text,
                                   const QString &caption,
                                   const QString &buttonContinue,
                                   const QString &dontAskAgainName,
                                   bool notify)
{
   return warningContinueCancelList(parent, text, QStringList(), caption,
                                buttonContinue, dontAskAgainName, notify);
}

int
KMessageBox::warningContinueCancelList(QWidget *parent, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const QString &buttonContinue,
                             const QString &dontAskAgainName,
                             bool /*notify*/)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    bool showMsg = true;

    if (!dontAskAgainName.isEmpty())
    {
       config = kapp->config();
       KConfigGroupSaver saver( config, grpNotifMsgs );
       showMsg = config->readBoolEntry( dontAskAgainName, true);
       if (!showMsg)
       {
          return Continue;
       }
    }

    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "warningYesNo", true, true,
                       buttonContinue, i18n("&Cancel"));

    bool checkboxResult;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString::null : i18n("Do not ask again"),
                       &checkboxResult);

    switch( result )
    {
      case KDialogBase::Yes:
      {
         if (!dontAskAgainName.isEmpty())
         {
            showMsg = !checkboxResult;
            if (!showMsg)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, showMsg);
            }
            config->sync();
         }
         return Continue;
      }

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
                                const QString &buttonNo,
                                const QString &dontAskAgainName,
                                bool /*notify*/)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = kapp->config();
       KConfigGroupSaver saver( config, grpNotifMsgs );
       QString dontAsk = config->readEntry( dontAskAgainName).lower();
       if (dontAsk == "yes")
       {
          return Yes;
       }
       if (dontAsk == "no")
       {
          return No;
       }
    }
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                       KDialogBase::Yes, KDialogBase::Cancel,
                       parent, "warningYesNoCancel", true, true,
                       buttonYes, buttonNo);

    bool checkboxResult;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString::null : i18n("Do not ask again"),
                       &checkboxResult);

    switch( result )
    {
      case KDialogBase::Yes:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "Yes");
            }
            config->sync();
         }
         return Yes;

      case KDialogBase::No:
         if (!dontAskAgainName.isEmpty())
         {
            if (checkboxResult)
            {
               KConfigGroupSaver saver( config, grpNotifMsgs );
               config->writeEntry( dontAskAgainName, "No");
            }
            config->sync();
         }
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
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString::null, 0);
}

void
KMessageBox::detailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption, bool /*notify*/)
{
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString::null, 0, details);
}

void
KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption, bool /*notify*/)
{
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString::null, 0);
}

void
KMessageBox::detailedSorry(QWidget *parent, const QString &text,
                   const QString &details,
                   const QString &caption, bool /*notify*/)
{
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString::null, 0, details);
}

void
KMessageBox::information(QWidget *parent,const QString &text,
                         const QString &caption, const QString &dontShowAgainName, bool /*notify*/)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    bool showMsg = true;

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

    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Information") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "information", true, true,
                       KStdGuiItem::ok() );

    bool checkboxResult;

    createKMessageBox(dialog, QMessageBox::Information, text, QStringList(),
		dontShowAgainName.isEmpty() ? QString::null : i18n("Do not show this message again"), 
                &checkboxResult);

    if (!dontShowAgainName.isEmpty())
    {
       showMsg = !checkboxResult;
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

int KMessageBox::messageBox( QWidget *parent, int type, const QString &text, const QString &caption, const QString &buttonYes, const QString &buttonNo )
{
    switch (type) {
        case QuestionYesNo:
            return KMessageBox::questionYesNo( parent,
                                               text, caption, buttonYes, buttonNo );
        case QuestionYesNoCancel:
            return KMessageBox::questionYesNoCancel( parent,
                                               text, caption, buttonYes, buttonNo );
        case WarningYesNo:
            return KMessageBox::warningYesNo( parent,
                                              text, caption, buttonYes, buttonNo );
        case WarningContinueCancel:
            return KMessageBox::warningContinueCancel( parent,
                                              text, caption, buttonYes );
        case WarningYesNoCancel:
            return KMessageBox::warningYesNoCancel( parent,
                                              text, caption, buttonYes, buttonNo );
        case Information:
            KMessageBox::information( parent,
                                      text, caption );
            return KMessageBox::Ok; // whatever

        case Error:
            KMessageBox::error( parent, text, caption );
            return KMessageBox::Ok; // whatever

        case Sorry:
            KMessageBox::sorry( parent, text, caption );
            return KMessageBox::Ok; // whatever
    }
    return KMessageBox::Cancel;
}

void KMessageBox::queuedMessageBox( QWidget *parent, int type, const QString &text, const QString &caption )
{
   KMessageBox_queue = true;
   (void) messageBox(parent, type, text, caption);
   KMessageBox_queue = false;
}

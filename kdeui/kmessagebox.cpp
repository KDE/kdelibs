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

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kguiitem.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <kstdguiitem.h>
#include <kactivelabel.h>
#include <kiconloader.h>

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
    }

   QPixmap ret = KApplication::kApplication()->iconLoader()->loadIcon(icon_name, KIcon::NoGroup, KIcon::SizeMedium, KIcon::DefaultState, 0, true);

   if (ret.isNull())
       return QMessageBox::standardIcon(icon);
   else
       return ret;
}

static int createKMessageBox(KDialogBase *dialog, QMessageBox::Icon icon, const QString &text, const QStringList &strlist, const QString &ask, bool *checkboxReturn, int options, const QString &details=QString::null)
{
    QVBox *topcontents = new QVBox (dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint());

    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);

    QLabel *label1 = new QLabel( contents);

    if (icon != QMessageBox::NoIcon)
        label1->setPixmap(themedMessageBoxIcon(icon));

    lay->addWidget( label1, 0, Qt::AlignCenter );
    lay->addSpacing(KDialog::spacingHint());
    // Enforce <p>text</p> otherwise the word-wrap doesn't work well
    QString qt_text;
    if ( !text.isEmpty() && (text[0] != '<') )
    {
        QStringList lines = QStringList::split('\n', text);
        for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
        {
           *it = QStyleSheet::convertFromPlainText( *it, QStyleSheetItem::WhiteSpaceNormal );
        }
        qt_text = lines.join(QString::null);
    }
    else
        qt_text = text;


    int pref_width = 0;
    int pref_height = 0;
    // Calculate a proper size for the text.
    {
       QSimpleRichText rt(qt_text, dialog->font());
       int scr = QApplication::desktop()->screenNumber(dialog);

       pref_width = QApplication::desktop()->screenGeometry(scr).width() / 3;
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

    KListBox *listbox = 0;
    if (!strlist.isEmpty())
    {
       listbox=new KListBox( topcontents );
       listbox->insertStringList( strlist );
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
         QLabel *label3 = new QLabel(details, detailsGroup);
         label3->setMinimumSize(label3->sizeHint());
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
KMessageBox::questionYesNoList(QWidget *parent, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           int options)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = KGlobal::config();
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
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);

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
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          int options)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = KGlobal::config();
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
    int result = createKMessageBox(dialog, QMessageBox::Information,
                       text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);

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
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          int options)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = KGlobal::config();
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
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);

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
                                   const KGuiItem &buttonContinue,
                                   const QString &dontAskAgainName,
                                   int options)
{
   return warningContinueCancelList(parent, text, QStringList(), caption,
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
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    bool showMsg = true;

    if (!dontAskAgainName.isEmpty())
    {
       config = KGlobal::config();
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
                       buttonContinue, KStdGuiItem::cancel() );

    bool checkboxResult;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);

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
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const QString &dontAskAgainName,
                                int options)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");

    if (!dontAskAgainName.isEmpty())
    {
       config = KGlobal::config();
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
                       dontAskAgainName.isEmpty() ? QString::null : i18n("&Do not ask again"),
                       &checkboxResult, options);

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
                   const QString &caption, int options)
{
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString::null, 0, options);
}

void
KMessageBox::detailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption, int options)
{
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString::null, 0, options, details);
}

void
KMessageBox::queuedDetailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
   KMessageBox_queue = true;
   (void) detailedError(parent, text, details, caption);
   KMessageBox_queue = false;
}


void
KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption, int options)
{
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString::null, 0, options);
}

void
KMessageBox::detailedSorry(QWidget *parent, const QString &text,
                   const QString &details,
                   const QString &caption, int options)
{
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString::null, 0, options, details);
}

void
KMessageBox::information(QWidget *parent,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, int options)
{
  informationList(parent, text, QStringList(), caption, dontShowAgainName, options);
}

void
KMessageBox::informationList(QWidget *parent,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, int options)
{
    KConfig *config = 0;
    QString grpNotifMsgs = QString::fromLatin1("Notification Messages");
    bool showMsg = true;

    if (!dontShowAgainName.isEmpty())
    {
       config = KGlobal::config();
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

    createKMessageBox(dialog, QMessageBox::Information, text, strlist,
		dontShowAgainName.isEmpty() ? QString::null : i18n("&Do not show this message again"),
                &checkboxResult, options);

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
   KConfig *config = KGlobal::config();
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

int KMessageBox::messageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption, const KGuiItem &buttonYes, const KGuiItem &buttonNo, int options )
{
    switch (type) {
        case QuestionYesNo:
            return KMessageBox::questionYesNo( parent,
                                               text, caption, buttonYes, buttonNo, QString::null, options );
        case QuestionYesNoCancel:
            return KMessageBox::questionYesNoCancel( parent,
                                               text, caption, buttonYes, buttonNo, QString::null, options );
        case WarningYesNo:
            return KMessageBox::warningYesNo( parent,
                                              text, caption, buttonYes, buttonNo, QString::null, options );
        case WarningContinueCancel:
            return KMessageBox::warningContinueCancel( parent,
                                              text, caption, buttonYes.text(), QString::null, options );
        case WarningYesNoCancel:
            return KMessageBox::warningYesNoCancel( parent,
                                              text, caption, buttonYes, buttonNo, QString::null, options );
        case Information:
            KMessageBox::information( parent,
                                      text, caption, QString::null, options );
            return KMessageBox::Ok;

        case Error:
            KMessageBox::error( parent, text, caption, options );
            return KMessageBox::Ok;

        case Sorry:
            KMessageBox::sorry( parent, text, caption, options );
            return KMessageBox::Ok;
    }
    return KMessageBox::Cancel;
}

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption, int options )
{
   KMessageBox_queue = true;
   (void) messageBox(parent, type, text, caption, KStdGuiItem::yes(),
                     KStdGuiItem::no(), options);
   KMessageBox_queue = false;
}

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption )
{
   KMessageBox_queue = true;
   (void) messageBox(parent, type, text, caption);
   KMessageBox_queue = false;
}

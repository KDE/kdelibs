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
 * $Log$
 * Revision 1.6  1999/09/12 13:35:04  espensa
 * I have had problems getting the action button to be properly underlined.
 * The changes should fix this once and for all. There were errors in the
 * "box->setButtonText()" (index errors)
 *
 * One minor typo removed in kmessagebox.h as well.
 *
 * Revision 1.5  1999/08/24 13:16:17  waba
 * WABA: Adding enums, fixing default for warningYesNo
 *
 * Revision 1.4  1999/08/16 15:38:32  waba
 * WABA: Make dialogs fixed size.
 *
 * Revision 1.3  1999/08/15 10:50:30  kulow
 * adding KMessageBox::about which uses the KDE icon instead of the Qt Information
 * icon - very cool! :)
 *
 * Revision 1.2  1999/07/26 07:27:04  kulow
 * it's OK
 *
 * Revision 1.1  1999/07/25 19:38:57  waba
 * WABA: Added some i18n'ed qmessagebox'es for convenience
 *
 *
 */

#include "kmessagebox.h"
#include <qmessagebox.h>

#include "kapp.h"
#include "klocale.h"

 /** 
  * Easy MessageBox Dialog. 
  *
  * Provides convenience functions for some i18n'ed standard dialogs.
  *
  * @author Waldo Bastian (bastian@kde.org)
  * @version $Id$
  */

int   
KMessageBox::questionYesNo(QWidget *parent, const QString &text,
                           const QString &caption,
                           const QString &buttonYes, 
                           const QString &buttonNo)
{
    QString _buttonYes = buttonYes;
    if (_buttonYes.isEmpty())
        _buttonYes = i18n("&Yes");
    QString _buttonNo = buttonNo;
    if (_buttonNo.isEmpty())
        _buttonNo = i18n("&No");

    QMessageBox *box = new QMessageBox(
      kapp->makeStdCaption(caption.isEmpty() ? i18n("Question") : caption),
      text,
      QMessageBox::Information,
      QMessageBox::Yes | QMessageBox::Default,
      QMessageBox::No | QMessageBox::Escape,	     
      0, parent, "information" );

    box->setButtonText(QMessageBox::Yes, _buttonYes);
    box->setButtonText(QMessageBox::No, _buttonNo);
    box->adjustSize();
    box->setFixedSize(box->size());

    int result = box->exec();
    delete box;
    if (result == QMessageBox::Yes)
	return Yes;
    else
        return No;
}

int 
KMessageBox::warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const QString &buttonYes,  
                          const QString &buttonNo)
{
    QString _buttonYes = buttonYes;
    if (_buttonYes.isEmpty())
        _buttonYes = i18n("&Yes");
    QString _buttonNo = buttonNo;
    if (_buttonNo.isEmpty())
        _buttonNo = i18n("&No");

    QMessageBox *box = new QMessageBox( 
      kapp->makeStdCaption(caption.isEmpty() ? i18n("Warning") : caption),
      text,
      QMessageBox::Warning,
      QMessageBox::Yes,
      QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
      0, parent, "warning" );

    box->setButtonText(QMessageBox::Yes, _buttonYes);
    box->setButtonText(QMessageBox::No, _buttonNo);
    box->adjustSize();
    box->setFixedSize(box->size());

    int result = box->exec();
    delete box;
    if (result == QMessageBox::Yes)
	return Yes;
    else
        return No;
}

int 
KMessageBox::warningYesNoCancel(QWidget *parent, const QString &text,
                                const QString &caption,
                                const QString &buttonYes,  
                                const QString &buttonNo)
{
    QString _buttonYes = buttonYes;
    if (_buttonYes.isEmpty())
        _buttonYes = i18n("&Yes");
    QString _buttonNo = buttonNo;
    if (_buttonNo.isEmpty())
        _buttonNo = i18n("&No");

    QMessageBox *box = new QMessageBox( 
      kapp->makeStdCaption(caption.isEmpty() ? i18n("Warning") : caption),
      text,
      QMessageBox::Warning,
      QMessageBox::Yes | QMessageBox::Default,
      QMessageBox::No,
      QMessageBox::Cancel | QMessageBox::Escape,	     
      parent, "warning" );

    box->setButtonText(QMessageBox::Yes, _buttonYes);
    box->setButtonText(QMessageBox::No, _buttonNo);
    box->setButtonText(QMessageBox::Cancel, i18n("&Cancel")); 
    box->adjustSize();
    box->setFixedSize(box->size());

    int result = box->exec();
    delete box;
    if (result == QMessageBox::Yes)
	return Yes;
    else if (result == QMessageBox::No)
        return No;
    else 
        return Cancel;
}

void
KMessageBox::error(QWidget *parent,  const QString &text,
                   const QString &caption)
{
    QMessageBox *box = new QMessageBox(
      kapp->makeStdCaption(caption.isEmpty() ? i18n("Error") : caption),
      text,
      QMessageBox::Critical,
      QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
      0, 0, parent, "critical" );
 
    box->setButtonText(QMessageBox::Ok, i18n("&OK"));
    box->adjustSize();
    box->setFixedSize(box->size());

    box->exec();
    delete box;
    return;
}

void
KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption)
{
    QMessageBox *box = new QMessageBox( 
      kapp->makeStdCaption(caption.isEmpty() ? i18n("Sorry") : caption),
      text,
      QMessageBox::Warning,
      QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
      0, 0, parent, "warning" );
 
    box->setButtonText(QMessageBox::Ok, i18n("&OK"));
    box->adjustSize();
    box->setFixedSize(box->size());

    box->exec();
    delete box;
    return;
}

void
KMessageBox::information(QWidget *parent,const QString &text,
                   const QString &caption)
{
    QMessageBox *box = new QMessageBox( 
      kapp->makeStdCaption(caption.isEmpty() ? i18n("Information") : caption),
      text,
      QMessageBox::Information,
      QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
      0, 0, parent, "information" );

    box->setButtonText(QMessageBox::Ok, i18n("&OK"));
    box->adjustSize();
    box->setFixedSize(box->size());

    box->exec();
    delete box;
    return;
}

void
KMessageBox::about(QWidget *parent, const QString &text,
                   const QString &caption)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("About %1").arg(kapp->getCaption());

    QMessageBox *box = new QMessageBox( _caption, text,
              QMessageBox::Information,
              QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape, 
              0, 0,
              parent, "about" );

    box->setButtonText(QMessageBox::Ok, i18n("&OK"));
    box->setIconPixmap(kapp->getIcon());
    box->adjustSize();
    box->setFixedSize(box->size());

    box->exec();
    delete box;
    return;
}


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
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("Question");
    _caption += " - "+kapp->getCaption();
    QString _buttonYes = buttonYes;
    if (_buttonYes.isEmpty())
        _buttonYes = i18n("&Yes");
    QString _buttonNo = buttonNo;
    if (_buttonNo.isEmpty())
        _buttonNo = i18n("&No");

#if 0
    return QMessageBox::information(parent, _caption, text, 
	_buttonYes, _buttonNo, QString::null, 0, 1);
#endif
    int result;
    QMessageBox *box;
    box = new QMessageBox( _caption, text,
             QMessageBox::Information,
             QMessageBox::Yes | QMessageBox::Default,
             QMessageBox::No | QMessageBox::Escape,	     
	     0, parent, "information" );
    box->setButtonText(0, _buttonYes);
    box->setButtonText(1, _buttonNo);
    box->adjustSize();
    box->setMinimumSize(box->size());
    box->setMaximumSize(box->size());
    result = box->exec();
    delete box;
    return result;
}

int 
KMessageBox::warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const QString &buttonYes,  
                          const QString &buttonNo)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("Warning");
    _caption += " - "+kapp->getCaption();
    QString _buttonYes = buttonYes;
    if (_buttonYes.isEmpty())
        _buttonYes = i18n("&Yes");
    QString _buttonNo = buttonNo;
    if (_buttonNo.isEmpty())
        _buttonNo = i18n("&No");

#if 0
    return QMessageBox::warning(parent, _caption, text,
	_buttonYes, _buttonNo, QString::null, 1, 1);
#endif
    int result;
    QMessageBox *box;
    box = new QMessageBox( _caption, text,
             QMessageBox::Warning,
             QMessageBox::Yes | QMessageBox::Default,
             QMessageBox::No | QMessageBox::Escape,	     
	     0, parent, "warning" );
    box->setButtonText(0, _buttonYes);
    box->setButtonText(1, _buttonNo);
    box->adjustSize();
    box->setMinimumSize(box->size());
    box->setMaximumSize(box->size());
    result = box->exec();
    delete box;
    return result;
}

int 
KMessageBox::warningYesNoCancel(QWidget *parent, const QString &text,
                                const QString &caption,
                                const QString &buttonYes,  
                                const QString &buttonNo)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("Warning");
    _caption += " - "+kapp->getCaption();
    QString _buttonYes = buttonYes;
    if (_buttonYes.isEmpty())
        _buttonYes = i18n("&Yes");
    QString _buttonNo = buttonNo;
    if (_buttonNo.isEmpty())
        _buttonNo = i18n("&No");

#if 0
    return QMessageBox::warning(parent, _caption, text, 
	       _buttonYes, _buttonNo, i18n("&Cancel"), 0, 2);

#endif
    int result;
    QMessageBox *box;
    box = new QMessageBox( _caption, text,
             QMessageBox::Warning,
             QMessageBox::Yes | QMessageBox::Default,
             QMessageBox::No,
             QMessageBox::Cancel | QMessageBox::Escape,	     
	     parent, "warning" );
    box->setButtonText(0, _buttonYes);
    box->setButtonText(1, _buttonNo);
    box->setButtonText(1, i18n("&Cancel"));
    box->adjustSize();
    box->setMinimumSize(box->size());
    box->setMaximumSize(box->size());
    result = box->exec();
    delete box;
    return result;
}

void
KMessageBox::error(QWidget *parent,  const QString &text,
                   const QString &caption)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("Error");
    _caption += " - "+kapp->getCaption();

#if 0
    (void) QMessageBox::critical(parent, _caption, text,
	       i18n("&OK"), QString::null, QString::null, 0, 0);
#endif
    QMessageBox *box;
    box = new QMessageBox( _caption, text,
             QMessageBox::Critical,
             QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
	     0, 0, parent, "critical" );
    box->setButtonText(0, i18n("&OK"));
    box->adjustSize();
    box->setMinimumSize(box->size());
    box->setMaximumSize(box->size());
    box->exec();
    delete box;
    return;
}

void
KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("Sorry");
    _caption += " - " + kapp->getCaption();

#if 0
    (void) QMessageBox::warning(parent, _caption, text, 
	       i18n("&OK"), QString::null, QString::null, 0, 0);
#endif

    QMessageBox *box;
    box = new QMessageBox( _caption, text,
             QMessageBox::Warning,
             QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
	     0, 0, parent, "warning" );
    box->setButtonText(0, i18n("&OK"));
    box->adjustSize();
    box->setMinimumSize(box->size());
    box->setMaximumSize(box->size());
    box->exec();
    delete box;

    return;
}

void
KMessageBox::information(QWidget *parent,const QString &text,
                   const QString &caption)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("Information");
    _caption += " - "+kapp->getCaption();

#if 0
    (void) QMessageBox::information(parent, _caption, text,  
	       i18n("&OK"), QString::null, QString::null, 0, 0);
#endif

    QMessageBox *box;
    box = new QMessageBox( _caption, text,
             QMessageBox::Information,
             QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
	     0, 0, parent, "information" );
    box->setButtonText(0, i18n("&OK"));
    box->adjustSize();
    box->setMinimumSize(box->size());
    box->setMaximumSize(box->size());
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

    QMessageBox *box;
    box  = new QMessageBox( _caption, text,
              QMessageBox::Information,
              QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape, 
              0, 0,
              parent, "about" );
    box->setButtonText(0, i18n("&OK"));
    box->setIconPixmap(kapp->getIcon());
    box->exec();
    delete box;
    return;
}


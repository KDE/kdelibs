/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KMESSAGEBOXWRAPPER_H
#define KMESSAGEBOXWRAPPER_H
#include <kmessagebox.h>
#include <kapplication.h>
#include <kdebug.h>

/**
 * @internal
 * Allows KIO classes to display dialog boxes with the correct
 * theme/style even in non-GUI apps like kded and kfmclient
 */
class KMessageBoxWrapper : public KMessageBox
{
public:
  static void error(QWidget *parent, 
                    const QString &text, 
                    const QString &caption = QString::null)
    {
	if (kapp) {
     	  kapp->enableStyles();
      	  KMessageBox::error( parent, text, caption );
	} else 
	  kdWarning() << text << endl;
    }

  static void sorry(QWidget *parent, 
                    const QString &text,
                    const QString &caption = QString::null)
    {
      if (kapp) {
	 kapp->enableStyles();
         KMessageBox::sorry( parent, text, caption );
      } else
	kdWarning() << text << endl;
    }
  
};
#endif

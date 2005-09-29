/*  This file is part of the KDE Libraries
    Copyright ( C ) 2002 Nadeem Hasan ( nhasan@kde.org )

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or ( at your option ) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qstring.h>
#include <qapplication.h>
#include <qpushbutton.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kstdguiitem.h>

#include "kwizard.h"

KWizard::KWizard( QWidget *parent, const char *name, bool modal, WFlags f )
            : QWizard( parent, name, modal, f )
{
  bool useIcons = KGlobalSettings::showIconsOnPushButtons();

  if ( useIcons )
  {
    KGuiItem back = KStdGuiItem::back( KStdGuiItem::UseRTL );
    KGuiItem forward = KStdGuiItem::forward( KStdGuiItem::UseRTL );

    backButton()->setIconSet( back.iconSet() );
    nextButton()->setIconSet( forward.iconSet() );

    finishButton()->setIconSet( SmallIconSet( "apply" ) );
    cancelButton()->setIconSet( SmallIconSet( "button_cancel" ) );
    helpButton()->setIconSet( SmallIconSet( "help" ) );

    backButton()->setText( i18n( "&Back" ) );
    nextButton()->setText( i18n( "Opposite to Back","&Next" ) );
  }

  QFont font = titleFont();
  font.setBold( true );
  setTitleFont( font );
}

#include "kwizard.moc"

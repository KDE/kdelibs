/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

// I (espen) prefer that header files are included alphabetically
#include <kaboutkde.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kstandarddirs.h>


KAboutKDE::KAboutKDE( QWidget *parent, bool modal )
  :KAboutDialog( KAboutDialog::AbtKDEStandard, QLatin1String("KDE"),
		 KDialogBase::Help|KDialogBase::Close, KDialogBase::Close,
		 parent, modal )
{
  const QString text1 = i18n(""
    "The <b>K Desktop Environment</b> is written and maintained by the "
    "KDE Team, a world-wide network of software engineers committed to "
    "<a href=\"http://www.gnu.org/philosophy/free-sw.html\">Free Software</a> development.<br><br>"
    "No single group, company or organization controls the KDE source "
    "code. Everyone is welcome to contribute to KDE.<br><br>"
    "Visit <A HREF=\"http://www.kde.org/\">http://www.kde.org</A> for "
    "more information on the KDE project. ");

  const QString text2 = i18n(""
    "Software can always be improved, and the KDE Team is ready to "
    "do so. However, you - the user - must tell us when "
    "something does not work as expected or could be done better.<br><br>"
    "The K Desktop Environment has a bug tracking system. Visit "
    "<A HREF=\"http://bugs.kde.org/\">http://bugs.kde.org</A> or "
    "use the \"Report Bug...\" dialog from the \"Help\" menu to report bugs.<br><br>"
    "If you have a suggestion for improvement then you are welcome to use "
    "the bug tracking system to register your wish. Make sure you use the "
    "severity called \"Wishlist\"." );

  const QString text3 = i18n(""
    "You do not have to be a software developer to be a member of the "
    "KDE team. You can join the national teams that translate "
    "program interfaces. You can provide graphics, themes, sounds, and "
    "improved documentation. You decide!"
    "<br><br>"
    "Visit "
    "<A HREF=\"http://www.kde.org/jobs/\">http://www.kde.org/jobs/</A> "
    "for information on some projects in which you can participate."
    "<br><br>"
    "If you need more information or documentation, then a visit to "
    "<A HREF=\"http://developer.kde.org/\">http://developer.kde.org</A> "
    "will provide you with what you need.");
  const QString text4 = i18n(""
    "KDE is available free of charge, but making it is not free.<br><br>"
    "Thus, the KDE team formed the KDE e.V., a non-profit organization"
    " legally founded in Tuebingen, Germany. The KDE e.V. represents"
    " the KDE project in legal and financial matters."
    " See <a href=\"http://www.kde-ev.org/\">http://www.kde-ev.org</a>"
    " for information on the KDE e.V.<br><br>"
    "The KDE team does need financial support. Most of the money is used to "
    "reimburse members and others on expenses they experienced when "
    "contributing to KDE. You are encouraged to support KDE through a financial "
    "donation, using one of the ways described at "
    "<a href=\"http://www.kde.org/support/\">http://www.kde.org/support/</a>."
    "<br><br>Thank you very much in advance for your support.");
  setHelp( QLatin1String("khelpcenter/main.html"), QString() );
  setTitle(i18n("K Desktop Environment. Release %1").
	   arg(QLatin1String(KDE_VERSION_STRING)) );
  addTextPage( i18n("About KDE","&About"), text1, true );
  addTextPage( i18n("&Report Bugs or Wishes"), text2, true );
  addTextPage( i18n("&Join the KDE Team"), text3, true );
  addTextPage( i18n("&Support KDE"), text4, true );
  setImage( locate( "data", QLatin1String("kdeui/pics/aboutkde.png")) );
  setImageBackgroundColor( Qt::white );
}

/*  This file is part of the KDE Libraries
    Copyright (C) 1999 Harri Porten (porten@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#include <qpushbutton.h>
#include <klocale.h>

#include "kwizard.h"

KWizard::KWizard(QWidget *parent, const char *name, bool modal, WFlags f)
  : QWizard(parent, name, modal, f)
{
  backButton()->setText("< " + i18n("Back"));
  nextButton()->setText(i18n("Next") + " >");
  helpButton()->setText(i18n("Help"));
  finishButton()->setText(i18n("Finish"));
  cancelButton()->setText(i18n("Cancel"));
}

#include "kwizard.moc"

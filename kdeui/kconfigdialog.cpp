/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+kdelibs at meyerhome dot net)
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */
#include "kconfigdialog.h"
#include "kconfigdialog.moc"

#include <kconfigskeleton.h>
#include <kconfigdialogmanager.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qvbox.h>

QAsciiDict<QObject> KConfigDialog::openDialogs;

// This class is here purly so we don't break binary compatibility down the road.
class KConfigDialog::KConfigDialogPrivate
{

public:
  KConfigDialogPrivate(KDialogBase::DialogType t) 
  : track(true), shown(false), type(t), mgr(0) { }

  bool track;
  bool shown;
  KDialogBase::DialogType type;
  KConfigDialogManager *mgr;
};

KConfigDialog::KConfigDialog( QWidget *parent, const char *name,
		  KConfigSkeleton *config,
		  KDialogBase::DialogType dialogType,
		  KDialogBase::ButtonCode dialogButtons,
		  bool modal ) :
    QObject(parent, name), d(new KConfigDialogPrivate(dialogType)) 
{		  
  openDialogs.insert(name, this);
  kdialogbase = new KDialogBase( dialogType, Qt::WStyle_DialogBorder | Qt::WDestructiveClose,
		  parent, name, true, i18n("Configure"), dialogButtons );

  connect(kdialogbase, SIGNAL(destroyed()), this, SLOT(deleteLater()));
  connect(kdialogbase, SIGNAL(okClicked()), this, SIGNAL(okClicked()));
  connect(kdialogbase, SIGNAL(applyClicked()), this, SIGNAL(applyClicked()));
  connect(kdialogbase, SIGNAL(defaultClicked()), this, SIGNAL(defaultClicked()));

  d->mgr = new KConfigDialogManager(kdialogbase, config);

  // TODO: Emit settingsChanged signal from slot to guarantee sequence
  connect(d->mgr, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));
  connect(d->mgr, SIGNAL(settingsChanged()), this, SLOT(settingsChangedSlot()));
  connect(d->mgr, SIGNAL(widgetModified()), this, SLOT(settingModified()));

  connect(kdialogbase, SIGNAL(okClicked()), this, SLOT(updateSettings()));
  connect(kdialogbase, SIGNAL(okClicked()), d->mgr, SLOT(updateSettings()));
  connect(kdialogbase, SIGNAL(applyClicked()), this, SLOT(updateSettings()));
  connect(kdialogbase, SIGNAL(applyClicked()), d->mgr, SLOT(updateSettings()));
  connect(kdialogbase, SIGNAL(defaultClicked()), this, SLOT(updateWidgetsDefault()));
  connect(kdialogbase, SIGNAL(defaultClicked()), d->mgr, SLOT(updateWidgetsDefault()));

  connect(kdialogbase, SIGNAL(defaultClicked()), this, SLOT(settingModified()));

  kdialogbase->enableButton(KDialogBase::Apply, false);
}

KConfigDialog::~KConfigDialog()
{
  openDialogs.remove(name());
  delete d;
}

void KConfigDialog::addPage(QWidget *page,
                                const QString &itemName,
                                const QString &pixmapName,
                                const QString &header,
                                bool manage)
{
  if(d->shown)
  {
    kdDebug(240) << "KConfigDialog::addPage, can not a page after the dialog has been shown.";
    return;
  }
  switch(d->type)
  {
    case KDialogBase::TreeList:
    case KDialogBase::IconList:
    case KDialogBase::Tabbed: {
      QVBox *frame = kdialogbase->addVBoxPage(itemName, header, SmallIcon(pixmapName, 32));
      frame->setSpacing( 0 );
      frame->setMargin( 0 );
      page->reparent(((QWidget*)frame), 0, QPoint());
    }
    break;

    case KDialogBase::Swallow: 
    {
      page->reparent(((QWidget*)kdialogbase), 0, QPoint());
      kdialogbase->setMainWidget(page);
    }
    break;

    case KDialogBase::Plain:
    {
      page->reparent(((QWidget*)kdialogbase), 0, QPoint());
      QFrame *page = kdialogbase->plainPage();
      QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 0 );
      page->reparent(((QWidget*)page), 0, QPoint());
      topLayout->addWidget( page );
      kdialogbase->setMainWidget(page);
    }
    break;

    default:
      kdDebug(240) << "KConfigDialog::addWidget" << " unknown type.";
  }
  if(manage)
    d->mgr->addWidget(page);
}

KConfigDialog* KConfigDialog::exists(const char* name)
{
  return ((KConfigDialog*)openDialogs.find(name));
}

bool KConfigDialog::showDialog(const char* name)
{
  KConfigDialog *dialog = exists(name);
  if(dialog)
    dialog->show();
  return (dialog != NULL);
}

void KConfigDialog::settingModified()
{
  if(d->track)
  {
    kdialogbase->enableButton(KDialogBase::Apply, d->mgr->hasChanged() || hasChanged());
    kdialogbase->enableButton(KDialogBase::Default, !(d->mgr->isDefault() && isDefault()));
  }
}

void KConfigDialog::settingsChangedSlot()
{
  // Update the buttons
  settingModified();
  emit (settingsChanged(name()));
}

void KConfigDialog::setCaption(const QString &caption)
{
  kdialogbase->setCaption(caption);
}

void KConfigDialog::show(bool track)
{
  if(!d->shown)
  {
    updateWidgets();
    d->mgr->updateWidgets();
    kdialogbase->enableButton(KDialogBase::Apply, d->mgr->hasChanged() || hasChanged());
    kdialogbase->enableButton(KDialogBase::Default, !(d->mgr->isDefault() && isDefault()));
    d->shown = true;
    if(!track)
    {
      kdialogbase->enableButton(KDialogBase::Apply, true);
      kdialogbase->enableButton(KDialogBase::Default, true);
    }
  }
  d->track = track;
  kdialogbase->show();
}

void KConfigDialog::updateSettings()
{
}

void KConfigDialog::updateWidgets()
{
}

void KConfigDialog::updateWidgetsDefault()
{
}

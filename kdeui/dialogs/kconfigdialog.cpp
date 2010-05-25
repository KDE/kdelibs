/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+kdelibs at meyerhome dot net)
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2004 Michael Brade <brade@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include "kconfigdialog.h"

#include <kcomponentdata.h>
#include <kconfigdialogmanager.h>
#include <kconfigskeleton.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpagewidgetmodel.h>
#include <kvbox.h>

#include <QtGui/QLayout>
#include <QtCore/QMap>

class KConfigDialog::KConfigDialogPrivate
{
public:
  KConfigDialogPrivate(KConfigDialog *q)
    : q(q), shown(false), manager(0) { }

  KPageWidgetItem* addPageInternal(QWidget *page, const QString &itemName,
                           const QString &pixmapName, const QString &header);

  void setupManagerConnections(KConfigDialogManager *manager);

  void _k_updateButtons();
  void _k_settingsChangedSlot();

  KConfigDialog *q;
  bool shown;
  KConfigDialogManager *manager;
  QMap<QWidget *, KConfigDialogManager *> managerForPage;

  /**
    * The list of existing dialogs.
   */
  static QHash<QString,KConfigDialog *> openDialogs;
};

QHash<QString,KConfigDialog *> KConfigDialog::KConfigDialogPrivate::openDialogs;

KConfigDialog::KConfigDialog( QWidget *parent, const QString& name,
          KConfigSkeleton *config ) :
    KPageDialog( parent ),
    d(new KConfigDialogPrivate(this))
{
  setCaption( i18n("Configure") );
  setFaceType( List );
  setButtons( Default|Ok|Apply|Cancel|Help );
  setHelp( QString(), KGlobal::mainComponent().componentName() );
  setDefaultButton( Ok );
  setObjectName( name );

  if ( !name.isEmpty() ) {
    KConfigDialogPrivate::openDialogs.insert(name, this);
  } else {
    QString genericName;
    genericName.sprintf("SettingsDialog-%p", static_cast<void*>(this));
    KConfigDialogPrivate::openDialogs.insert(genericName, this);
    setObjectName(genericName);
  }

  connect(this, SIGNAL(okClicked()), this, SLOT(updateSettings()));
  connect(this, SIGNAL(applyClicked()), this, SLOT(updateSettings()));
  connect(this, SIGNAL(applyClicked()), this, SLOT(_k_updateButtons()));
  connect(this, SIGNAL(cancelClicked()), this, SLOT(updateWidgets()));
  connect(this, SIGNAL(defaultClicked()), this, SLOT(updateWidgetsDefault()));
  connect(this, SIGNAL(defaultClicked()), this, SLOT(_k_updateButtons()));
  connect(this, SIGNAL(pageRemoved(KPageWidgetItem*)), this, SLOT(onPageRemoved(KPageWidgetItem*)));

  d->manager = new KConfigDialogManager(this, config);
  d->setupManagerConnections(d->manager);

  enableButton(Apply, false);
}

KConfigDialog::~KConfigDialog()
{
  KConfigDialogPrivate::openDialogs.remove(objectName());
  delete d;
}

KPageWidgetItem* KConfigDialog::addPage(QWidget *page,
                                const QString &itemName,
                                const QString &pixmapName,
                                const QString &header,
                                bool manage)
{
  Q_ASSERT(page);
  if (!page) {
      return 0;
  }

  KPageWidgetItem* item = d->addPageInternal(page, itemName, pixmapName, header);
  if (manage) {
    d->manager->addWidget(page);
  }

  if (d->shown && manage) {
    // update the default button if the dialog is shown
    bool is_default = isButtonEnabled(Default) && d->manager->isDefault();
    enableButton(Default,!is_default);
  }
  return item;
}

KPageWidgetItem* KConfigDialog::addPage(QWidget *page,
                                KConfigSkeleton *config,
                                const QString &itemName,
                                const QString &pixmapName,
                                const QString &header)
{
  Q_ASSERT(page);
  if (!page) {
      return 0;
  }

  KPageWidgetItem* item = d->addPageInternal(page, itemName, pixmapName, header);
  d->managerForPage[page] = new KConfigDialogManager(page, config);
  d->setupManagerConnections(d->managerForPage[page]);
  
  if (d->shown)
  {
    // update the default button if the dialog is shown
    bool is_default = isButtonEnabled(Default) && d->managerForPage[page]->isDefault();
    enableButton(Default,!is_default);
  }
  return item;
}

KPageWidgetItem* KConfigDialog::KConfigDialogPrivate::addPageInternal(QWidget *page,
                                        const QString &itemName,
                                        const QString &pixmapName,
                                        const QString &header)
{
  KVBox *frame = new KVBox(q);
  frame->setSpacing(-1);
  page->setParent(frame);

  KPageWidgetItem *item = new KPageWidgetItem( frame, itemName );
  item->setHeader( header );
  if ( !pixmapName.isEmpty() )
    item->setIcon( KIcon( pixmapName ) );

  q->KPageDialog::addPage( item );
  return item;
}

void KConfigDialog::KConfigDialogPrivate::setupManagerConnections(KConfigDialogManager *manager)
{
    q->connect(manager, SIGNAL(settingsChanged()), q, SLOT(_k_settingsChangedSlot()));
    q->connect(manager, SIGNAL(widgetModified()), q, SLOT(_k_updateButtons()));

    q->connect(q, SIGNAL(okClicked()), manager, SLOT(updateSettings()));
    q->connect(q, SIGNAL(applyClicked()), manager, SLOT(updateSettings()));
    q->connect(q, SIGNAL(cancelClicked()), manager, SLOT(updateWidgets()));
    q->connect(q, SIGNAL(defaultClicked()), manager, SLOT(updateWidgetsDefault()));
}

void KConfigDialog::onPageRemoved( KPageWidgetItem *item )
{	
	QMap<QWidget *, KConfigDialogManager *>::iterator j = d->managerForPage.begin();
	while (j != d->managerForPage.end())
	{
		// there is a manager for this page, so remove it
		if (item->widget()->isAncestorOf(j.key())) 
		{
			KConfigDialogManager* manager = j.value();
			d->managerForPage.erase(j);
			delete manager;
			d->_k_updateButtons();
			break;
		}
		j++;
	}
}

KConfigDialog* KConfigDialog::exists(const QString& name)
{
  QHash<QString,KConfigDialog *>::const_iterator it = KConfigDialogPrivate::openDialogs.constFind( name );
  if ( it != KConfigDialogPrivate::openDialogs.constEnd() )
      return *it;
  return 0;
}

bool KConfigDialog::showDialog(const QString& name)
{
  KConfigDialog *dialog = exists(name);
  if(dialog)
    dialog->show();
  return (dialog != NULL);
}

void KConfigDialog::KConfigDialogPrivate::_k_updateButtons()
{
  static bool only_once = false;
  if (only_once) return;
  only_once = true;

  QMap<QWidget *, KConfigDialogManager *>::iterator it;

  bool has_changed = manager->hasChanged() || q->hasChanged();
  for (it = managerForPage.begin();
          it != managerForPage.end() && !has_changed;
          ++it)
  {
    has_changed |= (*it)->hasChanged();
  }

  q->enableButton(KDialog::Apply, has_changed);

  bool is_default = manager->isDefault() && q->isDefault();
  for (it = managerForPage.begin();
          it != managerForPage.end() && is_default;
          ++it)
  {
    is_default &= (*it)->isDefault();
  }

  q->enableButton(KDialog::Default, !is_default);

  emit q->widgetModified();
  only_once = false;
}

void KConfigDialog::KConfigDialogPrivate::_k_settingsChangedSlot()
{
  // Update the buttons
  _k_updateButtons();
  emit q->settingsChanged(q->objectName());
}

void KConfigDialog::showEvent(QShowEvent *e)
{
  if (!d->shown)
  {
    QMap<QWidget *, KConfigDialogManager *>::iterator it;

    updateWidgets();
    d->manager->updateWidgets();
    for (it = d->managerForPage.begin(); it != d->managerForPage.end(); ++it)
      (*it)->updateWidgets();

    bool has_changed = d->manager->hasChanged() || hasChanged();
    for (it = d->managerForPage.begin();
            it != d->managerForPage.end() && !has_changed;
            ++it)
    {
      has_changed |= (*it)->hasChanged();
    }

    enableButton(Apply, has_changed);

    bool is_default = d->manager->isDefault() && isDefault();
    for (it = d->managerForPage.begin();
            it != d->managerForPage.end() && is_default;
            ++it)
    {
      is_default &= (*it)->isDefault();
    }

    enableButton(Default, !is_default);
    d->shown = true;
  }
  KPageDialog::showEvent(e);
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

bool KConfigDialog::hasChanged()
{
    return false;
}

bool KConfigDialog::isDefault()
{
    return true;
}

void KConfigDialog::updateButtons()
{
    d->_k_updateButtons();
}

void KConfigDialog::settingsChangedSlot()
{
    d->_k_settingsChangedSlot();
}

#include "kconfigdialog.moc"

/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kglobalaccel.h"

// For KGlobalAccelImpl
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#else
#include "kglobalaccel_emb.h"
#endif

#include <QMap>
#include <QSet>

#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kaction.h>
#include <kconfig.h>

KGlobalAccel* KGlobalAccel::s_instance = 0L;
static KStaticDeleter<KGlobalAccel> sd;

class KGlobalAccelData
{
public:
	KGlobalAccelData()
		: configGroup("Global Shortcuts")
		, enabled(true)
		, implEnabled(false)
	{
	}

	QMap<int, KAction*> grabbedKeys;
	QMultiMap<KAction*, int> grabbedActions;
	QSet<KAction*> actionsWithGlobalShortcuts;
	
	QString configGroup;
	bool enabled, implEnabled;
};

KGlobalAccel::KGlobalAccel()
	: d(new KGlobalAccelData)
	, i(new KGlobalAccelImpl(this))
{
}

KGlobalAccel::~KGlobalAccel()
{
	foreach (int key, d->grabbedKeys.keys())
		i->grabKey(key, false);
	
	delete i;
	delete d;
}

bool KGlobalAccel::isEnabled()
{
	return d->enabled;
}

void KGlobalAccel::setEnabled( bool enabled )
{
	d->enabled = enabled;

	if (enabled) {
		foreach (KAction* action, d->actionsWithGlobalShortcuts)
			checkAction(action);
	
	} else {
		foreach (int key, d->grabbedKeys.keys())
			i->grabKey(key, false);
		d->grabbedActions.clear();
		d->grabbedKeys.clear();
	}
}

const QString& KGlobalAccel::configGroup() const
{
	return d->configGroup;
}

// for kdemultimedia/kmix
void KGlobalAccel::setConfigGroup( const QString& s )
{
	d->configGroup = s;
}

bool KGlobalAccel::readSettings( KConfigBase* config )
{
	if (!config)
		config = KGlobal::config();

	QMapIterator<QString, QString> it = config->entryMap( configGroup() );
	while (it.hasNext()) {
		it.next();
		foreach (KActionCollection* collection, KActionCollection::allCollections()) {
			if (KAction* action = collection->action(it.key().toAscii().constData())) {
				action->setGlobalShortcut(KShortcut(it.value()), KAction::CustomShortcut);
				goto found;
			}

			kDebug(125) << k_funcinfo << "Warning: Could not find action '" << it.key() << "' - was this function called too early?" << endl;

			found:
			continue;
		}
	}
	
	config->sync();

	return true;
}

bool KGlobalAccel::writeSettings( KConfigBase* config, bool writeDefaults, KAction* oneAction ) const
{
	if (!config)
		config = KGlobal::config();

	KConfigGroup cg( config, configGroup() );

	if (oneAction) {
		if (writeDefaults || oneAction->globalShortcut() != oneAction->defaultGlobalShortcut())
			cg.writeEntry(oneAction->objectName(), oneAction->globalShortcut().toStringInternal());
		else if (cg.hasKey(oneAction->objectName()))
			cg.deleteEntry(oneAction->objectName());

	} else {
		foreach (KAction* action, d->actionsWithGlobalShortcuts)
			if (writeDefaults || action->globalShortcut() != action->defaultGlobalShortcut())
				cg.writeEntry(action->objectName(), action->globalShortcut().toStringInternal());
			else if (cg.hasKey(action->objectName()))
				cg.deleteEntry(action->objectName());
	}

	config->sync();
	
	return true;
}

KGlobalAccel * KGlobalAccel::self( )
{
	if (!s_instance)
		sd.setObject(s_instance, new KGlobalAccel());

	return s_instance;
}

void KGlobalAccel::checkAction( KAction* action )
{
	if (!action->globalShortcutAllowed() ||action->globalShortcut().isNull()) {
		if (d->actionsWithGlobalShortcuts.contains(action)) {
			disconnect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
			d->actionsWithGlobalShortcuts.remove(action);
			
			if (d->actionsWithGlobalShortcuts.isEmpty())
				enableImpl(false);
		}
	
	} else {
		if (!d->actionsWithGlobalShortcuts.contains(action)) {
			connect(action, SIGNAL(changed()), SLOT(actionChanged()));
			if (isEnabled())
				enableImpl(true);
			d->actionsWithGlobalShortcuts.insert(action);
		}
	}

	QList<int> currentlyGrabbed = d->grabbedActions.values(action);
	
	QList<int> needToGrab;
	if (action->isEnabled() && action->globalShortcutAllowed())
		foreach (const KShortcut& cut, action->globalShortcut().sequences())
			if (!cut.isNull() && cut.seq(0))
				needToGrab.append(cut.seq(0));
	
	foreach (int key, currentlyGrabbed)
		if (needToGrab.contains(key))
			needToGrab.removeAll(key);
		else
			grabKey(key, false, action);
	
	foreach (int key, needToGrab)
		grabKey(key, true, action);
}

void KGlobalAccel::actionChanged( )
{
	if (!isEnabled())
		return;

	KAction* action = qobject_cast<KAction*>(sender());
	if (!action)
		return;
	
	if (d->actionsWithGlobalShortcuts.contains(action)) {
		if (action->isEnabled() == d->grabbedActions.contains(action))
			// Nothing to do
			return;
		
		// Action has been enabled or disabled, grab / release
		if (action->isEnabled())
			checkAction(action);
	}
}

void KGlobalAccel::grabKey( int key, bool grab, KAction * action )
{
	if (grab && d->grabbedKeys.contains(key)) {
		kWarning(125) << k_funcinfo << "Tried to grab a key already owned by action " << d->grabbedKeys[key]->objectName() << endl;
		return;
	}

	if (i->grabKey(key, grab)) {
		if (grab) {
			d->grabbedKeys.insert(key, action);
			d->grabbedActions.insert(action, key);

		} else {
			d->grabbedKeys.remove(key);

			for (QMultiMap<KAction*, int>::Iterator it = d->grabbedActions.find(action); it != d->grabbedActions.end() && it.key() == action; ++it)
				if (it.value() == key) {
					d->grabbedActions.erase(it);
					break;
				}
		}
	}
}

bool KGlobalAccel::keyPressed( int key )
{
	bool consumed = false;
	foreach (KAction* action, d->grabbedKeys.values(key)) {
		consumed = true;
		action->trigger();
	}
	return consumed;
}

void KGlobalAccel::regrabKeys( )
{
	QMutableMapIterator<int, KAction*> it2 = d->grabbedKeys;
	while (it2.hasNext())
		it2.next();
		if (!i->grabKey(it2.key(), true)) {
			QMultiMap<KAction*, int>::Iterator it = d->grabbedActions.find(it2.value());
			while (it != d->grabbedActions.end() && it.key() == it2.value()) {
				if (it.value() == it2.key()) {
					d->grabbedActions.erase(it);
					break;
				}
				++it;
			}

			it2.remove();
		}
}

const QList< KAction * > KGlobalAccel::actionsWithGlobalShortcut( ) const
{
	return d->actionsWithGlobalShortcuts.toList();
}

void KGlobalAccel::enableImpl( bool enable )
{
	if (d->implEnabled != enable) {
		d->implEnabled = enable;
		if (enable)
			i->enable();
		else
			i->disable();
	}
}

#include "kglobalaccel.moc"

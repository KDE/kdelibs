/*
 * Copyright (c) 2000,2001 Alex Zepeda <jazepeda@pacbell.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Redistributions of source code or in binary form must consent to
 *    future terms and conditions as set forth by the founding author(s).
 *    The founding author is defined as the creator of following code, or
 *    lacking a clearly defined creator, the founding author is defined as
 *    the first person to claim copyright to, and contribute significantly
 *    to the following code.
 * 4. The following code may be used without explicit consent in any
 *    product provided the previous three conditions are met, and that
 *    the following source code be made available at no cost to consumers
 *    of mentioned product and the founding author as defined above upon
 *    request.  This condition may at any time be waived by means of 
 *    explicit written consent from the founding author.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include <qstring.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlayout.h>

#include <klocale.h>
#include <klibloader.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kpropsdlg.h>

#include "acl.h"
#include "kaclmanipulation.h"

class KACLPrivate {
public:
	QFrame *frame;
	QVBoxLayout *top_layout;

	QHBoxLayout *u_layout, *g_layout;
	QLabel *u_label, *g_label;
	QListView *u_lv, *g_lv;
	KPOSIX1e::KACLList acl;
	KPropertiesDialog *props;
};

KACLPlugin::KACLPlugin (KPropertiesDialog *props, const char *_name)
	: KPropsDlgPlugin (props)
{
	setName(_name);
	kdDebug() << "KACLPlugin::KACLPlugin" << endl;
	pr = new KACLPrivate;

	pr->props = props;

	pr->frame=props->dialog()->addPage(i18n("Extended Permissions"));
	pr->top_layout = new QVBoxLayout(pr->frame);

	pr->u_label = new QLabel(pr->frame, "user access");
	pr->u_label->setText(i18n("Users:"));
	pr->u_lv = new QListView (pr->frame, "main listview");
	pr->u_lv->addColumn(i18n("Username"));
	pr->u_lv->addColumn(i18n("Can Read?"));
	pr->u_lv->addColumn(i18n("Can Write?"));
	pr->u_lv->addColumn(i18n("Can Execute?"));
	pr->top_layout->addWidget(pr->u_label);
	pr->top_layout->addWidget(pr->u_lv);

	pr->u_layout = new QHBoxLayout(pr->top_layout);
	QPushButton *b;
	b = new QPushButton(i18n("Add User"), pr->frame, "b1");
	b->setEnabled(false);
	pr->u_layout->addWidget(b);
	b = new QPushButton(i18n("Edit User"), pr->frame, "b2");
	b->setEnabled(false);
	pr->u_layout->addWidget(b);
	b = new QPushButton(i18n("Delete User"), pr->frame, "b3");
	b->setEnabled(false);
	pr->u_layout->addWidget(b);

	pr->g_label = new QLabel(pr->frame, "group access");
	pr->g_label->setText(i18n("Groups:"));
	pr->g_lv = new QListView (pr->frame, "group listview");
	pr->g_lv->addColumn(i18n("Groupname"));
	pr->g_lv->addColumn(i18n("Can Read?"));
	pr->g_lv->addColumn(i18n("Can Write?"));
	pr->g_lv->addColumn(i18n("Can Execute?"));
	pr->top_layout->addWidget(pr->g_label);
	pr->top_layout->addWidget(pr->g_lv);

	pr->g_layout = new QHBoxLayout(pr->top_layout);
	QPushButton *gb;
	gb = new QPushButton(i18n("Add Group"), pr->frame, "gb1");
	gb->setEnabled(false);
	pr->g_layout->addWidget(gb);
	gb = new QPushButton(i18n("Edit Group"), pr->frame, "gb2");
	gb->setEnabled(false);
	pr->g_layout->addWidget(gb);
	gb = new QPushButton(i18n("Delete Group"), pr->frame, "gb3");
	gb->setEnabled(false);
	pr->g_layout->addWidget(gb);

	LoadACL();
}

KACLPlugin::~KACLPlugin ()
{
        delete pr;
}

void KACLPlugin::LoadACL()
{
	pr->acl.SetPath(pr->props->item()->url().path().latin1());
	pr->acl.ReadACL();
	KPOSIX1e::KACLEntry *ent;

	QString s_yes(i18n("Yes"));
	QString s_no(i18n("No"));
	for (ent = pr->acl.entries.first(); ent != 0; ent = pr->acl.entries.next()) {
		if (ent->qualifier == QString::null) {
			continue;
		}
		if (ent->tag == KPOSIX1e::KACLEntry::USER) {
			(void)new QListViewItem(pr->u_lv, ent->qualifier, 
				(ent->access_r) ? s_yes : s_no,
				(ent->access_w) ? s_yes : s_no,
				(ent->access_x) ? s_yes : s_no
			);
		}else if (ent->tag == KPOSIX1e::KACLEntry::GROUP) {
			(void)new QListViewItem(pr->g_lv, ent->qualifier,
				(ent->access_r) ? s_yes : s_no,
				(ent->access_w) ? s_yes : s_no,
				(ent->access_x) ? s_yes : s_no
			);
		}
	}
}

// Module factory stuff.. should be automated

KACLPluginFactory::KACLPluginFactory (QObject *parent, const char *name)
	: KLibFactory (parent, name)
{
}

KACLPluginFactory::~KACLPluginFactory()
{
}

QObject *KACLPluginFactory::createObject (QObject *parent, const char *name, const char *classname, const QStringList &)
{
	if ( strcmp( classname, "KPropsDlgPlugin" ) == 0 ) {
		assert(parent);
		if ( !parent->inherits( "KPropertiesDialog" ) ) {
			kdDebug() << "Passed wrong arguments" << endl;
			return 0L;
		}
		QObject *obj = new KACLPlugin (static_cast<KPropertiesDialog *>(parent), name);
		return obj;
	}
	return 0L;
}

extern "C" {
	void *init_libkacl_prop_page() {
		return new KACLPluginFactory();
	}
}

#include "acl.moc"

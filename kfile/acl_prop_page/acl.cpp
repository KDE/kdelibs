// $Id$

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

KACLPlugin::KACLPlugin (KPropertiesDialog *props)
	: KPropsDlgPlugin (props)
{
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

QObject *KACLPluginFactory::create (QObject *parent, const char *name, const char *classname, const QStringList &)
{
	kdDebug() << "KACLPluginFactory::create" << endl;
	if ( strcmp( classname, "KPropsDlgPlugin" ) == 0 ) {
		assert( parent );
		if ( !parent->inherits( "KPropertiesDialog" ) ) {
			kdDebug() << "Passed wrong arguments" << endl;
			return 0L;
		}
		QObject *obj = new KACLPlugin (static_cast<KPropertiesDialog *>( parent ));
		emit objectCreated( obj );
		return obj;
	}
	return 0L;
}

extern "C" {
	void *init_libkacl_prop_page() {
		kdDebug() << "init_kacl_prop_page has been called" << endl;
		return new KACLPluginFactory();
	}
}

#include "acl.moc"

#include "kppdgroupitem.h"
#include "kppdoptionitem.h"
#include "icons.h"

#include <kiconloader.h>

KPPDGroupItem::KPPDGroupItem(global_ppd_file_t *ppd, ppd_group_t *group, QListViewItem *parent)
	: KPPDBaseItem(ppd,parent)
{
	group_ = group;
	QString	str = QString::fromLatin1(getGroupIconName(group->text));
	setPixmap(0,UserIcon(str));
	setOpen(true);

	// initialize "changeable" : false if group name contains "install" (case insensitive)
	bool	changeable(true);
	if (QString(group_->text).find("install",0,false) != -1) changeable = false;

	setText(0,group_->text);

	// parse options
	for (int i=group_->num_options-1;i>=0;i--)
	{
		ppd_option_t	*opt = group_->options+i;
		if (strcmp(opt->keyword,"PageRegion") == 0) continue;
		new KPPDOptionItem(ppd,opt,this,changeable);
	}

	// parse sub-groups
	for (int i=group_->num_subgroups-1;i>=0;i--)
	{
		ppd_group_t	*subgroup = group_->subgroups+i;
		new KPPDGroupItem(ppd,subgroup,this);
	}
}

KPPDGroupItem::~KPPDGroupItem()
{
}

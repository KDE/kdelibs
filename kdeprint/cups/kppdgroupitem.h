#ifndef KPPDGROUPITEM_H
#define	KPPDGROUPITEM_H

#include "kppdbaseitem.h"

#include <qdict.h>

class KPPDGroupItem : public KPPDBaseItem
{
public:
	KPPDGroupItem(global_ppd_file_t *ppd, ppd_group_t *group, QListViewItem *parent);
	~KPPDGroupItem();

// members
	ppd_group_t	*group_;
};

#endif

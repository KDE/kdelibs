#ifndef JOBITEM_H
#define JOBITEM_H

#include <qlistview.h>
#include "kmobject.h"

class KMJob;

class JobItem : public QListViewItem, public KMObject
{
public:
	JobItem(QListView *parent, KMJob *job = 0);
	void init(KMJob *job);

	int jobID() const;
	KMJob* job() const;

private:
	KMJob	*m_job;
	int	m_ID;
};

inline int JobItem::jobID() const
{ return m_ID; }

inline KMJob* JobItem::job() const
{ return m_job; }

#endif

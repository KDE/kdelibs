#include "jobitem.h"
#include "kmjob.h"

#include <kiconloader.h>

JobItem::JobItem(QListView *parent, KMJob *job)
: QListViewItem(parent)
{
	init(job);
}

void JobItem::init(KMJob *job)
{
	m_job = job;
	if (m_job)
	{
		setPixmap(0,SmallIcon(KMJob::pixmap(m_job->state())));
		setText(0,QString::number(m_job->id()));
		setText(1,m_job->printer());
		setText(2,m_job->name());
		setText(3,m_job->owner());
		setText(4,KMJob::stateString(m_job->state()));
		setText(5,QString::number(m_job->size()));
		m_ID = m_job->id();
	}
	widthChanged();
}

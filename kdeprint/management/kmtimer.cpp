#include "kmtimer.h"
#include "kmmainview.h"

KMMainView* KMTimer::m_view = 0;
int KMTimer::m_count = 0;

void KMTimer::blockTimer()
{
	if (m_view)
		if ((m_count++) == 0)
			m_view->stopTimer();
}

void KMTimer::releaseTimer(bool refresh)
{
	if (m_view)
		if ((--m_count) == 0)
			if (refresh) m_view->slotTimer();
			else m_view->startTimer();
}

void KMTimer::setMainView(KMMainView *v)
{
	m_view = v;
}

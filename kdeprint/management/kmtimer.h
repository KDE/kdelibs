#ifndef KMTIMER_H
#define KMTIMER_H

class KMMainView;

class KMTimer
{
public:
	static void blockTimer();
	static void releaseTimer(bool refresh = false);
	static void setMainView(KMMainView*);
private:
	static KMMainView	*m_view;
	static int		m_count;
};

#endif

#ifndef KJOB_H
#define KJOB_H

#include <qstring.h>

struct KJob
{
	enum JobAction {
		Remove = 0x01,
		Move   = 0x02,
		Hold   = 0x04,
		Resume = 0x08,
		All    = 0xFF
	};
	enum JobState {
		Printing = 1,
		Queued   = 2,
		Held     = 3,
		Error    = 4,
		Unknown  = 5
	};

	KJob() : ID(-1), state(KJob::Error), size(0), discarded(false) {}

	// public members
	int	ID;
	QString	name;
	QString	printer;
	QString	user;
	int	state;
	int	size;

	// internal members
	bool	discarded;
	QString	uri;
};

#endif

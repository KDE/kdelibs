#ifndef KMJOB_H
#define KMJOB_H

#include <qstring.h>
#include "kmobject.h"

class KMJob : public KMObject
{
public:
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

	KMJob();
	KMJob(const KMJob& j);

	void copy(const KMJob& j);
	static QString pixmap(int state);
	static QString stateString(int state);

	// inline access functions
	int id() const				{ return m_ID; }
	void setId(int id)			{ m_ID = id; }
	const QString& name() const		{ return m_name; }
	void setName(const QString& s)		{ m_name = s; }
	const QString& printer() const		{ return m_printer; }
	void setPrinter(const QString& s)	{ m_printer = s; }
	const QString& owner() const		{ return m_owner; }
	void setOwner(const QString& s)		{ m_owner = s; }
	int state() const			{ return m_state; }
	void setState(int s)			{ m_state = s; }
	int size() const			{ return m_size; }
	void setSize(int s)			{ m_size = s; }
	const QString& uri() const		{ return m_uri; }
	void setUri(const QString& s)		{ m_uri = s; }

protected:
	// normal members
	int	m_ID;
	QString	m_name;
	QString	m_printer;
	QString	m_owner;
	int	m_state;
	int	m_size;

	// internal members
	QString	m_uri;
};

#endif

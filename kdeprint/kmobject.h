#ifndef KMOBJECT_H
#define KMOBJECT_H

class KMObject
{
public:
	KMObject();

	bool isDiscarded() const;
	void setDiscarded(bool on = true);

protected:
	bool m_discarded;
};

inline KMObject::KMObject() : m_discarded(false)
{ }

inline bool KMObject::isDiscarded() const
{ return m_discarded; }

inline void KMObject::setDiscarded(bool on)
{ m_discarded = on; }

#endif

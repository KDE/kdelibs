#ifndef PDQINFOS_H
#define PDQINFOS_H

#include <qstring.h>

class PdqInfos
{
public:
	static PdqInfos* self();

	PdqInfos();
	~PdqInfos();

	QString pdqGlobalRcFile() const;
	void setPdqGlobalRcFile(const QString& d);

private:
	void load();
	void save();

private:
	QString	rcfile_;

	static PdqInfos	*unique_;
};

inline QString PdqInfos::pdqGlobalRcFile() const
{ return rcfile_; }

inline void PdqInfos::setPdqGlobalRcFile(const QString& s)
{ rcfile_ = s; save(); }

#endif

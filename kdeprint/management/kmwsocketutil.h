#ifndef	KMWSOCKETUTIL_H
#define	KMWSOCKETUTIL_H

#include <qstring.h>
#include <qlist.h>

struct SocketInfo
{
	QString	IP;
	QString	Name;
	int	Port;
};

class QProgressBar;
class SocketConfig;
class QWidget;

class KMWSocketUtil
{
	friend class SocketConfig;

public:
	KMWSocketUtil();

	bool checkPrinter(const char *host, int port);
	bool checkPrinter(unsigned long IP, int port);

	const QList<SocketInfo>* printerList() { return &printerlist_; }
	bool scanNetwork(QProgressBar *bar = 0);
	void configureScan(QWidget *parent = 0);

private:
	QList<SocketInfo>	printerlist_;
	unsigned long		start_, stop_;
	int			port_;
	int			timeout_;	// in milliseconds
};

#endif

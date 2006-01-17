#ifndef KWALLETASYNC_H
#define KWALLETASYNC_H

#include <qobject.h>

namespace KWallet { class Wallet; }

class WalletReceiver : public QObject
{
	Q_OBJECT
public Q_SLOTS:
	void walletOpened( bool );
};

#endif

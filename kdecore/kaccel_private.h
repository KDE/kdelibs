
/**
 * Accelerator information, similar to an action.
 *
 * It is used internally by @ref KAccel.
 * @internal
 * @deprecated
 */
class KKeyEntry {
 public:
    int aCurrentKeyCode;
    int aDefaultKeyCode;
    int aConfigKeyCode;
    bool bConfigurable;
    bool bEnabled;
    int aAccelId;
    const QObject *receiver;
    const char *member;
    QString descr;
    int menuId;
    QPopupMenu *menu;

    void operator=(const KKeyEntry& e) {
      aCurrentKeyCode = e.aCurrentKeyCode;
      aDefaultKeyCode = e.aDefaultKeyCode;
      aConfigKeyCode = e.aConfigKeyCode;
      bConfigurable = e.bConfigurable;
      bEnabled = e.bEnabled;
      aAccelId = e.aAccelId;
      receiver = e.receiver;
      member = e.member;
      descr = e.descr;
      menuId = e.menuId;
      menu = e.menu;
    }

    KKeyEntry() {
      aCurrentKeyCode = 0;
      aDefaultKeyCode = 0;
      aConfigKeyCode = 0;
      bConfigurable = false;
      bEnabled = false;
      aAccelId = 0;
      receiver = 0;
      member = 0;
      menuId = 0;
      menu = 0;
    }
    KKeyEntry(const KKeyEntry& e) {
      *this = e;
    }
};

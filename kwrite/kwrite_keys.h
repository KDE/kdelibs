// $Id$

#ifndef _KWRITE_KEYS_H_
#define _KWRITE_KEYS_H_

#include <qobject.h>
#include <qlist.h>
#include <qlineedit.h>
#include <qmenudata.h>
#include <qdialog.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

//#include <X11/Xlib.h> //used to have the XEvent type


class KWCommand;
class KWCommandGroup;
class KWCommandDispatcher;
class KWCommandData;
class KWCommandGroupData;
class KWKeyData;


struct KWAccel {
  bool equals(int kc1, int kc2);
  int keyCode1;
  int keyCode2;
};

struct KWAccelMatch {
  int unmatchedModifiers;
  int id;
  KWCommand *command;
  KWCommandGroup *group;
  bool consumeKeyEvent;
  bool second;
};


class KWCommand : public QObject {
    Q_OBJECT
    friend KWCommandGroup;
    friend KWCommandDispatcher;
  public:
    KWCommand(int id, const QString &name, KWCommandGroup *);

    const QString &name() {return m_name;}
    int id() {return m_id;}

    int accelCount();
    QString accelString(int pos = 0) {return accelString(m_accels[pos]);}

//    int containsAccel(int keyCode1, int keyCode2);
    void addAccel(int keyCode1, int keyCode2);
//    void removeAccel(int pos);
//    void removeAccel(int keyCode1, int keyCode2);

//    QString groupName();

//    void makeDefault();
//    void restoreDefault();
//    void saveAccels();
//    void restoreAccels();
//    void changeAccels();

    void getData(KWCommandData *data);
    void setData(const KWCommandData *data);

    void readConfig(KConfig *);
    void writeConfig(KConfig *);

    void emitAccelString();

    static QString accelString(KWAccel &);
    static const int nAccels = 4;

  signals:
    void changed(const QString &accelString);
    void activated(int id);

  protected:
    void matchKey(int keyCode, int modifiers, KWAccelMatch &match);

    int m_id;
    QString m_name;
    KWCommandGroup *m_group;

    KWAccel m_accels[nAccels];
    bool m_second[nAccels];
//    KGuiCmdAccel oldAccels[nAccels]; //for "cancel" in the dialog
//    KGuiCmdAccel defaultAccels[nAccels];
};

class KWCommandGroup : public QObject {
    Q_OBJECT
    friend KWCommand;
    friend KWCommandDispatcher;
  public:
    KWCommandGroup(const QString &name);
    void setSelectModifiers(int selectModifiers, int selectFlag,
      int selectModifiers2, int selectFlag2);
    void addCommand(KWCommand *command);
    void addCommand(int id, const QString &name, int keyCode01 = 0,
      int keyCode11 = 0, int keyCode21 = 0);
    const QString &name() {return m_name;}

    void getData(KWCommandGroupData *data);
    void setData(const KWCommandGroupData *data);

    void readConfig(KConfig *);
    void writeConfig(KConfig *);

  signals:
    void activated(int id);

  protected:
    void matchKey(int keyCode, int modifiers, KWAccelMatch &match);

    QString m_name;
    int m_selectModifiers, m_selectFlag;
    int m_selectModifiers2, m_selectFlag2;
    QList<KWCommand> m_commandList;
};

class KWCommandDispatcher : public QObject {
    Q_OBJECT
  public:
    KWCommandDispatcher(QObject *host);

    void addGroup(KWCommandGroup *group);
    void setEnabled(bool enabled) {m_enabled = enabled;}

    void getData(KWKeyData *data);
    void setData(const KWKeyData *data);

    void readConfig(KConfig *);
    void writeConfig(KConfig *);

//  protected slots:
//    void destroy();

  protected:
    virtual bool eventFilter(QObject *, QEvent *);

    QList<KWCommandGroup> m_groupList;
    bool m_enabled;
    bool m_second;
};



class KWCommandData {
    friend KWCommand;
  public:

    const QString &name() {return m_name;}
    int accelCount();
    QString accelString(int pos) {return KWCommand::accelString(m_accels[pos]);}
    int containsAccel(int keyCode1, int keyCode2);
    void addAccel(int keyCode1, int keyCode2);
    void removeAccel(int pos);
    void removeAccel(int keyCode1, int keyCode2);

  protected:
//    void setData(const QString &name, KWAccel *accels);

    QString m_name;
    KWAccel m_accels[KWCommand::nAccels];
};

class KWCommandGroupData {
    friend KWCommandGroup;
  public:
    ~KWCommandGroupData();

    const QString &name() {return m_name;}
    int commandCount() {return m_count;}
    KWCommandData *command(int i) {return &m_commandList[i];}
    KWCommandData *containsAccel(int keyCode1, int keyCode2);
    void removeAccel(int keyCode1, int keyCode2);

  protected:
//    void setData(const QString &name, KWCommandData *, int count);

    QString m_name;
    KWCommandData *m_commandList;
    int m_count;
};

class KWKeyData {
    friend KWCommandDispatcher;

    struct Contains {
      KWCommandData *command;
      KWCommandGroupData *group;
    };

  public:
    ~KWKeyData();

    int groupCount() {return m_count;}
    KWCommandGroupData *group(int i) {return &m_groupList[i];}
    void containsAccel(int keyCode1, int keyCode2, Contains &);
    void removeAccel(int keyCode1, int keyCode2);

  protected:
//    setData(KWCommandGroupData *, int count);

    KWCommandGroupData *m_groupList;
    int m_count;
};





/** 
  A class for accelerator input
*/
class KAccelInput : public QLineEdit
{
    Q_OBJECT
  public:
    KAccelInput(QWidget *parent);
    int getKeyCode1();
    int getKeyCode2();
    void clear();
  signals:
    void valid(bool);
    void changed();
//  // not compiled public slots:
//    void removeModifiers();
  protected:
    void setContents();
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

    int pos;
    int keyCodes[2];
};

class KWKeyConfigTab : public QWidget {
    Q_OBJECT
  public:
    KWKeyConfigTab(QWidget *parent, KWKeyData &, const char *name = 0L);
    void updateAccelList();

  protected slots:
    void groupSelected(int);
    void commandSelected(int);
    void accelSelected(int);
    void accelChanged();
    void assign();
    void remove();
    void defaults();

  protected:
    QListBox *m_commandList;
    QListBox *m_accelList;
    KAccelInput *m_accelInput;
    QLabel *m_currentBinding;
    QPushButton *m_assignButton;
    QPushButton *m_removeButton;
    KWKeyData &m_keyData;
    KWCommandGroupData *m_group;
    KWCommandData *m_command;
};




#endif // KWRITE_KEYS_H

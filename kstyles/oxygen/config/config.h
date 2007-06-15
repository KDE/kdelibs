#ifndef CONFIG_H
#define CONFIG_H

#include "ui_config.h"
#include <QDialog>

class Config : public QDialog
{
   Q_OBJECT
public:
   Config(QWidget *parent = 0L);
protected:
   bool eventFilter ( QObject * watched, QEvent * event );
private:
   Ui::Config ui;
   bool infoItemHovered;
   void generateColorModes(QComboBox *box);
   void setDefaults();
   void resetValues();
   void save();
   void loadSettings();
private slots:
   void resetInfo();
   void handleButton(QAbstractButton *);
   void setBgModeInfo(int index);
   void setTabTransInfo(int index);
   void bgModeSet(int bgMode);
};

#endif

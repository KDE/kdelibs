
#ifndef SSK_KACTCFGITEM_H
#define SSK_KACTCFGITEM_H

class QCheckBox;
class KIconLoaderButton;

/**
 * An item in the action configuration widget.
 * @internal
 */
class KActCfgListItem : public QListViewItem
{
public:
	KActCfgListItem( QListView *view, KAction *action );
	virtual ~KActCfgListItem();
	void setAllText();

	void loadFromAction();
	void updateAction();

	KAction *action;
	bool dirty;

	int accel;
	QIconSet *icon;
	QString iconPath;

};

class KActItemCfg : public QWidget
{
public:
	KActItemCfg( QWidget *parent = 0 );
	virtual ~KActItemCfg() {}

	KIconLoaderButton *_iconBtn;
	QCheckBox *_acNone;
        QCheckBox *_acDef;
        QCheckBox *_acCustom;
};

#endif

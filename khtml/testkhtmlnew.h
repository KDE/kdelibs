#ifndef TESTKHTMLNEW_H
#define TESTKHTMLNEW_H

#include <kmainwindow.h>

class KHTMLPart;
class QComboBox;
class QLineEdit;
class KPushButton;
class KUrl;
class QLabel;
class QMovie;

namespace KParts
{
    class URLArgs;
}

/**
 * @internal
 */
class TestKHTML : public KMainWindow
{
    Q_OBJECT
public:
    TestKHTML();
    ~TestKHTML();

    KHTMLPart *doc() const;

public slots:
    void openUrl(const KUrl &url, const KParts::URLArgs &args);
    void openUrl(const KUrl &url);
    void openUrl(const QString &url);
    void openUrl();

    void reload();
    void toggleNavigable(bool s);
    void toggleEditable(bool s);

private slots:
    void startLoading();
    void finishedLoading();

private:
    void setupActions();

    KHTMLPart *m_part;
    KComboBox *m_combo;
    QToolButton *m_goButton;
    QToolButton *m_reloadButton;
    QLineEdit *m_comboEdit;
    QLabel *m_indicator;
    QMovie *m_movie;

#ifndef __KDE_HAVE_GCC_VISIBILITY
    KHTMLFactory *m_factory;
#endif

};

#endif

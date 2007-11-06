#ifndef KXMLGUIFILEMERGER_P_H
#define KXMLGUIFILEMERGER_P_H

#include <QStringList>

/**
 * @internal
 * Helper class for KXMLGUIClient::setXMLFile
 * Finds the xml file with the largest version number and takes
 * care of keeping user settings (from the most local file)
 * like action shortcuts or toolbar customizations.
 */
class KXmlGuiFileMerger
{
public:
    KXmlGuiFileMerger(const QStringList& files);

    QString finalFile() const { return m_file; }
    QString finalDocument() const { return m_doc; }

private:
    QString m_file;
    QString m_doc;
};


#endif /* KXMLGUIFILEMERGER_P_H */


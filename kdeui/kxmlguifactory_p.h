#ifndef __kxmlguifactory_p_h__
#define __kxmlguifactory_p_h__

#include <qstringlist.h>
#include <qmap.h>

#include <kaction.h>

class QWidget;
class KXMLGUIClient;
class KXMLGUIBuilder;

namespace KXMLGUI
{

/*
 * This structure is used to know to which client certain actions and custom elements
 * (i.e. menu separators) belong.
 * We do not only use a ContainerClient per GUIClient but also per merging group.
 *
 * groupName : Used for grouped merging. Specifies the group name to which these actions/elements
 * belong to.
 * actionLists : maps from action list name to action list.
 * mergingName : The (named) merging point.
 *
 * A ContainerClient always belongs to a ContainerNode.
 */
struct ContainerClient
{
    KXMLGUIClient *client;
    QPtrList<KAction> actions;
    QValueList<int> customElements;
    QString groupName; //is empty if no group client
    QMap<QString, QPtrList<KAction> > actionLists;
    QString mergingName;
};

class ContainerNode;

struct MergingIndex
{
    int value; // the actual index value, used as index for plug() or createContainer() calls
    QString mergingName; // the name of the merging index (i.e. the name attribute of the
                         // Merge or DefineGroup tag)
    QString clientName; // the name of the client that defined this index
};

/*
 * Here we store detailed information about a container, its clients (client=a guiclient having actions
 * plugged into the container), child nodes, naming information (tagname and name attribute) and
 * merging index information, to plug/insert new actions/items a the correct position.
 *
 * The builder variable is needed for using the proper GUIBuilder for destruction ( to use the same for
 * con- and destruction ). The builderCustomTags and builderContainerTags variables are cached values
 * of what the corresponding methods of the GUIBuilder which built the container return. The stringlists
 * is shared all over the place, so there's no need to worry about memory consumption for these
 * variables :-)
 *
 * The mergingIndices list contains the merging indices ;-) , as defined by <Merge>, <DefineGroup>
 * or by <ActionList> tags. The order of these index structures within the mergingIndices list
 * is (and has to be) identical with the order in the DOM tree.
 *
 * Beside the merging indices we have the "real" index of the container. It points to the next free
 * position.
 * (used when no merging index is used for a certain action, custom element or sub-container)
 */
struct ContainerNode
{
    ContainerNode( QWidget *_container, const QString &_tagName, const QString &_name,
                   ContainerNode *_parent = 0L, KXMLGUIClient *_client = 0L,
                   KXMLGUIBuilder *_builder = 0L, int id = -1,
                   const QString &_mergingName = QString::null,
                   const QString &groupName = QString::null,
                   const QStringList &customTags = QStringList(),
                   const QStringList &containerTags = QStringList() );

    ContainerNode *parent;
    KXMLGUIClient *client;
    KXMLGUIBuilder *builder;
    QStringList builderCustomTags;
    QStringList builderContainerTags;
    QWidget *container;
    int containerId;

    QString tagName;
    QString name;

    QString groupName; //is empty if the container is in no group

    QPtrList<ContainerClient> clients;
    QPtrList<ContainerNode> children;

    int index;
    QValueList<MergingIndex> mergingIndices;

    QString mergingName;

    QValueList<MergingIndex>::Iterator findIndex( const QString &name );
    ContainerNode *findContainerNode( QWidget *container );
    ContainerNode *findContainer( const QString &_name, bool tag );
    ContainerNode *findContainer( const QString &name, const QString &tagName,
                                  const QPtrList<QWidget> *excludeList,
                                         KXMLGUIClient *currClient );
    void adjustMergingIndices( int offset, const QValueList<MergingIndex>::Iterator &it );
};

};

#endif
/* vim: et sw=4
 */

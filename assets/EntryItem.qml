import bb.cascades 1.2
import org.keepassb 1.0
import "common.js" as Common

GroupListItem {
    id: groupListEntryItem
    
    property variant kpb: Qt.kpb // references to important globals (app, database, etc)
        
    signal editRequested(variant selectedEntry)
    signal deleteRequested(variant selectedEntry)
    
    itemType: "entry"
    title: ListItemData.title
    description: Common.getEntryDescription(ListItemData)
    imageSource: ListItemData.expired ? "asset:///images/ic_expired_item.png" : "asset:///pwicons/" + ListItemData.iconId + ".png"
    contextActions: ActionSet {
        title: ListItemData.title
        actions: [
            ActionItem {
                title: qsTr("Multi-Copy", "A button/action which copies the whole entry into clipboard (also see 'Multi-Copy' in thesaurus).") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_multi_copy.png"
                onTriggered: {
                    Common.performMultiCopy(ListItemData);
                }
            },
            ActionItem {
                title: qsTr("Copy User Name", "A button/action which copies user_name value to the clipboard. Here, 'User Name' refers to login information rather that person's own name.") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_copy_username.png"
                onTriggered: {
                    kpb.app.copyWithTimeout(ListItemData.userName);
                }
            },
            ActionItem {
                title: qsTr("Copy Password", "A button/action which copies password password value to the clipborad.") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_copy_password.png"
                onTriggered: {
                    kpb.app.copyWithTimeout(ListItemData.password);
                }
            },
            ActionItem {
                title: qsTr("Edit Entry", "A button/action to edit the selected entry") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_edit.png"
                enabled: kpb.database.isEditable() && !ListItemData.deleted 
                onTriggered: editRequested(ListItemData)
            },
            DeleteActionItem {
                title: qsTr("Delete", "A button/action to delete an entry") + Retranslate.onLocaleOrLanguageChanged
                enabled: kpb.database.isEditable() && !ListItemData.deleted
                onTriggered: deleteRequested(ListItemData)
            }
        ]
    }
}
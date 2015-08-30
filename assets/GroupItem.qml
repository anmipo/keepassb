import bb.cascades 1.2
import org.keepassb 1.0

GroupListItem {
    id: groupListGroupItem
    
    property variant kpb: Qt.kpb // references to important globals (app, database, etc)

    signal editRequested(variant selectedGroup)
    signal deleteRequested(variant selectedGroup)

    itemType: "group"
    title: ListItemData.name
    imageSource: ListItemData.expired ? "asset:///images/ic_expired_item.png" : "asset:///pwicons/" + ListItemData.iconId + ".png"
    counterText: "(" + ListItemData.itemsCount + ")"

    contextActions: ActionSet {
        title: ListItemData.name
        ActionItem {
            title: qsTr("Edit Group", "A button/action to edit the selected group") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_edit_group.png"
            enabled: kpb.database.isEditable() && !ListItemData.deleted 
            onTriggered: editRequested(ListItemData)
        }
        DeleteActionItem {
            title: qsTr("Delete", "A button/action to delete a group") + Retranslate.onLocaleOrLanguageChanged
            enabled: kpb.database.isEditable() && !ListItemData.deleted
            onTriggered: deleteRequested(ListItemData)
        }
    }
}
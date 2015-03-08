/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import org.keepassb 1.0
import "common.js" as Common

ScrollView {
    property bool hasHistory: (database.getFormatVersion() == 4)
    Container {
        LabelTextButton { 
            labelText: qsTr("Expiry Date", "Label of a field with date and time when the entry will no longer be valid. 'Never' is also a possible value.") + Retranslate.onLocaleOrLanguageChanged
            valueText: entry.expires ? Common.timestampToString(entry.expiryTime) : qsTr("Never", "Expiry Date of the entry which does not expire.")
        }
        LabelTextButton { 
            labelText: qsTr("Creation Date", "Label of a field with entry creation date and time") + Retranslate.onLocaleOrLanguageChanged
            valueText: Common.timestampToString(entry.creationTime)  
        }
        LabelTextButton { 
            labelText: qsTr("Last Modification Date", "Label of a field with entry's last modification date and time") + Retranslate.onLocaleOrLanguageChanged
            valueText: Common.timestampToString(entry.lastModificationTime)  
        }
        LabelTextButton { 
            labelText: qsTr("Last Access Date", "Label of a field with date and time when the entry was last accessed/viewed") + Retranslate.onLocaleOrLanguageChanged
            valueText: Common.timestampToString(entry.lastAccessTime)
        }
        Header {
            title: qsTr("Previous Versions", "Header of a list with previous versions/revisions of an entry.") + Retranslate.onLocaleOrLanguageChanged
            visible: hasHistory
        }
        Label {
            text: qsTr("There are no previous versions.", "Explanation for the empty list of previous entry versions/revisions.")
            visible: hasHistory && (entry.historySize == 0)
        }
        ListView {
            id: entryHistoryList
            visible: hasHistory && (entry.historySize > 0)
            scrollRole: ScrollRole.Main
            preferredHeight: 360
            dataModel: hasHistory ? entry.getHistoryDataModel() : null
            onTriggered: {
                var item = dataModel.data(indexPath);
                var viewHistoryEntryPage = Qt.createComponent("ViewEntryPage.qml");
                var historyEntryPage = viewHistoryEntryPage.createObject(null, {"entry": item, "editable": false});
                naviPane.push(historyEntryPage);
            }
            function updateHeight(itemHeight) {
                entryHistoryList.maxHeight = itemHeight * entry.historySize;
            }
            listItemComponents: [
                ListItemComponent {
                    GroupListItem {
                        id: entryHistoryListItem
                        itemType: "entry"
                        title: ListItemData.title
                        description: ListItemData.lastModificationTime.toString()
                        imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
                        attachedObjects: [
                            // ensures the list is large enough to fit all the entries, but no more than that
                            LayoutUpdateHandler {
                                onLayoutFrameChanged: {
                                    entryHistoryListItem.ListItem.view.updateHeight(layoutFrame.height);
                                }
                            }
                        ]
                    }
                }
            ]
        }
    }
}

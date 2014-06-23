/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import org.keepassb 1.0

Page {
    property PwGroupV4 group

    titleBar: TitleBar { }
    
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    actions: [
        ActionItem {
            title: qsTr("Search") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_search.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                searchField.visible = true;
                searchField.requestFocus();
            }
        }
    ]
    
	onGroupChanged: {
        titleBar.title = group.name;
    }
    
    Container {
        layout: DockLayout { }
        Container {
            layout: StackLayout { }
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            TextField {
                id: searchField
                hintText: qsTr("Search") + Retranslate.onLocaleOrLanguageChanged
                visible: false
                inputMode: TextFieldInputMode.Text
                input.submitKey: SubmitKey.Search
                input.submitKeyFocusBehavior: SubmitKeyFocusBehavior.Lose
                input.onSubmitted: {
                    searchField.visible = false;
                    searchField.text = "";
                    database.search(searchField.text);
                    var searchResultsPageComponent = Qt.createComponent("SearchResultsPage.qml");
                    var searchResultsPage = searchResultsPageComponent.createObject(null, 
                            {"searchResult": database.searchResult});
                    naviPane.push(searchResultsPage);
                }
            }
            ListView {
                id: groupList
                objectName: "groupList"
                dataModel: group
                visible: !group.isEmpty() 
                scrollRole: ScrollRole.Main
                accessibility.name: qsTr("Password list") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    var itemType = group.itemType(indexPath);
                    var item = group.data(indexPath);
                    if (itemType == "group") {
                        var viewSubGroupPage = Qt.createComponent("ViewGroupPage.qml");
                        var subGroupPage = viewSubGroupPage.createObject(null, {"group": item});
                        naviPane.push(subGroupPage);
                    } else if (itemType == "entry") {
                        var viewEntryPage = Qt.createComponent("ViewEntryV4Page.qml");
                        var entryPage = viewEntryPage.createObject(null, {"data": item});
                        naviPane.push(entryPage);
                    } else {
                        console.log("WARN: unknown item type");
                    }
                }
                listItemComponents: [
                    ListItemComponent {
                        type: "entry"
                        StandardListItem {
                            title: ListItemData.title
                            description: ListItemData.userName
                            imageSpaceReserved: true
                            imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
                            accessibility.name: qsTr("Entry") + Retranslate.onLocaleOrLanguageChanged + ListItemData.title
                        }
                    },
                    ListItemComponent {
                        type: "group"
                        StandardListItem {
                            title: ListItemData.name
                            status: "(" + ListItemData.childCount + ")"
                            imageSpaceReserved: true
                            imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
                            accessibility.name: qsTr("Group") + Retranslate.onLocaleOrLanguageChanged + ListItemData.name
                        }
                    },
                    ListItemComponent {
                        type: "none"
                        StandardListItem {
                            visible: false;
                        }
                    }
                ]
            }
        }
        Container {
            layout: StackLayout { }
            visible: group.isEmpty()
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
            ImageView {
                imageSource: "asset:///images/ic_empty_group.png"
                horizontalAlignment: HorizontalAlignment.Center
            }
            Label {
                text: qsTr("This group is empty") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.BodyText
                horizontalAlignment: HorizontalAlignment.Center
            }
        }
        attachedObjects: [
            ComponentDefinition {
                source: "SearchResultsPage.qml"
            },
            ComponentDefinition {
                source: "ViewEntryV4Page.qml"
            }
        ]
    }
}

/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import org.keepassb 1.0

Page {
    property PwGroupV4 group

    onCreationCompleted: {
        app.restartWatchdog();
    }

    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                leftPadding: 10
                rightPadding: 10
                ImageView {
                    imageSource: "asset:///pwicons/" + group.iconId + ".png"
                    verticalAlignment: VerticalAlignment.Center
                }
                Label {
                    id: titleLabel
                    text: titleBar.title
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                }
            }
        }
    }
    
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
                // Upon submit, TextField first gets onFocusChanged and then onSubmitted.
                // This property stores the query text between these events.
                property string searchQuery 
                
                id: searchField
                hintText: qsTr("Search") + Retranslate.onLocaleOrLanguageChanged
                visible: false
                clearButtonVisible: true; 
                inputMode: TextFieldInputMode.Text
                input.submitKey: SubmitKey.Search
                input.submitKeyFocusBehavior: SubmitKeyFocusBehavior.Lose
                onTextChanging: {
                    app.restartWatchdog();
                }
                onFocusedChanged: {
                    if (!focused) {
                        searchQuery = text;
                        visible = false;
                        text = "";
                    }
                }
                input.onSubmitted: {
                    searchField.visible = false;
                    searchField.text = "";
                    database.search(searchQuery);
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
                        }
                    },
                    ListItemComponent {
                        type: "group"
                        StandardListItem {
                            title: ListItemData.name
                            status: "(" + ListItemData.childCount + ")"
                            imageSpaceReserved: true
                            imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
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

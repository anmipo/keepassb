/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import org.keepassb 1.0
import "common.js" as Common

Page {
    property PwGroup group
    property bool autofocus: false

    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                leftPadding: 10
                rightPadding: 10
                ImageView {
                    imageSource: "asset:///pwicons-dark/" + group.iconId + ".png"
                    verticalAlignment: VerticalAlignment.Center
                }
                Label {
                    id: titleLabel
                    text: titleBar.title
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    textStyle.color: Color.White
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                }
            }
            expandableArea.indicatorVisibility: (group.name.length > 30) ? TitleBarExpandableAreaIndicatorVisibility.Visible : TitleBarExpandableAreaIndicatorVisibility.Hidden
            expandableArea.toggleArea: TitleBarExpandableAreaToggleArea.EntireTitleBar
            expandableArea.content: TextArea {
                editable: false
                text: group.name
                textFormat: TextFormat.Plain
                backgroundVisible: false
                autoSize.maxLineCount: 3
            }
        }
    }
    
    function performAutofocus() {
        searchField.requestFocus();
    }
    function startSearch() {
        searchContainer.visible = true;
        performAutofocus();
    }
    function cancelSearch() {
        searchContainer.visible = false;
        searchField.searchQuery = searchField.text;
        searchField.text = "";
    }
    
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.Default
    actions: [
        ActionItem {
            title: qsTr("Search") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_search.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: startSearch()
            shortcuts: [
                SystemShortcut {
                    type: SystemShortcuts.Search
                    onTriggered: startSearch()
                }
            ]
        }
    ]
    
    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            id: backButton
            onTriggered: {
                naviPane.pop();
            }
        } 
    }
    
	onGroupChanged: {
        titleBar.title = group.name;
        if (!group.parentGroup) {
            backButton.imageSource = "asset:///images/ic_lock.png";
            backButton.title = qsTr("Lock");
        } 
    }
    
    Container {
        onTouchCapture: {
            app.restartWatchdog();
        }
        
        layout: DockLayout { }
        Container {
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            Container {  // search field
                id: searchContainer
                visible: false
                leftPadding: 10
                rightPadding: 10
                topPadding: 10
                bottomPadding: 10
                horizontalAlignment: HorizontalAlignment.Fill
                layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                ImageView {
                    imageSource: "asset:///images/ic_search.png"
                    horizontalAlignment: HorizontalAlignment.Left
                }
                TextField {
                    // Upon submit, TextField first gets onFocusChanged and then onSubmitted.
                    // This property stores the query text between these events.
                    property string searchQuery 
                    
                    id: searchField
                    hintText: qsTr("Search") + Retranslate.onLocaleOrLanguageChanged
                    horizontalAlignment: HorizontalAlignment.Fill
                    layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                    clearButtonVisible: true 
                    inputMode: TextFieldInputMode.Text
                    input.submitKey: SubmitKey.Search
                    input.submitKeyFocusBehavior: SubmitKeyFocusBehavior.Lose
                    onTextChanging: {
                        app.restartWatchdog();
                    }
                    input.onSubmitted: {
                        searchContainer.visible = false;
                        searchQuery = searchField.text;
                        searchField.text = "";
                        database.search(searchQuery);
                        var searchResultsPageComponent = Qt.createComponent("SearchResultsPage.qml");
                        var searchResultsPage = searchResultsPageComponent.createObject(null, 
                                {"searchResult": database.searchResult});
                        naviPane.push(searchResultsPage);
                    }
                }
                Button {
                    text: qsTr("Cancel") + Retranslate.onLocaleOrLanguageChanged
                    horizontalAlignment: HorizontalAlignment.Right
                    preferredWidth: 50
                    onClicked: {
                        searchField.text = "";
                        searchContainer.visible = false;
                    }
                }
            }
            ListView {
                id: groupList
                objectName: "groupList"
                dataModel: group
                visible: !group.isEmpty()
                horizontalAlignment: HorizontalAlignment.Right
                scrollRole: ScrollRole.Main
                onTriggered: {
                    var itemType = group.itemType(indexPath);
                    var item = group.data(indexPath);
                    if (itemType == "group") {
                        var viewSubGroupPage = Qt.createComponent("ViewGroupPage.qml");
                        var subGroupPage = viewSubGroupPage.createObject(null, {"group": item});
                        naviPane.push(subGroupPage);
                    } else if (itemType == "entry") {
                        var formatVersion = Qt.database.getFormatVersion();
                        var viewEntryPage;
                        if (formatVersion == 3) {
                            viewEntryPage = Qt.createComponent("ViewEntryV3Page.qml");
                        } else {
                            viewEntryPage = Qt.createComponent("ViewEntryV4Page.qml");
                        }
                        var entryPage = viewEntryPage.createObject(null, {"data": item});
                        naviPane.push(entryPage);
                    } else {
                        console.log("WARN: unknown item type");
                    }
                    cancelSearch();
                }
                listItemComponents: [
                    ListItemComponent {
                        type: "entry"
                        StandardListItem {
                            title: ListItemData.title
                            description: Common.getEntryDescription(ListItemData)
                            imageSpaceReserved: true
                            imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
                            contextActions: ActionSet {
                                title: ListItemData.title
                                actions: [
                                    ActionItem {
                                        title: qsTr("Copy User Name") + Retranslate.onLocaleOrLanguageChanged
                                        imageSource: "asset:///images/ic_copy.png"
                                        onTriggered: {
                                            Qt.app.copyWithTimeout(ListItemData.userName);
                                        }
                                    },
                                    ActionItem {
                                        title: qsTr("Copy Password") + Retranslate.onLocaleOrLanguageChanged
                                        imageSource: "asset:///images/ic_copy_password.png"
                                        onTriggered: {
                                            Qt.app.copyWithTimeout(ListItemData.password);
                                        }
                                    }
                                ]
                            }
                        }
                    },
                    ListItemComponent {
                        type: "group"
                        StandardListItem {
                            title: "<b>" + ListItemData.name + "</b>"
                            textFormat: TextFormat.Html
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
                source: "ViewEntryV3Page.qml"
            },
            ComponentDefinition {
                source: "ViewEntryV4Page.qml"
            }
        ]
    }
}

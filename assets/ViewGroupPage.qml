/*
 * Copyright (c) 2014-2016 Andrei Popleteev. 
 * Licensed under the MIT license.
 */

import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common

PageWithWatchdog {
    id: viewGroupPage
    property PwGroup group
    property bool isEmptyGroup: (group.itemsCount == 0)

    onCreationCompleted: {
        group.registerAccessEvent();
    }
    
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
    
    function startSearch() {
        cancelPanels(); // if any
        var searchResultsPageComponent = Qt.createComponent("SearchResultsPage.qml");
        var searchResultsPage = searchResultsPageComponent.createObject(null, {"searchQuery": ""});
        naviPane.push(searchResultsPage);
    }
    /** Hides any opened panels, like search or sort order config. */
    function cancelPanels() {
        sortOrderContainer.visible = false;
    }
    function canCreateEntryHere() {
        // In V3, no entries are allowed in the root group. 
        var rootForbidden = (database.getFormatVersion() == 3);
        return (database.isEditable() && !group.deleted && 
                !(database.isRoot(group) && rootForbidden)); 
    }
    function canCreateGroupHere() {
        return (database.isEditable() && !group.deleted); 
    }
    function createEntry() {
        var newEntry = group.createEntry(); // add an entry to the current group
        var editEntryPageComponent = Qt.createComponent("EditEntryPage.qml");
        var editEntryPage = editEntryPageComponent.createObject(viewGroupPage, {"entry": newEntry, "creationMode": true});
        editEntryPage.open();
        editEntryPage.autofocus();
    }
    function createGroup() {
        var newGroup = group.createGroup(); // add a subgroup to the current group
        var editGroupPageComponent = Qt.createComponent("EditGroupPage.qml");
        var editGroupPage = editGroupPageComponent.createObject(viewGroupPage, {"group": newGroup, "creationMode": true});
        editGroupPage.open();
        editGroupPage.autofocus();
    }
    function showGroup(group) {
        var viewSubGroupPage = Qt.createComponent("ViewGroupPage.qml");
        var subGroupPage = viewSubGroupPage.createObject(null, {"group": group});
        naviPane.push(subGroupPage);
    }
    function showEntry(entry) {
        var viewEntryPageComponent = Qt.createComponent("ViewEntryPage.qml");
        var viewEntryPage = viewEntryPageComponent.createObject(null, {"entry": entry});
        naviPane.push(viewEntryPage);
    }
    function showEditGroupDialog(selGroup) {
        var editGroupPageComponent = Qt.createComponent("EditGroupPage.qml");
        var editGroupSheet = editGroupPageComponent.createObject(viewGroupPage, {"group": selGroup});
        editGroupSheet.open();
        editGroupSheet.autofocus();
    }
    function showEditEntryDialog(selEntry) {
        var editEntryPageComponent = Qt.createComponent("EditEntryPage.qml");
        var editEntrySheet = editEntryPageComponent.createObject(viewGroupPage, {"entry": selEntry});
        editEntrySheet.open();
        editEntrySheet.autofocus()
    }
    function confirmDeleteGroup(selGroup) {
        deleteGroupConfirmationDialog.targetGroup = selGroup;
        deleteGroupConfirmationDialog.show();
    }
    function confirmDeleteEntry(selEntry) {
        deleteEntryConfirmationDialog.targetEntry = selEntry;
        deleteEntryConfirmationDialog.show();
    }
    
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.Default
    actions: [
        ActionItem {
            title: qsTr("Search", "A button/action which opens a search dialog.") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_search.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: startSearch()
            shortcuts: [
                SystemShortcut {
                    type: SystemShortcuts.Search
                    onTriggered: startSearch()
                }
            ]
        },
        ActionItem {
            title: qsTr("Create Entry", "A button/action to create a new entry") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_add_entry.png"
            enabled: canCreateEntryHere()
            ActionBar.placement: (isEmptyGroup ? ActionBarPlacement.OnBar : ActionBarPlacement.Default)
            onTriggered: createEntry()
        },
        ActionItem {
            title: qsTr("Create Group", "A button/action to create a new group") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_add_group.png"
            enabled: canCreateGroupHere()
            ActionBar.placement: (isEmptyGroup ? ActionBarPlacement.OnBar : ActionBarPlacement.Default)
            onTriggered: createGroup()
        },
        ActionItem {
            title: qsTr("Sort Order", "A button/action to show settings which change list sorting order") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_sort.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                cancelPanels(); // if any
                sortOrderContainer.visible = true;
            }
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
            backButton.title = qsTr("Lock", "A button/action which locks/closes the database.");
        }
    }
    
    Container {
        layout: DockLayout { }
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Fill
        Container {
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill

            Container {  // sort order settings
                id: sortOrderContainer
                visible: false
                leftPadding: 10
                rightPadding: 10
                topPadding: 10
                bottomPadding: 10
                horizontalAlignment: HorizontalAlignment.Fill
                layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                onVisibleChanged: {
                    if (visible)
                        sortOrderDropDown.expanded = true;
                }
                DropDown {
                    id: sortOrderDropDown
                    title: qsTr("Sort Order", "A setting which defines sorting of entries in groups. Example: 'Sort order   Title (A..Z)'.") + Retranslate.onLocaleOrLanguageChanged
                    onSelectedOptionChanged: {
                        if (selectedOption) {
                            appSettings.groupSortingType = selectedOption.value;
                            sortOrderContainer.visible = false;
                        }
                    }
                    Option {
                        text: qsTr("None", "One of the possible values of the 'Sort Order' setting. Will be displayed as 'Sort Order   None', meaning no sorting, or as-is item order.") + Retranslate.onLocaleOrLanguageChanged
                        value: Settings.GROUP_SORTING_NONE
                        selected: (appSettings.groupSortingType == Settings.GROUP_SORTING_NONE);
                    }
                    Option {
                        text: qsTr("Title (A..Z)", "One of the possible values of the 'Sort Order' setting. Will be displayed as 'Sort Order    Title'.") + Retranslate.onLocaleOrLanguageChanged
                        value: Settings.GROUP_SORTING_NAME_ASC
                        selected: (appSettings.groupSortingType == Settings.GROUP_SORTING_NAME_ASC);
                    }
                    Option {
                        text: qsTr("Title (Z..A)", "One of the possible values of the 'Sort Order' setting. Will be displayed as 'Sort Order    Title'.") + Retranslate.onLocaleOrLanguageChanged
                        value: Settings.GROUP_SORTING_NAME_DESC
                        selected: (appSettings.groupSortingType == Settings.GROUP_SORTING_NAME_DESC);
                    }
                    Option {
                        text: qsTr("Creation Date (Recent First)", "One of the possible values of the 'Sort Order' setting. Will be displayed as 'Sort Order    Creation Date'.") + Retranslate.onLocaleOrLanguageChanged
                        value: Settings.GROUP_SORTING_CREATION_TIME_DESC
                        selected: (appSettings.groupSortingType == Settings.GROUP_SORTING_CREATION_TIME_DESC);
                    }
                    Option {
                        text: qsTr("Creation Date (Oldest First)", "One of the possible values of the 'Sort Order' setting. Will be displayed as 'Sort Order    Creation Date'.") + Retranslate.onLocaleOrLanguageChanged
                        value: Settings.GROUP_SORTING_CREATION_TIME_ASC
                        selected: (appSettings.groupSortingType == Settings.GROUP_SORTING_CREATION_TIME_ASC);
                    }
                    Option {
                        text: qsTr("Last Modified Date (Recent First)", "One of the possible values of the 'Sort Order' setting. Will be displayed as 'Sort Order    Last Modified Date'.") + Retranslate.onLocaleOrLanguageChanged
                        value: Settings.GROUP_SORTING_LAST_MODIFICATION_TIME_DESC
                        selected: (appSettings.groupSortingType == Settings.GROUP_SORTING_LAST_MODIFICATION_TIME_DESC);
                    }
                    Option {
                        text: qsTr("Last Modified Date (Oldest First)", "One of the possible values of the 'Sort Order' setting. Will be displayed as 'Sort Order    Last Modified Date'.") + Retranslate.onLocaleOrLanguageChanged
                        value: Settings.GROUP_SORTING_LAST_MODIFICATION_TIME_ASC
                        selected: (appSettings.groupSortingType == Settings.GROUP_SORTING_LAST_MODIFICATION_TIME_ASC);
                    }
                }
            }
            ListView {
                id: groupList

                function _showEditGroupDialog(selGroup) {
                    return viewGroupPage.showEditGroupDialog(selGroup);
                }
                function _showEditEntryDialog(selEntry) {
                    return viewGroupPage.showEditEntryDialog(selEntry);
                }
                function _confirmDeleteGroup(selGroup) {
                    return viewGroupPage.confirmDeleteGroup(selGroup);
                }
                function _confirmDeleteEntry(selEntry) {
                    return viewGroupPage.confirmDeleteEntry(selEntry);
                }
                
                dataModel: group
                visible: !isEmptyGroup
                horizontalAlignment: HorizontalAlignment.Right
                scrollRole: ScrollRole.Main
                onTriggered: {
                    var itemType = group.itemType(indexPath);
                    var item = group.data(indexPath);
                    if (itemType == "group") {
                        showGroup(item);
                    } else if (itemType == "entry") {
                        showEntry(item);
                    } else {
                        console.log("WARN: unknown item type");
                    }
                    cancelPanels();
                }
                listItemComponents: [
                    ListItemComponent {
                        type: "entry"
                        EntryItem {
                            id: groupListEntryItem
                            onEditRequested: groupListEntryItem.ListItem.view._showEditEntryDialog(selectedEntry)
                            onDeleteRequested: groupListEntryItem.ListItem.view._confirmDeleteEntry(selectedEntry)
                        }
                    },
                    ListItemComponent {
                        type: "group"
                        GroupItem {
                            id: groupListGroupItem
                            onEditRequested: groupListGroupItem.ListItem.view._showEditGroupDialog(selectedGroup)
                            onDeleteRequested: groupListGroupItem.ListItem.view._confirmDeleteGroup(selectedGroup)
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
            visible: isEmptyGroup
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
            ImageView {
                imageSource: "asset:///images/ic_empty_group.png"
                horizontalAlignment: HorizontalAlignment.Center
            }
            Label {
                bottomMargin: 40
                text: qsTr("This group is empty", "Description for groups without any subgroups or entries.") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.BodyText
                horizontalAlignment: HorizontalAlignment.Center
            }
        }
        attachedObjects: [
            ComponentDefinition {
                source: "SearchResultsPage.qml"
            },
            ComponentDefinition {
                source: "ViewEntryPage.qml"
            },
            SystemDialog {
                property variant targetEntry
                id: deleteEntryConfirmationDialog
                title: qsTr("Delete Entry", "Title of a delete confirmation dialog") + Retranslate.onLocaleOrLanguageChanged
                body: qsTr("Really delete this entry?", "A confirmation dialog for deleting entry") + Retranslate.onLocaleOrLanguageChanged
                confirmButton.label: qsTr("Delete", "A button/action to confirm deletion of an entry") + Retranslate.onLocaleOrLanguageChanged
                onFinished: {
                    if (value == SystemUiResult.ConfirmButtonSelection) {
                        Common.deleteEntry(targetEntry);
                        // refresh the ListView, otherwise it crashes
                        group.itemsChanged(DataModelChangeType.AddRemove, 0);
                    }
                }
            },
            SystemDialog {
                property variant targetGroup
                id: deleteGroupConfirmationDialog
                title: qsTr("Delete Group", "Title of a delete confirmation dialog") + Retranslate.onLocaleOrLanguageChanged
                body: qsTr("Really delete this group, its subgroups and entries?", "A confirmation dialog for deleting a group") + Retranslate.onLocaleOrLanguageChanged
                confirmButton.label: qsTr("Delete", "A button/action to confirm deletion of a group") + Retranslate.onLocaleOrLanguageChanged
                onFinished: {
                    if (value == SystemUiResult.ConfirmButtonSelection) {
                        Common.deleteGroup(targetGroup);
                        // refresh the ListView, otherwise it crashes
                        group.itemsChanged(DataModelChangeType.AddRemove, 0);
                    }
                }
            }
        ]
    }
}

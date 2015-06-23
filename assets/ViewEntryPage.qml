/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.device 1.2
import bb.system 1.2
import bb.cascades.pickers 1.0
import org.keepassb 1.0
import "common.js" as Common

PageWithWatchdog {
    id: viewEntryPage
    property PwEntry entry
    property string currentView
    property bool editable: true // set to false to disable editing controls (e.g. for history entries)
    property bool isExtraFieldsSupported: (database.getFormatVersion() == 4)

    onCreationCompleted: {
        entry.registerAccessEvent();
        if (Common.isAllStandardFieldsEmpty(entry)) {
            titleBar.selectedOption = extraTabOption;
        } else { 
            titleBar.selectedOption = generalTabOption;
        }
    }
    
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    actions: [
        ActionItem {
            title: qsTr("Multi-Copy", "A button/action which copies the whole entry into clipboard (also see 'Multi-Copy' in thesaurus).") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_multi_copy.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                Common.performMultiCopy(entry);
            }
        },
        ActionItem {
            id: addExtraField
            title: qsTr("Add Extra Field", "A button/action to add an extra field to an entry") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_add_field.png"
            enabled: isExtraFieldsSupported && editable && database.isEditable() && !entry.deleted
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                titleBar.selectedOption = extraTabOption;
                viewEntryExtrasTab.onAddExtraField();
            }
        },
        ActionItem {
            id: attachFileAction
            title: qsTr("Attach File", "A button/action to attach a file to an entry") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_add_attachment.png"
            enabled: editable && database.isEditable() && !entry.deleted
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                titleBar.selectedOption = extraTabOption;
                viewEntryExtrasTab.onAddAttachment();
            }
        },
        ActionItem {
            id: editEntryAction
            title: qsTr("Edit Entry", "A button/action to edit an entry") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_edit.png"
            enabled: editable && database.isEditable() && !entry.deleted
            ActionBar.placement: ActionBarPlacement.OnBar
            shortcuts: [
                SystemShortcut {
                    type: SystemShortcuts.Edit
                    enabled: editEntryAction.enabled
                    onTriggered: editEntryAction.triggered()
                }
            ]
            onTriggered: {
                var editEntryPageComponent = Qt.createComponent("EditEntryPage.qml");
                var editEntryPage = editEntryPageComponent.createObject(viewEntryPage, {"entry": entry});
                editEntryPage.open();
                editEntryPage.autofocus();
            }
        },
        DeleteActionItem {
            title: qsTr("Delete", "A button/action to delete an entry.") + Retranslate.onLocaleOrLanguageChanged
            enabled: editable && database.isEditable() && !entry.deleted
            onTriggered: {
                deleteEntryConfirmationDialog.show();
            }
            attachedObjects: [
                SystemDialog {
                    id: deleteEntryConfirmationDialog
                    title: qsTr("Delete Entry", "Title of a delete confirmation dialog") + Retranslate.onLocaleOrLanguageChanged
                    body: qsTr("Really delete this entry?", "A confirmation dialog for deleting entry") + Retranslate.onLocaleOrLanguageChanged
                    confirmButton.label: qsTr("Delete", "A button/action to confirm deletion of an entry") + Retranslate.onLocaleOrLanguageChanged
                    onFinished: {
                        if (value == SystemUiResult.ConfirmButtonSelection) {
                            var parentGroup = entry.parentGroup;
                            Common.deleteEntry(entry);
                            // refresh the parent ListView, otherwise it crashes
                            parentGroup.itemsChanged(DataModelChangeType.AddRemove, 0);
                            // in case we viewed an entry from search results - refresh search
                            if (!!searchResultsPage) {
                                searchResultsPage.performSearch();
                            }
                            naviPane.pop();
                        }
                    }
                }
            ]
        }
    ]   
     
    titleBar: TitleBar { 
        kind: TitleBarKind.Segmented
        onSelectedValueChanged: {
            app.restartWatchdog();
            if (selectedValue)
                setCurrentView(selectedValue);
        }
        options: [
            Option {
                id: generalTabOption
                text: qsTr("General", "Title of a page which shows main/basic properties of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "general"
            },
            Option {
                id: extraTabOption
                text: qsTr("Extra", "Title of a page which shows additional/advanced properties of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "extra"
            },
            Option {
                id: historyTabOption
                text: qsTr("History", "Title of a page which lists the previous versions of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "history"
            }
        ]
    }

    function setCurrentView(viewName) {
        var newView;
        var newViewComponent;
        switch (viewName) {
            case "general": 
                newView = viewEntryGeneralTab; 
                break;
            case "extra":   
                newView = viewEntryExtrasTab; 
                break;
            case "history":   
                newView = viewEntryHistoryTab;   
                break;
            default:
                console.log("WARN: unknown option");
                return;
        }
        currentView = viewName;
        if (entryContent.count() == 0) {
            entryContent.add(newView);
        } else {
            entryContent.replace(0, newView);
        }
    }

    Container {
        leftPadding: 10
        rightPadding: 10
        topPadding: 10
        bottomPadding: 10
        Container {
            id: entryContent
            // subpages will be dynamically inserted here
        }
    }
    attachedObjects: [
        ViewEntryGeneralTab {
            id: viewEntryGeneralTab
        },
        ViewEntryExtrasTab {
            id: viewEntryExtrasTab
        },
        ViewEntryHistoryTab {
            id: viewEntryHistoryTab
        }
    ]
}

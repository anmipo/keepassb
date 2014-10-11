/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.device 1.2
import bb.system 1.2
import bb.cascades.pickers 1.0
import org.keepassb 1.0
import "common.js" as Common

Page {
    id: viewEntryPage
    property PwEntryV3 entry
    property string currentView
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll

    actions: [
        ActionItem {
            id: editEntryAction
            title: qsTr("Edit", "A button/action to edit an entry") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_edit.png"
            enabled: database.isEditable() && !entry.deleted
            ActionBar.placement: ActionBarPlacement.OnBar
            shortcuts: [
                SystemShortcut {
                    type: SystemShortcuts.Edit
                    onTriggered: editEntryAction.triggered()
                }
            ]
            onTriggered: {
                var editEntryPageComponent = Qt.createComponent("EditEntryV3Page.qml");
                var editEntryPage = editEntryPageComponent.createObject(viewEntryPage, {"entry": entry});
                editEntryPage.open();
                editEntryPage.autofocus();
            }
        },
        DeleteActionItem {
            title: qsTr("Delete", "A button/action to delete an entry.") + Retranslate.onLocaleOrLanguageChanged
            enabled: database.isEditable() && !entry.deleted
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
                            naviPane.pop();
                            var parentGroup = entry.parentGroup;
                            Common.deleteEntry(entry);
                            // refresh the parent ListView, otherwise it crashes
                            parentGroup.itemsChanged(DataModelChangeType.AddRemove, 0);
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
                text: qsTr("General", "Title of a page which shows main/basic properties of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "general"
            },
            Option {
                text: qsTr("Extra", "Title of a page which shows additional/advanced properties of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "extra"
            },
            Option {
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
        onTouchCapture: {
            app.restartWatchdog();
        }

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

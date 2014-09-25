/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0

Sheet {
    id: editGroupSheet
    property PwGroup group
    property int iconId: group.iconId 

    onGroupChanged: {
        console.log("edit group: " + group);
    }

    // Sets input focus on group name field. Must be called from the parent component
    function autoFocus() {
        nameField.requestFocus();
    }
    
    // Checks if the user edited any of the fields
    function isModified() {
        return (group.iconId != iconId) || (group.name != nameField.text);
    }
    function saveChanges() {
        app.restartWatchdog();
        group.iconId = iconId
        group.name = nameField.text;
        database.save();
    }
    
    function updateIconId(newIconId) {
        iconId = newIconId;
    }

    onCreationCompleted: {
        database.dbLocked.connect(function() {
                // close without saving when DB is being locked
                editGroupSheet.close();
            });
    }
    onClosed: {
        destroy();
    }
    Page {
        titleBar: TitleBar {
            title: qsTr("Edit Group", "Title of a page for editing group properties") + Retranslate.onLocaleOrLanguageChanged
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    app.restartWatchdog();
                    close();
                }
            }
            acceptAction: ActionItem {
                title: qsTr("Save", "A button/action to save group changes") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    saveChanges();
                    close();
                } 
            }
        }

        Container {
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10
            
            onTouchCapture: {
                app.restartWatchdog();
            }
            
            Label {
                text: qsTr("Name", "Label of the group name edit field") + Retranslate.onLocaleOrLanguageChanged
            }
            MonoTextField {
                id: nameField
                text: group.name
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                ImageView {
                    imageSource: "asset:///pwicons-dark/" + iconId + ".png"
                    horizontalAlignment: HorizontalAlignment.Left
                    verticalAlignment: VerticalAlignment.Fill
                }
                Button {
                    text: qsTr("Change Icon", "A button/action to change group's icon")
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1.0
                    }
                    onClicked: {
                        var iconPickerSheet = iconPickerSheetComponent.createObject(this);
                        iconPickerSheet.iconPicked.connect(updateIconId);
                        iconPickerSheet.open();
                    }
                }
            }
        }
    }
    attachedObjects: [
        SystemDialog {
            id: dismissChangesDialog
            title: qsTr("Discard Changes", "Title of a dialog asking whether the user wants to discard unsaved changes.") + Retranslate.onLocaleOrLanguageChanged
            body: qsTr("There are unsaved changes.", "A notification when the user closes an editor with unsaved changes.") + Retranslate.onLocaleOrLanguageChanged
            cancelButton.label: qsTr("Discard", "A button/action to ignore and discard any unsaved changes made by the user.") + Retranslate.onLocaleOrLanguageChanged
            confirmButton.label: qsTr("Edit", "A button/action to return to edit mode") + Retranslate.onLocaleOrLanguageChanged
            onFinished: {
                if (value == SystemUiResult.CancelButtonSelection) {
                    editGroupSheet.close();
                }
            }
        },
        ComponentDefinition {
            id: iconPickerSheetComponent
            source: "asset:///IconPickerPage.qml"
        }
    ]
}
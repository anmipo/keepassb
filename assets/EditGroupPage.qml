/*
 * Copyright (c) 2014-2016 Andrei Popleteev. 
 * Licensed under the MIT license.
 */

import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0

Sheet {
    id: editGroupSheet
    property PwGroup group
    property bool creationMode: false
    property int iconId: group.iconId 

    onGroupChanged: {
        console.log("edit group: " + group);
    }

    // Sets input focus on group name field. Must be called from the parent component
    function autofocus() {
        nameField.requestFocus();
    }
    
    // Checks if the user edited any of the fields
    function isModified() {
        return (group.iconId != iconId) || (group.name != nameField.text);
    }
    function saveChanges() {
        app.restartWatchdog();
        // no backup on edit for groups (both V3 and V4)
        group.iconId = iconId;
        group.name = nameField.text;
        group.registerModificationEvent();
        database.save();
    }
    
    function updateIconId(newIconId) {
        iconId = newIconId;
    }

    onCreationCompleted: {
        // close without saving when DB is being locked
        database.dbLocked.connect(_close);
        group.registerAccessEvent();
    }
    onClosed: {
        database.dbLocked.disconnect(_close);
        destroy();
    }
    function _close() {
        close();
    }

    PageWithWatchdog {
        titleBar: TitleBar {
            title: creationMode ?
                qsTr("New Group", "Title of a page for creating(editing) new group") + Retranslate.onLocaleOrLanguageChanged :
                qsTr("Edit Group", "Title of a page for editing group properties") + Retranslate.onLocaleOrLanguageChanged
            acceptAction: ActionItem {
                title: qsTr("Save", "A button/action to save group changes") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    nameField.validator.validate();
                    if (!nameField.validator.valid) {
                        nameField.validator.showMessage();
                        return;
                    }
                    
                    saveChanges();
                    group.parentGroup.itemsChanged(DataModelChangeType.Init, 0);
                    editGroupSheet.close();
                } 
            }
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    app.restartWatchdog();
                    if (creationMode) {
                        var parentGroup = group.parentGroup; 
                        group.deleteWithoutBackup();
                        parentGroup.itemsChanged(DataModelChangeType.AddRemove, 0);
                    }
                    editGroupSheet.close();
                }
            }
        }

        Container {
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10            
            Label {
                text: qsTr("Name", "Label of the group name edit field") + Retranslate.onLocaleOrLanguageChanged
            }
            MonoTextField {
                id: nameField
                text: group.name
                trimOnBlur: true
                validator: Validator {
                    mode: ValidationMode.Delayed
                    delay: 500
                    errorMessage: qsTr("This group name is reserved", "An error message when a group is being given a name which is reserved for internal use only.") + Retranslate.onLocaleOrLanguageChanged
                    onValidate: {
                        var nameTrimmed = nameField.text.trim();
                        if (group.isNameReserved(nameTrimmed)) {
                            state = ValidationState.Invalid;
                        } else {
                            state = ValidationState.Valid;
                        }
                    }
                }
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
                        var iconPickerSheet = iconPickerSheetComponent.createObject(editGroupSheet);
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
            title: qsTr("Unsaved Changes", "Title of a dialog asking whether the user wants to discard unsaved changes.") + Retranslate.onLocaleOrLanguageChanged
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
            source: "IconPickerPage.qml"
        }
    ]
}
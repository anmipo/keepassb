/*
 * Copyright (c) 2015 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0

Sheet {
    id: editFieldSheet
    property PwEntryV4 entry
    property PwField field  // null in creation mode
    property string fieldName
    property string fieldValue
    property bool fieldProtected 

    onFieldChanged: {
        console.log("edit field: " + field);
        if (field) {
            fieldName = field.name;
            fieldValue = field.value;
            fieldProtected = field.protected;
        }
    }

    // Sets input focus on group name field. Must be called from the parent component
    function autofocus() {
        nameField.requestFocus();
    }
    
    // Checks if the user edited anything
    function isModified() {
        return field && (
            (field.protected != protectedToggle.checked) || 
            (field.name != nameField.text) || 
            (field.value != valueField.text));
    }
    function saveChanges() {
        app.restartWatchdog();
        
        if (field) { 
            // we're in edit mode
            entry.backupState();
            entry.setExtraField(field, nameField.text, valueField.text, protectedToggle.checked);
        } else { 
            // we're in creation mode
            entry.setExtraField(null, nameField.text, valueField.text, protectedToggle.checked);
        }
        entry.registerModificationEvent();
        database.save();
    }
    
    onCreationCompleted: {
        // close without saving when DB is being locked
        database.dbLocked.connect(_close);
		entry.registerAccessEvent();
    }
    onClosed: {
        database.dbLocked.disconnect(_close);
        destroy();
    }
    function _close() {
        close();
    }

    Page {
        titleBar: TitleBar {
            title: qsTr("Edit Field", "Title of a page for editing an entry field") + Retranslate.onLocaleOrLanguageChanged
            acceptAction: ActionItem {
                title: qsTr("Save", "A button/action to save entry field changes") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    nameField.validator.validate();
                    if (!nameField.validator.valid) {
                        nameField.validator.showMessage();
                        return;          
                    }
                    
                    saveChanges();
                    entry.getExtraFieldsDataModel().itemsChanged(DataModelChangeType.Init, 0);
                    editFieldSheet.close();
                } 
            }
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    app.restartWatchdog();
                    if (!field) { // is in creation mode
                        entry.getExtraFieldsDataModel().itemsChanged(DataModelChangeType.AddRemove, 0);
                    }
                    editFieldSheet.close();
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
                text: qsTr("Name", "Label of the field's name edit control") + Retranslate.onLocaleOrLanguageChanged
            }
            MonoTextField {
                id: nameField
                text: fieldName
                validator: Validator {
                    mode: ValidationMode.FocusLost
                    delay: 0
                    onValidate: {
                        var name = nameField.text;
                        var nameTrimmed = name.trim();
                        if (name != nameTrimmed) {
                            // force-remove leading/trailing whitespaces first 
                            nameField.text = nameTrimmed; 
                        }
                        
                        if (nameTrimmed.length == 0) {
                            state = ValidationState.Invalid;
                            errorMessage = qsTr("Please name the field", "An invitation to input a name for entry's field.") + Retranslate.onLocaleOrLanguageChanged
                            return;
                        }
                        
                        // If field name was edited, or it's a new field - check uniqueness
                        var checkUniqueness = (!field || (field.name != nameTrimmed)); 
                        if (checkUniqueness && entry.containsFieldName(nameTrimmed)) {
                            state = ValidationState.Invalid;
                            errorMessage = qsTr("A field with such name already exists", "An error message when a field is being given a name which is already used or reserved for internal use only.") + Retranslate.onLocaleOrLanguageChanged
                        } else {
                            state = ValidationState.Valid;
                        }
                    }
                }
            }
            Label {
                text: qsTr("Value", "Label of the field's value edit control") + Retranslate.onLocaleOrLanguageChanged
            }
            TextArea {
                id: valueField
                text: fieldValue
                input.flags: TextInputFlag.SpellCheck
                inputMode: TextAreaInputMode.Text
                textFormat: TextFormat.Plain
                textStyle.fontFamily: "\"DejaVu Sans Mono\", Monospace"
                onTextChanging: {
                    app.restartWatchdog();
                }
            }
            Container {
                topPadding: 10
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                Label {
                    text: qsTr("In-memory protection", "An on/off toggle switch to enable encryption of field data in device memory")
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1.0
                    }
                }
                ToggleButton {
                    id: protectedToggle
                    checked: fieldProtected
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
                    editFieldSheet.close();
                }
            }
        }
    ]
}
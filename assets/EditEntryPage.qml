import bb.cascades 1.2
import bb.system 1.2
import bb.cascades.pickers 1.0
import org.keepassb 1.0

Sheet {
    id: entryEditSheet
    property PwEntry entry
    property bool creationMode: false
    property int iconId: entry.iconId 

    // Checks if the user edited any of the fields
    function isModified() {
        var oldExpDate = entry.expiryTime;
        var newExpDate = expiryDateField.value;
        var expiryDateChanged = 
                (oldExpDate.getYear()  != newExpDate.getYear()) || 
                (oldExpDate.getMonth() != newExpDate.getMonth()) || 
                (oldExpDate.getDay()   != newExpDate.getDay()); 
        return (entry.iconId != iconId) || 
                (entry.title != titleField.text) ||
                (entry.userName != usernameField.text) ||
                (entry.password != passwordField.text) ||
                (entry.url != urlField.text) ||
                (entry.notes != notesField.text) ||
                (entry.expires != expiryDateEnabledCheckbox.checked) ||
                expiryDateChanged;
    }

    function saveChanges() {
        app.restartWatchdog();

        entry.registerModificationEvent();
        if (!creationMode) {
            entry.backupState();
        }

        entry.iconId = iconId;
        entry.title = titleField.text;
        entry.userName = usernameField.text;
        entry.password = passwordField.text;
        entry.url = urlField.text;
        entry.notes = notesField.text;
        
        var expiryTime = expiryDateField.value;
        // expiryDateField resets time to 00:00, so we re-assign the time part to original.
        expiryTime.setHours(entry.expiryTime.getHours());
        expiryTime.setMinutes(entry.expiryTime.getMinutes());
        expiryTime.setSeconds(entry.expiryTime.getSeconds());
        entry.expiryTime = expiryTime;
        
        // "expires" must be set after the date, as the flag may affect the timestamp.
        // In particular, it will reset the date/time to EXPIRY_DATE_NEVER (if expires is false) 
        entry.expires = expiryDateEnabledCheckbox.checked;

        database.save();
    }
    function autofocus() {
        titleField.requestFocus();
    }
    function updatePassword(newPwd) {
        passwordField.text = newPwd; 
    }
    function updateIconId(newIconId) {
        iconId = newIconId;
    }
    
    function dismissAndClose() {
        if (creationMode) {
            var parentGroup = entry.parentGroup; 
            entry.deleteWithoutBackup();
            parentGroup.itemsChanged(DataModelChangeType.AddRemove, 0);
        }
        entryEditSheet.close();
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
    
    PageWithWatchdog {
        titleBar: TitleBar {
            title: creationMode ?
                qsTr("New Entry", "Title of a page for creating (editing) a new entry") + Retranslate.onLocaleOrLanguageChanged :
                qsTr("Edit Entry", "Title of a page for editing entry properties") + Retranslate.onLocaleOrLanguageChanged
            acceptAction: ActionItem {
                title: qsTr("Save", "A button/action to save current item") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    saveChanges();
                    entry.parentGroup.itemsChanged(DataModelChangeType.Init, 0);
                    entryEditSheet.close();
                } 
            }
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action to dismiss current dialog") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    app.restartWatchdog();
                    if (isModified()) {
                        dismissChangesDialog.show();
                    } else {
                        dismissAndClose();
                    }
                }
            }
        }
        ScrollView {
            scrollRole: ScrollRole.Main
            Container {
                topPadding: 10
                leftPadding: 10
                rightPadding: 10
                bottomPadding: 10
                Label {
                    text: qsTr("Title", "Label of the entry title edit field") + Retranslate.onLocaleOrLanguageChanged
                }
                MonoTextField {
                    id: titleField
                    text: entry.title
                    trimOnBlur: true
                    input.flags: TextInputFlag.SpellCheck
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
                        text: qsTr("Change Icon", "A button/action to change entry's icon")
                        horizontalAlignment: HorizontalAlignment.Fill
                        verticalAlignment: VerticalAlignment.Center
                        layoutProperties: StackLayoutProperties {
                            spaceQuota: 1.0
                        }
                        onClicked: {
                            var iconPickerSheet = iconPickerSheetComponent.createObject(entryEditSheet);
                            iconPickerSheet.iconPicked.connect(updateIconId);
                            iconPickerSheet.open();
                        }
                    }
                }
                Divider{}
                Label {
                    text: qsTr("User Name", "Label of the username edit field") + Retranslate.onLocaleOrLanguageChanged
                }
                MonoTextField {
                    id: usernameField
                    text: entry.userName
                    trimOnBlur: true
                }
                Divider{}
                Label {
                    text: qsTr("Password", "Label of the password edit field") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    MonoTextField {
                        id: passwordField
                        text: entry.password
                        inputMode: TextFieldInputMode.Password
                    }
                    Button {
                        imageSource: "asset:///images/ic_new_password.png"
                        preferredWidth: 50
                        verticalAlignment: VerticalAlignment.Top
                        horizontalAlignment: HorizontalAlignment.Right
                        onClicked: {
                            var generatorSheet = passwordGeneratorSheetComponent.createObject(entryEditSheet);
                            generatorSheet.newPasswordReady.connect(updatePassword);
                            generatorSheet.open();
                        }
                    }
                }
                Divider{}
                Label {
                    text: qsTr("URL", "Label of the link/internet address edit field") + Retranslate.onLocaleOrLanguageChanged
                }
                MonoTextField {
                    id: urlField
                    text: entry.url
                    inputMode: TextFieldInputMode.Url
                    trimOnBlur: true
                }
                Divider{}
                Label {
                    text: qsTr("Notes", "Label of the notes/comments edit field") + Retranslate.onLocaleOrLanguageChanged
                }
                TextArea {
                    id: notesField
                    text: entry.notes
                    autoSize.maxLineCount: 5
                    input.flags: TextInputFlag.SpellCheck
                    inputMode: TextAreaInputMode.Text
                    textFormat: TextFormat.Plain
                    textStyle.fontFamily: "\"DejaVu Sans Mono\", Monospace"
                    onTextChanging: {
                        app.restartWatchdog();
                    }
                }
                Divider{}
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Label {
                        text: qsTr("Entry Expires", "On/off option which enables entry expiration deadline") + Retranslate.onLocaleOrLanguageChanged
                        layoutProperties: StackLayoutProperties {
                            spaceQuota: 1
                        }
                    }
                    ToggleButton {
                        id: expiryDateEnabledCheckbox
                        checked: entry.expires
                    }
                }
                DateTimePicker {
                    id: expiryDateField
                    title: qsTr("Expiry Date", "Label of the entry expiration/best before date") + Retranslate.onLocaleOrLanguageChanged
                    mode: DateTimePickerMode.Date
                    enabled: expiryDateEnabledCheckbox.checked
                    value: Qt.formatDateTime(entry.expiryTime, "yyyy-MM-dd") // must remove time, otherwise 23:59:59 rounds up to next day
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
                    dismissAndClose();
                }
            }
        },
        ComponentDefinition {
            id: passwordGeneratorSheetComponent
            source: "PasswordGeneratorPage.qml"
        },
        ComponentDefinition {
            id: iconPickerSheetComponent
            source: "IconPickerPage.qml"
        }
    ]
}
import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common

Sheet {
    id: entryEditSheet
//    property PwEntryV3 entry -- used from the caller
    property int iconId: entry.iconId 

    // Checks if the user edited any of the fields
    function isModified() {
        return (entry.iconId != iconId) || 
                (entry.title != titleField.text) ||
                (entry.userName != usernameField.text) ||
                (entry.password != passwordField.text) ||
                (entry.url != urlField.text) ||
                (entry.notes != notesField.text);
    }

    function saveChanges() {
        app.restartWatchdog();
        
        entry.iconId = iconId
        entry.title = titleField.text;
        entry.userName = usernameField.text;
        entry.password = passwordField.text;
        entry.url = urlField.text;
        entry.notes = notesField.text;
        database.save();
    }

    function updatePassword(newPwd) {
        passwordField.text = newPwd; 
    }
    function updateIconId(newIconId) {
        iconId = newIconId;
    }
    onCreationCompleted: {
        database.dbLocked.connect(function() {
                // close without saving when DB is being locked
                entryEditSheet.close();
            });
    }
    onClosed: {
        destroy();
    }
    Page {
        titleBar: TitleBar {
            title: qsTr("Edit Entry", "Title of a dialog box") + Retranslate.onLocaleOrLanguageChanged
            acceptAction: ActionItem {
                title: qsTr("Save", "A button/action to save current item") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    saveChanges();
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
                        entryEditSheet.close();
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
                onTouchCapture: {
                    app.restartWatchdog();
                }

                Label {
                    text: qsTr("Title", "Label of the entry title edit field") + Retranslate.onLocaleOrLanguageChanged
                }
                MonoTextField {
                    id: titleField
                    text: entry.title
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
                            var iconPickerSheet = iconPickerSheetComponent.createObject(this);
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
                            var generatorSheet = passwordGeneratorSheetComponent.createObject(this);
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
                    entryEditSheet.close();
                }
            }
        },
        ComponentDefinition {
            id: passwordGeneratorSheetComponent
            source: "asset:///PasswordGeneratorPage.qml"
        },
        ComponentDefinition {
            id: iconPickerSheetComponent
            source: "asset:///IconPickerPage.qml"
        }
    ]
}
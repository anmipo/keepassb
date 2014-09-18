import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common

Sheet {
    id: entryEditSheet
//    property PwEntryV3 entry -- used from the caller 

    // Checks if the user edited any of the fields
    function isModified() {
        return (entry.title != titleField.text) ||
                (entry.userName != usernameField.text) ||
                (entry.password != passwordField.text) ||
                (entry.url != urlField.text) ||
                (entry.notes != notesField.text);
    }

    function saveChanges() {
        entry.title = titleField.text;
        entry.userName = usernameField.text;
        entry.password = passwordField.text;
        entry.url = urlField.text;
        entry.notes = notesField.text;
        database.save();
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
                leftPadding: 10
                rightPadding: 10
                bottomPadding: 10
            
                Header {
                    title: qsTr("Title", "Label of the entry title edit field") + Retranslate.onLocaleOrLanguageChanged
                    bottomMargin: 10
                }
                MonoTextField {
                    id: titleField
                    text: entry.title
                }
                Header {
                    title: qsTr("User Name", "Label of the username edit field") + Retranslate.onLocaleOrLanguageChanged
                    bottomMargin: 10
                }
                MonoTextField {
                    id: usernameField
                    text: entry.userName
                }
                Header {
                    title: qsTr("Password", "Label of the password edit field") + Retranslate.onLocaleOrLanguageChanged
                    bottomMargin: 10
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
                        imageSource: "asset:///images/ic_edit.png"
                        preferredWidth: 50
                        verticalAlignment: VerticalAlignment.Top
                        horizontalAlignment: HorizontalAlignment.Right
                    }
                }
                Header {
                    title: qsTr("URL", "Label of the link/internet address edit field") + Retranslate.onLocaleOrLanguageChanged
                    bottomMargin: 10
                }
                MonoTextField {
                    id: urlField
                    text: entry.url
                }
                Header {
                    title: qsTr("Notes", "Label of the notes/comments edit field") + Retranslate.onLocaleOrLanguageChanged
                    bottomMargin: 10
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
            confirmButton.label: qsTr("Edit", "A button/action to return to entry edit mode") + Retranslate.onLocaleOrLanguageChanged
            onFinished: {
                if (value == SystemUiResult.CancelButtonSelection) {
                    entryEditSheet.close();
                }
            }
        }
    ]
}
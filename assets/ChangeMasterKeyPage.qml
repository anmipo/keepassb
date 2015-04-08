/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.4
import bb.cascades.pickers 1.0
import bb.system 1.2
import QtQuick 1.0
import org.keepassb 1.0
import "common.js" as Common

Sheet {
    property alias keyType: masterKeyType.selectedValue 
    property string keyFilePath: ""
    property string dbName
    
    onCreationCompleted: {
        // close without saving when DB is being locked
        database.dbLocked.connect(_close);
        app.restartWatchdog();
        loadRecentItems();
        dbName = Common.prettifyFilePath(database.getDatabaseFilePath());
    }
    onClosed: {
        database.dbLocked.disconnect(_close);
        destroy();
    }
    function _close() {
        passwordEdit.text = ""
        keyDropDown.selectedIndex = 0
        close();
    }
    
    function autofocus() {
        passwordEdit.requestFocus();
    }

    function changeMasterKey(password, keyFilePath) {
        // at least one of the parameters is guaranteed to be non-empty
        if (database.changeMasterKey(passwordEdit.text, keyFilePath)) {
            // change successful
            if (password.length == 0) {
                // new master key does not use password, so disable Quick Unlock
                appSettings.quickUnlockEnabled = false;
            } else {
                if (appSettings.quickUnlockEnabled) {
                    // update quick password
                    app.prepareQuickUnlock(password);
                }
            }
            _close();
        } else {
            // There was an error. The message will be shown via signals.
            // Nothing to do, keep the form opened.
        }
    }

    function addKeyOption(fullPath) {
        var option = Common.createUniqueOption(fullPath, keyDropDown, newOptionComponent);
        // The first option should always be "None", so insert after it
        keyDropDown.insert(1, option);
    }
    
    function loadRecentItems() {
        var items = appSettings.getRecentFiles();
        for (var i = items.length - 1; i >= 0; i--) {
            var paths = items[i].split("|");
            var keyFile = paths[1];
            // empty keyFile a ignored, since there is a dedicated "(none)" option 
            if (keyFile != "")
                addKeyOption(keyFile);
        }
        keyDropDown.selectedIndex = 0; // default to "none"
    }

    Page {
        titleBar: TitleBar {
            title: qsTr("Master Key", "Title of a dialog for changing main/master password of the current database") + Retranslate.onLocaleOrLanguageChanged
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action to cancel/close master password change form") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    _close();
                }
            }
        }
        Container {
            layout: DockLayout {}
            onTouchCapture: {
                app.restartWatchdog();
            }
            Container {
                verticalAlignment: VerticalAlignment.Top
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10
                leftPadding: 10
                rightPadding: 10
                bottomPadding: 10
                Label {
                    text: qsTr("You are about to change the master key for this database: <i>%1</i>", "Help text in the master key change dialog. %1 is the currently opened database file (for example, 'documents/database.dat')").arg(dbName) + Retranslate.onLocaleOrLanguageChanged
                    multiline: true
                    textFormat: TextFormat.Html
                    textStyle.base: SystemDefaults.TextStyles.BodyText
                    bottomMargin: 30
                }
                DropDown {
                    id: masterKeyType
                    title: qsTr("Type", "Title of a dropdown selection box to choose the type of database's master key (password, key file, or bot)") + Retranslate.onLocaleOrLanguageChanged
                    options: [
                        Option {
                            id: passwordOnly
                            text: qsTr("Password Only", "One of the possible values of the 'Master key type' selector") + Retranslate.onLocaleOrLanguageChanged
                            value: "password"
                            selected: true
                        },
                        Option {
                            id: keyFileOnly
                            text: qsTr("Key File Only", "One of the possible values of the 'Master key type' selector") + Retranslate.onLocaleOrLanguageChanged
                            value: "keyfile"
                        },
                        Option {
                            id: passwordAndKeyFile
                            text: qsTr("Password + Key File", "One of the possible values of the 'Master key type' selector") + Retranslate.onLocaleOrLanguageChanged
                            value: "password+keyfile"
                        }
                    ]
                    onSelectedValueChanged: {
                        app.restartWatchdog();
                        if (selectedValue == "password") {
                            keyDropDown.selectedIndex = 0;
                        } else if (selectedValue == "keyfile") {
                            passwordEdit.text = "";
                        } else {
                            keyDropDown.selectedIndex = 0;
                            passwordEdit.text = "";
                        }
                    }
                }
                DropDown {
                    property int lastSelectedIndex: 0
                    id: keyDropDown
                    title: qsTr("Key File", "A label before the selected key file (see thesaurus). Example: 'Key File    Documents/file.dat'.") + Retranslate.onLocaleOrLanguageChanged
                    visible: (keyType != 'password')
                    onSelectedOptionChanged: {
                        app.restartWatchdog();
                        if (!selectedOption) 
                            return;
                    
                        if (selectedOption == keyBrowseOption) {
                            // lastSelectedIndex should not change here
                            keyFilePicker.open();
                        } else {
                            lastSelectedIndex = selectedIndex;
                            keyFilePath = selectedOption.value;
                        }
                    }
                    options: [
                        Option {
                            id: keyNoneOption
                            text: qsTr("(none)", "Value of the 'Key File' field when no key file were selected; will be displayed as 'Key Flie    (none)'.") + Retranslate.onLocaleOrLanguageChanged
                            value: ""
                            selected: true
                        },
                        // dynamically inserted options are inserted here
                        Option {
                            id: keyBrowseOption
                            text: qsTr("Browse...", "A button/action which opens a file selection dialog.") + Retranslate.onLocaleOrLanguageChanged
                            imageSource: "asset:///images/ic_browse.png"
                            value: "_browse_"
                        }
                    ]
                }
                TextField {
                    id: passwordEdit
                    visible: (keyType != "keyfile")
                    hintText: qsTr("Enter password", "Invitation to enter a password.") + Retranslate.onLocaleOrLanguageChanged
                    inputMode: TextFieldInputMode.Password
                    text: ""
                    input.submitKey: SubmitKey.EnterKey
                    input.onSubmitted: {
                        saveChangesButton.clicked();
                    }
                }
                Button {
                    id: saveChangesButton
                    text: qsTr("Save Changes") + Retranslate.onLocaleOrLanguageChanged
                    horizontalAlignment: HorizontalAlignment.Fill
                    onClicked: {
                        var isKeyFileEmpty = (keyFilePath.length == 0);
                        var isPasswordEmpty =  (passwordEdit.text.length == 0);
                        if ((keyType == "keyfile") && isKeyFileEmpty) {
                            errorToast.body = qsTr("Please specify the key file", "An error message/invitation to choose the key file for the master key")
                            errorToast.show();
                            keyDropDown.expanded = true;
                        } else if (((keyType == "password") && isPasswordEmpty) 
                                || ((keyType == "password+keyfile") && isKeyFileEmpty && isPasswordEmpty)) {
                            // In case of password+keyfile either one would suffice, but we enforce both, starting with the password
                            errorToast.body = qsTr("Password cannot be empty", "An error message/invitation to enter a non-empty password for the master key")
                            errorToast.show();
                            passwordEdit.requestFocus();
                        } else {
                            changeMasterKey(passwordEdit.text, keyFilePath);
                        }
                    }
                }
            }
        }
    }
    
    attachedObjects: [
        SystemToast {
            id: errorToast
            position: SystemUiPosition.TopCenter
        },
        ComponentDefinition {
            id: newOptionComponent
            Option {}
        },
        FilePicker {
            id: keyFilePicker
            mode: FilePickerMode.Picker
            type: FileType.Other
            title: qsTr("Choose Key File", "Title of a key file selection dialog; an invitation to choose a file (also see 'key file' in the thesaurus).") + Retranslate.onLocaleOrLanguageChanged
            onFileSelected: {
                addKeyOption(selectedFiles[0]); // actual path
                keyDropDown.selectedIndex = 1; // option[0] is "None"
            }
            onCanceled: {
                keyDropDown.selectedIndex = keyDropDown.lastSelectedIndex; // restore the pre-Browse selection
            }
        }
    ]
}

/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.cascades.pickers 1.0
import bb.system 1.2
import QtQuick 1.0
import org.keepassb 1.0
import "common.js" as Common

Page {
    // emmitted when a DB is successfully unlocked
    signal databaseUnlocked()
    property string dbFilePath: "_"
    property string keyFilePath: "_"
    // true when "Demo database" is selected
    property bool demoMode: true
    
    function showErrorToast(message, errorCode) {
        unlockProgressDialog.cancel();
        if (errorCode)
            message = qsTr("%1 (#%2)", 
                    "A template for 'Error message (#error_code)'; change only for right-to-left langugages")
                    .arg(message)
                    .arg(errorCode);
        dbErrorToast.body = message; 
        dbErrorToast.show();
    }
    
    function showFileAccessError(message, errorDescription) {
        unlockProgressDialog.cancel();
        dbErrorToast.body = qsTr("%1\n(%2)", 
                "A template for 'Error message (Error description)'; change only for right-to-left langugages")
                .arg(message)
                .arg(errorDescription); 
        dbErrorToast.show();
    }
    
    function focusOnPassword() {
        if (!dbDemoOption.selected)
            passwordEdit.requestFocus();
    }   

    /**
     * Puts given DB path into dropdown options and selects it.
     */
    function chooseDatabaseFile(dbFilePath) {
        addDatabaseOption(dbFilePath); // actual path
        dbDropDown.selectedIndex = 0;
    }
    
    function addDatabaseOption(fullPath) {
        var option = Common.createUniqueOption(fullPath, dbDropDown, newOptionComponent);
        dbDropDown.insert(0, option); // adds to the top 
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
            var dbFile = paths[0];
            var keyFile = paths[1];
            addDatabaseOption(dbFile);
            // empty keyFile a ignored, since there is a dedicated "(none)" option 
            if (keyFile != "")
                addKeyOption(keyFile);
        }
        dbDropDown.selectedIndex = 0;
        // keyDropDown will be selected automagically
    }
    
    function selectKeyOptionByFilename(keyfile) {
        var index = 0;
        for (var i = 1; i < keyDropDown.count(); i++) {
            if (keyDropDown.options[i].value == keyfile) {
                index = i;
                break;
            }
        }
        keyDropDown.selectedIndex = index;
    }
    
    onCreationCompleted: {
        app.invokedWithDatabase.connect(chooseDatabaseFile);
        database.fileOpenError.connect(showFileAccessError);
        database.dbUnlockError.connect(showErrorToast);
        database.dbUnlocked.connect(function() {
            unlockProgressDialog.cancel();
            databaseUnlocked();
        });
        database.progressChanged.connect(function(progress) {
            unlockProgressDialog.progress = progress;
        });
        database.invalidPasswordOrKey.connect(function() {
            showErrorToast(qsTr("Invalid password or key file", 
                    "An error message shown when the decryption fails. Also see 'key file' in thesaurus.") + Retranslate.onLocaleOrLanguageChanged);
        });
        loadRecentItems();
    }
    
    Container {
        layout: DockLayout {}
        Container {
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Fill    
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10
            DropDown {
                property int lastSelectedIndex: -1

                id: dbDropDown
                title: qsTr("Database", "A label before the selected database file. Example: 'Database    documents/database.dat'") + Retranslate.onLocaleOrLanguageChanged
                onSelectedOptionChanged: {
                    if (!selectedOption) 
                        return;

                    if (selectedOption == dbBrowseOption) {
                        // lastSelectedIndex should not change here
                        dbFilePicker.open();
                    } else if (selectedOption == dbCreateOption) {
                        // lastSelectedIndex should not change here
                        // so if the user cancels, we select the previous DB
                        newDbFilePicker.open();
                    } else if (selectedOption == dbDemoOption) {
                        lastSelectedIndex = selectedIndex; 
                        demoMode = true;
                        dbFilePath = selectedOption.value
                        console.log("Using demo DB");
                        selectKeyOptionByFilename(""); // no key file needed
                    } else {
                        lastSelectedIndex = selectedIndex; 
                        demoMode = false;
                        dbFilePath = selectedOption.value
                        var keyfile = appSettings.getKeyFileForDatabase(dbFilePath);
                        selectKeyOptionByFilename(keyfile);
                    }
                }
                options: [
                    // dynamically inserted options are inserted here
                    Option {
                        id: dbDemoOption
                        // TODO hide this after first non-demo file open
                        text: qsTr("Demo Database", "A special database used for demonstration purpose. Example: 'Database    Demo Database'.") + Retranslate.onLocaleOrLanguageChanged
                        imageSource: "asset:///pwicons/13.png"
                        value: "app/native/assets/demo.kdbx"
                    },
                    Option {
                        id: dbCreateOption
                        text: qsTr("Create Database...", "An action to create a new database.") + Retranslate.onLocaleOrLanguageChanged
                        imageSource: "asset:///images/ic_create_database.png"
                        value: "_create_"
                    },
                    Option {
                        id: dbBrowseOption
                        text: qsTr("Browse...", "A button/action which opens a file selection dialog.") + Retranslate.onLocaleOrLanguageChanged
                        imageSource: "asset:///images/ic_browse.png"
                        value: "_browse_"
                    }
                ]
            }
            DropDown {
                property int lastSelectedIndex: -1
                
                id: keyDropDown
                title: qsTr("Key File", "A label before the selected key file (see thesaurus). Example: 'Key File    Documents/file.dat'.") + Retranslate.onLocaleOrLanguageChanged
                topMargin: 20
                visible: !demoMode
                onSelectedOptionChanged: {
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
                topMargin: 20
                visible: !demoMode
                hintText: qsTr("Enter password", "Invitation to enter a password.") + Retranslate.onLocaleOrLanguageChanged
                inputMode: TextFieldInputMode.Password
                text: ""
                input.submitKey: SubmitKey.EnterKey
                input.onSubmitted: openDbAction.triggered()
            }
            CheckBox {
                id: enableQuickUnlock
                topMargin: 20
                visible: !demoMode
                text: qsTr("Enable Quick Unlock", "A checkbox which activates the Quick Unlock function. 'Enable' is used as 'activate', 'allow', 'turn on'.") + Retranslate.onLocaleOrLanguageChanged
                checked: appSettings.quickUnlockEnabled
                onCheckedChanged: {
                    appSettings.quickUnlockEnabled = checked;
                }
            }
            Label {
                visible: demoMode
                topMargin: 20
                verticalAlignment: VerticalAlignment.Center
                text: qsTr("Demo database enables you to test KeePassB without importing a real database.\nTo continue, tap the lock symbol below.", "Description of the 'Demo Database' option. The 'lock symbol' refers to a button with a padlock icon.") + Retranslate.onLocaleOrLanguageChanged
                multiline: true
            }
        }
    }
    attachedObjects: [
        FilePicker {
            id: dbFilePicker
            mode: FilePickerMode.Picker
            type: FileType.Other
            title: qsTr("Choose Database", "Title of a database selection dialog; an invitation to choose a file.") + Retranslate.onLocaleOrLanguageChanged
            onFileSelected: {
                chooseDatabaseFile(selectedFiles[0]);
            }
            onCanceled: {
                dbDropDown.selectedIndex = dbDropDown.lastSelectedIndex; // restore the pre-Browse selection
            }
        },
        FilePicker {
            id: newDbFilePicker
            mode: FilePickerMode.Saver
            type: FileType.Other
            defaultSaveFileNames: qsTr("NewDatabase.kdbx", "Default file name for new databases. The '.kdbx' extension should not be translated.");
            title: qsTr("Create Database", "Title of a file saving dialog used when creating new database file.") + Retranslate.onLocaleOrLanguageChanged
            onFileSelected: {
                var newDbFileName = selectedFiles[0];
                console.log("Creating new DB: " + newDbFileName);
                if (database.createDatabaseV4(newDbFileName)) {
                    console.log("createDatabaseV4() success");
                    // Now force the user to change (or rather set) the master key                    
                    var changeMasterKeySheetComponent =Qt.createComponent("ChangeMasterKeyPage.qml");
                    //TODO: check if the above leaks
                    var changeMasterKeySheet = changeMasterKeySheetComponent.createObject(naviPane.top, {"creationMode": true});
                    changeMasterKeySheet.open();
                    changeMasterKeySheet.autofocus();
                    
                    // Restore the pre-Browse selection, in case creation gets cancelled;
                    dbDropDown.selectedIndex = dbDropDown.lastSelectedIndex;
                    // Select new db/keyfile once (if) creation is completed. 
                    changeMasterKeySheet.masterKeyChanged.connect(loadRecentItems);
                } else {
                    console.log("createDatabaseV4() failed");
                    dbDropDown.selectedIndex = dbDropDown.lastSelectedIndex; // restore the pre-Browse selection 
                }
            }
            onCanceled: {
                dbDropDown.selectedIndex = dbDropDown.lastSelectedIndex; // restore the pre-Browse selection
            }
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
        },
        SystemToast {
            id: dbErrorToast
            position: SystemUiPosition.TopCenter
        },
        SystemProgressDialog {
            id: unlockProgressDialog
            title: qsTr("Decrypting...", "Title of a progress indicator while a database is being decrypted/decoded.") + Retranslate.onLocaleOrLanguageChanged
            autoUpdateEnabled: true
            confirmButton.label: ""
            cancelButton.label: ""
        },
        ComponentDefinition {
            id: newOptionComponent
            Option {}
        }
    ]
    
    actions: [
        ActionItem {
            id: openDbAction
            title: qsTr("Open", "A button/action which opens/unlocks a database file.") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_unlock.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                if (demoMode) {
                    var password = "demo";
                    enableQuickUnlock.checked = false;
                } else {
                    var password = passwordEdit.text;
                    
                    if ((password.length == 0) && (keyFilePath.length == 0)) {
                        dbErrorToast.body = qsTr("Please provide a password or a key file.", "Notification message when the user tries to open a database without providing any credentials"); 
                        dbErrorToast.show();
                        return;
                    }
                    
                    // disable Quick Unlock when using only key file
                    if (password.length == 0) {
                        appSettings.quickUnlockEnabled = false;
                    }
                    
                    if (appSettings.quickUnlockEnabled) {
                        app.prepareQuickUnlock(password);
                    }
                    unlockProgressDialog.progress = 0;
                    unlockProgressDialog.show();
                    appSettings.addRecentFiles(dbFilePath, keyFilePath);
                }
                passwordEdit.text = "";
                database.unlock(dbFilePath, password, keyFilePath);
                loadRecentItems(); // to rearrange items
            }
        }
    ]
}

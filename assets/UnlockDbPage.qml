/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.cascades.pickers 1.0
import bb.system 1.2
import QtQuick 1.0
import org.keepassb 1.0

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
            message += " (#" + errorCode + ")";
        dbErrorToast.body = message; 
        dbErrorToast.show();
    }
    
    // Returns a nice human-readable file path.
    // Basically, it just cuts off the prefix, such as 
    //   "/accounts/1000/shared" 
    //   "/accounts/1000/removable"
    //   "/accounts/1000-enterprise/shared"
    //   "/accounts/1000-enterprise/removable"
    function prettifyFilePath(path) {
        return path.split('/').slice(4).join('/');
    }
    
    function focusOnPassword() {
        passwordEdit.requestFocus();
    }   

    /**
     * Puts given DB path into dropdown options and selects it.
     */
    function chooseDatabaseFile(dbFilePath) {
        addDatabaseOption(dbFilePath); // actual path
        dbDropDown.selectedIndex = 0;
    }
    
    /**
     * Creates an Option of the given DropDown; 
     * deletes the previous option with the same value, if any.
     */
    function createUniqueOption(fullPath, dropdown) {
        // First, remove any already existing options with this path
        var existingIndex = -1;
        var options = dropdown.options;
        for (var i = 0; i < dropdown.count(); i++) {
            if (options[i].value == fullPath) {
                existingIndex = i;
                break;
            } 
        }
        if (existingIndex >= 0)
            dropdown.remove(dropdown.options[existingIndex]);
    
        var option = newOptionComponent.createObject();
        option.value = fullPath;
        option.text = prettifyFilePath(fullPath);
        return option;
    }
    
    function addDatabaseOption(fullPath) {
        var option = createUniqueOption(fullPath, dbDropDown);
        dbDropDown.insert(0, option); // adds to the top 
    }
    
    function addKeyOption(fullPath) {
        var option = createUniqueOption(fullPath, keyDropDown);
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
        database.fileOpenError.connect(showErrorToast);
        database.dbUnlockError.connect(showErrorToast);
        database.dbUnlocked.connect(function() {
            unlockProgressDialog.cancel();
            databaseUnlocked();
        });
        database.unlockProgressChanged.connect(function(progress) {
            unlockProgressDialog.progress = progress;
        });
        database.invalidPasswordOrKey.connect(function() {
            showErrorToast(qsTr("Invalid password or key file") + Retranslate.onLocaleOrLanguageChanged);
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
                title: qsTr("Database") + Retranslate.onLocaleOrLanguageChanged
                onSelectedOptionChanged: {
                    if (!selectedOption) 
                        return;

                    if (selectedOption == dbBrowseOption) {
                        // lastSelectedIndex should not change here
                        dbFilePicker.open();
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
                    Option {
                        id: dbDemoOption
                        // TODO hide this after first non-demo file open
                        text: qsTr("Demo database") + Retranslate.onLocaleOrLanguageChanged
                        imageSource: "asset:///pwicons/13.png"
                        value: "app/native/assets/demo.kdbx"
                    },
                    Option {
                        id: dbBrowseOption
                        text: qsTr("Browse...") + Retranslate.onLocaleOrLanguageChanged
                        imageSource: "asset:///images/ic_browse.png"
                        value: "_browse_"
                    }
                ]
            }
            DropDown {
                property int lastSelectedIndex: -1
                
                id: keyDropDown
                title: qsTr("Key file") + Retranslate.onLocaleOrLanguageChanged
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
                        text: qsTr("(none)") + Retranslate.onLocaleOrLanguageChanged
                        value: ""
                    },
                    Option {
                        id: keyBrowseOption
                        text: qsTr("Browse...") + Retranslate.onLocaleOrLanguageChanged
                        imageSource: "asset:///images/ic_browse.png"
                        value: "_browse_"
                    }
                ]
            }
            TextField {
                id: passwordEdit
                topMargin: 20
                visible: !demoMode
                hintText: qsTr("Enter password") + Retranslate.onLocaleOrLanguageChanged
                inputMode: TextFieldInputMode.Password
                text: ""
                input.submitKey: SubmitKey.EnterKey
                input.onSubmitted: openDbAction.triggered()
            }
            CheckBox {
                id: enableQuickUnlock
                topMargin: 20
                visible: !demoMode
                text: qsTr("Enable quick unlock") + Retranslate.onLocaleOrLanguageChanged
                checked: appSettings.quickUnlockEnabled
                onCheckedChanged: {
                    appSettings.quickUnlockEnabled = checked;
                }
            }
            Label {
                visible: demoMode
                topMargin: 20
                verticalAlignment: VerticalAlignment.Center
                text: qsTr("Demo option enables you to test KeePassB without importing a real database.\nTo continue, tap the lock symbol below.") + Retranslate.onLocaleOrLanguageChanged
                multiline: true
            }
        }
    }
    attachedObjects: [
        FilePicker {
            id: dbFilePicker
            mode: FilePickerMode.Picker
            type: FileType.Other
            title: qsTr("Choose database") + Retranslate.onLocaleOrLanguageChanged
            onFileSelected: {
                chooseDatabaseFile(selectedFiles[0]);
            }
            onCanceled: {
                dbDropDown.selectedIndex = dbDropDown.lastSelectedIndex; // restore the pre-Browse selection
            }
        },
        FilePicker {
            id: keyFilePicker
            mode: FilePickerMode.Picker
            type: FileType.Other
            title: qsTr("Choose key file") + Retranslate.onLocaleOrLanguageChanged
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
        },
        SystemProgressDialog {
            id: unlockProgressDialog
            title: qsTr("Decrypting...") + Retranslate.onLocaleOrLanguageChanged
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
            title: qsTr("Open") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_unlock.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                if (demoMode) {
                    var password = "demo";
                    enableQuickUnlock.checked = false;
                } else {
                    var password = passwordEdit.text;
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

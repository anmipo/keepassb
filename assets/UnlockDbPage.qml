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

    onDbFilePathChanged: {
        if (dbFilePath.length > 0) {
            dbFileField.textStyle.fontStyle = FontStyle.Default;
            dbFileField.text = prettifyFilePath(dbFilePath); // human-friendly path
        } else {
            dbFileField.textStyle.fontStyle = FontStyle.Italic;
            dbFileField.text = qsTr("Choose database")
        }
        if (rememberRecent.checked)
            appSettings.recentDbPath = dbFilePath;
    }
    onKeyFilePathChanged: {
        if (keyFilePath.length > 0) {
            keyFileField.textStyle.fontStyle = FontStyle.Default;
            keyFileField.text = prettifyFilePath(keyFilePath); // human-friendly path
        } else {
            keyFileField.textStyle.fontStyle = FontStyle.Italic;
            keyFileField.text = qsTr("Choose a key file (optional)");
        }
        if (rememberRecent.checked)
            appSettings.recentKeyFilePath = keyFilePath;
    }

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
    
    titleBar: TitleBar {
        title: qsTr("KeePassB") + Retranslate.onLocaleOrLanguageChanged
        visibility: ChromeVisibility.Default
    }

    function applyTrackRecentDb(track) {
        rememberRecent.checked = track;
        if (track) {
            appSettings.recentDbPath = dbFilePath;
            appSettings.recentKeyFilePath = keyFilePath;
        } else {
            appSettings.recentDbPath = "";
            appSettings.recentKeyFilePath = "";
        }
    }

    Container {
        layout: DockLayout {}
        leftPadding: 10
        rightPadding: 10
        topPadding: 0
        bottomPadding: 10
        Container {
            verticalAlignment: VerticalAlignment.Top
            Header {
                title: qsTr("Database")
                bottomMargin: 5
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
	            Label {
	                id: dbFileField
	                horizontalAlignment: HorizontalAlignment.Fill
	                verticalAlignment: VerticalAlignment.Center
	                multiline: true
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }
	            }
                Button {
                    accessibility.name: qsTr("Browse for database") + Retranslate.onLocaleOrLanguageChanged
                    imageSource: "asset:///images/ic_browse.png"
                    preferredWidth: 50
	                verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Right
                    onClicked: {
                        dbFilePicker.open()
                    }
                }
            }

            Header {
                title: qsTr("Password and key file")
                topMargin: 20
                bottomMargin: 5
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                bottomMargin: 5
                TextField {
                    id: passwordEdit
                    hintText: qsTr("Enter password") + Retranslate.onLocaleOrLanguageChanged
                    inputMode: TextFieldInputMode.Password
                    rightPadding: 50
                    text: ""
                    input.submitKey: SubmitKey.Done
                    input.onSubmitted: openDbAction.triggered()
                }
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                Label {
                    id: keyFileField
                    multiline: true
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }
                }
                Button {
                    imageSource: "asset:///images/ic_browse.png"
                    accessibility.name: qsTr("Browse for key file") + Retranslate.onLocaleOrLanguageChanged
                    preferredWidth: 50
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    onClicked: {
                        keyFilePicker.open();
                    }
                }
            }
            Header {
                title: qsTr("Options")
                topMargin: 20
                bottomMargin: 5
            }
            CheckBox {
                id: rememberRecent
                text: qsTr("Remember selected files")
                checked: false
                onCheckedChanged: {
                    appSettings.setTrackRecentDb(checked);
                    applyTrackRecentDb(checked);
                }
            }
//            CheckBox {
//                text: qsTr("Enable quick unlock")
//            }
	    }
    }

    onCreationCompleted: {
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
                showErrorToast("Invalid password or key file");
            });
    }

    attachedObjects: [
        FilePicker {
            id: dbFilePicker
            mode: FilePickerMode.Picker
            type: FileType.Other
            title: qsTr("Choose database") + Retranslate.onLocaleOrLanguageChanged
            onFileSelected: {
                dbFilePath = selectedFiles[0]; // actual path
            }
        },
        FilePicker {
            id: keyFilePicker
            mode: FilePickerMode.Picker
            type: FileType.Other
            title: qsTr("Choose key file") + Retranslate.onLocaleOrLanguageChanged
            onFileSelected: {
                keyFilePath = selectedFiles[0]; // actual path
            } 
            onCanceled: {
                keyFilePath = ""; // the only UI way (so far) to clear the key path...
            }
        },
		SystemToast {
			id: dbErrorToast
			icon: "asset:///images/warning.png"
		},
		SystemProgressDialog {
            id: unlockProgressDialog
            title: qsTr("Decrypting...") + Retranslate.onLocaleOrLanguageChanged
            autoUpdateEnabled: true
            confirmButton.label: ""
            cancelButton.label: ""
		}
    ]

    actions: ActionItem {
        id: openDbAction
        title: qsTr("Open") + Retranslate.onLocaleOrLanguageChanged
    	imageSource: "asset:///images/ic_unlock.png"
    	ActionBar.placement: ActionBarPlacement.OnBar
    	onTriggered: {
    	    var password = passwordEdit.text;
    	    passwordEdit.text = "";
            unlockProgressDialog.progress = 0;
            unlockProgressDialog.show();
            database.unlock(dbFilePath, password, keyFilePath);
    	}
    }
}

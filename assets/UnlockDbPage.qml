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
    property string dbFilePath;
    property string keyFilePath;
        
    onDbFilePathChanged: {
        dbFileEdit.text = prettifyFilePath(dbFilePath); // human-friendly path         
    }
    onKeyFilePathChanged: {
        keyFileEdit.text = prettifyFilePath(keyFilePath); // human-friendly path
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
    }
    
    Container {
        layout: DockLayout {}
        leftPadding: 10
        rightPadding: 10
        topPadding: 10
        bottomPadding: 10
        Container {
            verticalAlignment: VerticalAlignment.Top
            Label {
                text: qsTr("Database") + Retranslate.onLocaleOrLanguageChanged
                textStyle {
                    base: SystemDefaults.TextStyles.PrimaryText
                }
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
	            TextField {
	                id: dbFileEdit
                    //text: "/shared/documents/kpb-test.kdbx"
                    focusPolicy: FocusPolicy.None
	                horizontalAlignment: HorizontalAlignment.Fill
	                verticalAlignment: VerticalAlignment.Center
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
            Label {
                text: qsTr("Password") + Retranslate.onLocaleOrLanguageChanged
                textStyle {
                    base: SystemDefaults.TextStyles.PrimaryText
                }
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                bottomMargin: 20
                TextField {
                    id: passwordEdit
                    hintText: qsTr("Enter password") + Retranslate.onLocaleOrLanguageChanged
                    inputMode: TextFieldInputMode.Password
                    rightPadding: 50
                    text: "qwe$t"
                    input.submitKey: SubmitKey.Done
                    input.onSubmitted: openDbAction.triggered()
                }
                Button {
                    accessibility.name: qsTr("Choose file") + Retranslate.onLocaleOrLanguageChanged
                    imageSource: "asset:///images/ic_browse.png"
                    preferredWidth: 50
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    opacity: 0
                }
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                TextField {
                    id: keyFileEdit
                    //text: "/shared/documents/test.dat"
                    focusPolicy: FocusPolicy.None
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment: VerticalAlignment.Center
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
        openDbAction.triggered(); //TODO remove this debug line
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

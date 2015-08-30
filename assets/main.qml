/*
 * Copyright (c) 2011-2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import bb.cascades 1.2
import bb.system 1.2
import QtQuick 1.0
import org.keepassb 1.0
import "common.js" as Common

NavigationPane {
    id: naviPane
    
    // emitted when the DB is successfully saved
    signal databaseSaved()
    
    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered : {
                var settingsPage = settingsPageComponent.createObject();
                naviPane.push(settingsPage);
            }
        }
        helpAction: HelpActionItem {
            title: qsTr("About", "A button/action which opens an information window about the application.") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_about.png"
            onTriggered : {
                var aboutPage = aboutPageComponent.createObject();
                naviPane.push(aboutPage);
            }
        }
        actions: [
            ActionItem {
                title: (appSettings.quickUnlockEnabled ? qsTr("Quick Lock", "A button which engages Quick Lock mode (see thesaurus).") : qsTr("Lock", "A button which closes/locks the current database.")) + Retranslate.onLocaleOrLanguageChanged
                imageSource: (appSettings.quickUnlockEnabled ? "asset:///images/ic_quicklock.png" : "asset:///images/ic_lock.png")
                enabled: !database.locked
                onTriggered: {
                    app.lock();
                }
            },
            ActionItem {
                title: qsTr("Change Master Key", "A button/action to change the master key (main password) of the database") + Retranslate.onLocaleOrLanguageChanged
				imageSource: "asset:///images/ic_edit_key.png"
                enabled: !database.locked && database.isEditable();
                onTriggered: {
                    var changeMasterKeySheet = changeMasterKeySheetComponent.createObject(naviPane.top, {creationMode: false});
                    changeMasterKeySheet.open();
                    changeMasterKeySheet.autofocus();
                }
            }
        ]
    }

    firstPage: UnlockDbPage {
        id: unlockDbPage
        objectName: "unlockDbPage"
        onDatabaseUnlocked: {
            app.restartWatchdog();
            
            database.dbAboutToSave.connect(function() {
                saveProgressDialog.show();
            });
            database.dbSaveError.connect(showFileSaveError);
            database.fileSaveError.connect(showFileSaveError);
            database.progressChanged.connect(function(progress) {
                saveProgressDialog.progress = progress;
            });
            database.dbSaved.connect(function() {
                saveProgressDialog.cancel();
                dbSavedConfirmationToast.show();
                databaseSaved();
            });

            var viewGroupPage = Qt.createComponent("ViewGroupPage.qml");
            var groupPage = viewGroupPage.createObject(null, {"group": database.rootGroup});
            naviPane.push(groupPage);
            if (appSettings.searchAfterUnlock) {
                groupPage.startSearch();
            }
        }
    }

    onCreationCompleted: {
        Qt.app = app; // a hack to make 'app' available from ListItemComponent
        Qt.database = database; // a hack to make 'database' available from ListItemComponent
        Qt.appSettings = appSettings; // a hack to make 'appSettings' available from ListItemComponent
        
        app.clipboardUpdated.connect(function() {
                showClipboardToast(qsTr("Copied to clipboard", "A notification which confirms successful copying of text.") + Retranslate.onLocaleOrLanguageChanged)
            });
        app.clipboardCleared.connect(function() {
                showClipboardToast(qsTr("Clipboard cleared", "A notification message") + Retranslate.onLocaleOrLanguageChanged)
            });
        database.dbLocked.connect(function() {
                app.stopWatchdog();
                naviPane.navigateTo(naviPane.firstPage);
                quickUnlockPage.close();
            });
        app.appLocked.connect(function() {
                app.stopWatchdog();
                quickUnlockPage.open();
                quickUnlockPage.autoFocus();                
            });

        if (!app.canAccessSharedFiles()) {
            filePermissionErrorDialog.show();
        }
    }
    
    attachedObjects: [
        ComponentDefinition {
            id: viewGroupPage
            source: "ViewGroupPage.qml"
        },
        ComponentDefinition {
            id: settingsPageComponent
            source: "AppSettings.qml"
        },
        ComponentDefinition {
            id: aboutPageComponent
            source: "AboutPage.qml"
        },
        QuickUnlockPage {
            id: quickUnlockPage
        },
        SystemToast {
            id: clipboardToast  
        },
        SystemToast {
            id: dbSavedConfirmationToast
            body: qsTr("Database saved", "A short notification/confirmation message") + Retranslate.onLocaleOrLanguageChanged
        },
        SystemDialog {
            id: dbSaveErrorDialog
            title: qsTr("Error", "Title of an error notification popup") + Retranslate.onLocaleOrLanguageChanged
            cancelButton.label: ""
        },
        SystemProgressDialog {
            id: saveProgressDialog
            title: qsTr("Saving...", "Title of a progress indicator while a database is being encrypted/saved.") + Retranslate.onLocaleOrLanguageChanged
            progress: -1; // infinite
            autoUpdateEnabled: true
            confirmButton.label: ""
            cancelButton.label: ""
        },
        ComponentDefinition {
            id: changeMasterKeySheetComponent
            source: "ChangeMasterKeyPage.qml"
        },
        SystemDialog {
            id: filePermissionErrorDialog
            title: qsTr("File Access", "Title of a message asking user's permission to access files in phone's memory.") + Retranslate.onLocaleOrLanguageChanged
            body: qsTr("In order to work correctly, KeePassB needs your permission to access file storage of this device. Please change permission settings and restart the app.", "Instructions shown when the user has not allowed the app to access files in phone's memory.") + Retranslate.onLocaleOrLanguageChanged
            cancelButton.label: ""
            confirmButton.label: qsTr("Change Settings...", "A button which will open system settings.") + Retranslate.onLocaleOrLanguageChanged
            onFinished: {
                invokePermissionSettings.trigger("bb.action.OPEN");
                // The user might come back for the instructions, so keep them shown
                filePermissionErrorDialog.show();
            }
        },
        Invocation {
            id: invokePermissionSettings
            query {
                invokeActionId: "bb.action.OPEN"
                invokeTargetId: "sys.settings.target"
                data: "org.KeePassB"
                mimeType: "settings/view"
                uri: "settings://permissions"
            }
        },
        SystemDialog {
            property PwEntry entry
            id: multiCopyInfoDialog
            title: qsTr("Multi-Copy", "Title of a window describing how Multi-Copy function works") + Retranslate.onLocaleOrLanguageChanged
            body: qsTr("Multi-Copy makes it easier to copy and paste both user name and password.\n1) Switch to the target app.\n2) Paste the user name.\n3) Wave your palm in front of the phone's earpiece (the phone will vibrate briefly).\n4) Now paste the password.",
                "Brief description of the Multi-Copy function. Try to keep it brief.") + Retranslate.onLocaleOrLanguageChanged
            cancelButton.label: ""
            onFinished: {
                // The info was shown, so consider Multi Copy was used once.
                appSettings.multiCopyFirstUse = false;
                Common.performMultiCopy(entry, true);
            }
        }
    ]

    function showClipboardToast(message) {
        clipboardToast.body = message; 
        clipboardToast.show();
    }

    function showFileSaveError(message, errorDescription) {
        saveProgressDialog.cancel();
        dbSaveErrorDialog.body = qsTr("%1\n(%2)",
                "A template for 'Error message (Error description)'; change only for right-to-left langugages")
                .arg(message)
                .arg(errorDescription); 
        dbSaveErrorDialog.show();
    }

    onPopTransitionEnded: {
        page.destroy();
    }

    onPushTransitionEnded: {
        // auto focusing on UnlockDbPage's text field requires this weird workaround
        if (page == unlockDbPage) {
            unlockDbPage.focusOnPassword();
        }
        if (page.hasOwnProperty("autofocus") && page.autofocus) {
            page.performAutofocus();
        }
    }

    onTopChanged: {
        if (page === firstPage) {
            if (!database.locked) {
                database.lock();
            }
            unlockDbPage.focusOnPassword();
        } else {
            app.restartWatchdog();
        }
    }
}

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

NavigationPane {
    id: naviPane
    
    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered : {
                var settingsPage = settingsPageComponent.createObject();
                naviPane.push(settingsPage);
            }
        }
        helpAction: HelpActionItem {
            title: qsTr("About") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_about.png"
            onTriggered : {
                var aboutPage = aboutPageComponent.createObject();
                naviPane.push(aboutPage);
            }
        }
        actions: [
            ActionItem {
                title: qsTr("Lock") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_lock.png"
                enabled: !database.locked
                onTriggered: {
                    app.lock();
                }
            }
        ]
    }

    firstPage: UnlockDbPage {
        id: unlockDbPage
        objectName: "unlockDbPage"
        // dbFilePath: - set in onCreationCompleted
        // keyFilePath: - set in onCreationCompleted
        onDatabaseUnlocked: {
            app.restartWatchdog();
            var viewGroupPage = Qt.createComponent("ViewGroupPage.qml");
            var groupPage = viewGroupPage.createObject(null, {"group": database.rootGroup});
            naviPane.push(groupPage);
        }
    }
          
    onCreationCompleted: {
        Qt.app = app;
        unlockDbPage.dbFilePath = appSettings.recentDbPath;
        unlockDbPage.keyFilePath = appSettings.recentKeyFilePath;
        
        app.clipboardUpdated.connect(function() {
                showToast(qsTr("Copied to clipboard") + Retranslate.onLocaleOrLanguageChanged)
            });
        app.clipboardCleared.connect(function() {
                showToast(qsTr("Clipboard cleared") + Retranslate.onLocaleOrLanguageChanged)
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
            id: toast  
        }
    ]

    function showToast(message) {
        toast.body = message; 
        toast.show();
    }

    onPopTransitionEnded: {
        page.destroy();
    }

    onPushTransitionEnded: {
        // auto focusing on UnlockDbPage's text field requires this weird workaround
        if (page == unlockDbPage) {
            unlockDbPage.focusOnPassword();
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

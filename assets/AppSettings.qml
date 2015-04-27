import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common
PageWithWatchdog {
    titleBar: TitleBar {
        title: qsTr("Settings", "Title of the configuration/preferences screen") + Retranslate.onLocaleOrLanguageChanged
    }

    ScrollView {
        scrollRole: ScrollRole.Main
        Container {
            layout: StackLayout { orientation: LayoutOrientation.TopToBottom }
            StandardListItem {
                title: qsTr("Timeouts", "Title of the settings group which configures several time intervals") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                imageSource: "asset:///images/ic_history.png"
                onTouch: {
                    if (event.isUp()) {
                        var timeoutSettingsPage = timeoutSettingsPageComponent.createObject(this);                        
                        naviPane.push(timeoutSettingsPage);
                    }
                }
            }
            StandardListItem {
                title: qsTr("Smart Copy", "Title of the settings group which configures advanced copying to clipboard") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                imageSource: "asset:///images/ic_copy.png"
                onTouch: {
                    if (event.isUp()) {
                        var smartCopySettingsPage = smartCopySettingsPageComponent.createObject(this);
                        naviPane.push(smartCopySettingsPage);
                    }
                }                
            }
            StandardListItem {
                title: qsTr("Quick Unlock", "Title of the settings group related to the Quick Unlock function (see thesaurus)") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                imageSource: "asset:///images/ic_quicklock.png"
                onTouch: {
                    if (event.isUp()) {
                        var quickUnlockSettingsPage = quickUnlockSettingsPageComponent.createObject(this);
                        naviPane.push(quickUnlockSettingsPage);
                    }
                }                
            }
            StandardListItem {
                title: qsTr("Lists", "Title of a group of settings which define how various lists of items should be displayed.") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                imageSource: "asset:///images/ic_view_details.png"
                onTouch: {
                    if (event.isUp()) {
                        var listsSettingsPage = listsSettingsPageComponent.createObject(this);
                        naviPane.push(listsSettingsPage);
                    }
                }                
            }
            StandardListItem {
                title: qsTr("Saving", "Title of a group of settings related to database editing/saving function") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                imageSource: "asset:///images/ic_save.png"
                onTouch: {
                    if (event.isUp()) {
                        var savingSettingsPage = savingSettingsPageComponent.createObject(this);
                        naviPane.push(savingSettingsPage);
                    }
                }                
            }
            StandardListItem {
                title: qsTr("Search", "Title of a group of settings related to search function") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                imageSource: "asset:///images/ic_browse.png"
                onTouch: {
                    if (event.isUp()) {
                        var searchSettingsPage = searchSettingsPageComponent.createObject(this);
                        naviPane.push(searchSettingsPage);
                    }
                }                
            }
            StandardListItem {
                title: qsTr("Recent Files", "Title of a group of settings related to handling of recently opened files.") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                imageSource: "asset:///images/ic_history.png"
                onTouch: {
                    if (event.isUp()) {
                        var recentFilesSettingsPage = recentFilesSettingsPageComponent.createObject(this);
                        naviPane.push(recentFilesSettingsPage);
                    }
                }                
            }
            // Divider {} -- list item above has its own borders
            Header {
                title: qsTr("Support", "Title of the settings group with links to 'contact us'") + Retranslate.onLocaleOrLanguageChanged
            }
            StandardListItem {
                focusPolicy: FocusPolicy.KeyAndTouch
                focusAutoShow: FocusAutoShow.Default
                title: qsTr("Contact Us", "Button/action to start writing an email to the developer") + Retranslate.onLocaleOrLanguageChanged
                description: qsTr("Suggestions? Problems? Let us know!", "Subtitle for the 'Contact us' button") + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_email.png"
                onTouch: {
                    if (event.isUp())
                        invokeSendEmail.trigger("bb.action.SENDEMAIL");
                }
                attachedObjects: [
                    Invocation {
                        id: invokeSendEmail
                        query {
                            uri: "mailto:blackberry@popleteev.com?subject=" + Application.applicationName 
                            + "%20" + Application.applicationVersion + ""
                            invokeActionId: "bb.action.SENDEMAIL"
                            invokeTargetId: "sys.pim.uib.email.hybridcomposer"
                        }
                    }
                ]
            }
            StandardListItem {
                title: qsTr("Recommend KeePassB", "Button/action to open the app store and rate this app.") + Retranslate.onLocaleOrLanguageChanged
                description: qsTr("Version %1", "Application version number. %s will be replaced with the actual number, for example: 'Version 2.3.4'").arg(Application.applicationVersion) + Retranslate.onLocaleOrLanguageChanged
                imageSource: "asset:///images/ic_rate_app.png"
                onTouch: {
                    if (event.isUp())
                        invokeRecommendApp.trigger("bb.action.OPEN");
                }
                attachedObjects: [
                    Invocation {
                        id: invokeRecommendApp
                        query {
                            mimeType: "application/x-bb-appworld"
                            uri: "appworld://content/59933812"
                        }
                    }
                ]
            }
        }
    }
    attachedObjects: [
        ComponentDefinition {
            id: quickUnlockHelpComponent
            source: "QuickUnlockHelp.qml"
        },
        SystemToast {
            id: changesConfirmationToast
            body: qsTr("Changes have been applied", "Notification after the user changes some settings with immediate effect (e.g. clear history of recent files).")
        },
        ComponentDefinition {
            id: timeoutSettingsPageComponent
            PageWithWatchdog {
                titleBar: TitleBar {
                    title: qsTr("Timeouts", "Title of the settings group which configures several time intervals") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    DropDown {
                        id: lockTimeout
                        title: qsTr("Lock Database", "A setting specifying the time interval after which the database will be automatically closed. Will be shown with one of its possible values, e.g. 'Lock Database    After 15 seconds'.") + Retranslate.onLocaleOrLanguageChanged
                        onSelectedOptionChanged: {
                            if (selectedOption) {
                                appSettings.autoLockTimeout = selectedOption.value;
                            }
                        }
                        Option {
                            text: qsTr("When Minimized", "One of the possible values of the 'Lock Database' timeout option. Will be displayed as 'Lock Database    When Minimized'") + Retranslate.onLocaleOrLanguageChanged
                            value: 0
                            selected: (appSettings.autoLockTimeout == 0)
                        }
                        Option {
                            text: qsTr("After 15 seconds", "One of the possible values of the 'Lock Database' timeout option. Will be displayed as 'Lock Database    After 15 seconds'") + Retranslate.onLocaleOrLanguageChanged
                            value: 15 * 1000
                            selected: (appSettings.autoLockTimeout == 15 * 1000)
                        }
                        Option {
                            text: qsTr("After 30 seconds", "One of the possible values of the 'Lock Database' timeout option. Will be displayed as 'Lock Database    After 30 seconds'") + Retranslate.onLocaleOrLanguageChanged
                            value: 30 * 1000
                            selected: (appSettings.autoLockTimeout == 30 * 1000)
                        }
                        Option {
                            text: qsTr("After 1 minute", "One of the possible values of the 'Lock Database' timeout option. Will be displayed as 'Lock Database    After 1 minute'") + Retranslate.onLocaleOrLanguageChanged
                            value: 60 * 1000
                            selected: (appSettings.autoLockTimeout == 60 * 1000)
                        }
                        Option {
                            text: qsTr("After 2 minutes", "One of the possible values of the 'Lock Database' timeout option. Will be displayed as 'Lock Database    After 2 minutes'") + Retranslate.onLocaleOrLanguageChanged
                            value: 2 * 60 * 1000
                            selected: (appSettings.autoLockTimeout == 120 * 1000)
                        }
                        Option {
                            text: qsTr("After 5 minutes", "One of the possible values of the 'Lock Database' timeout option. Will be displayed as 'Lock Database    After 5 minutes'") + Retranslate.onLocaleOrLanguageChanged
                            value: 5 * 60 * 1000
                            selected: (appSettings.autoLockTimeout == 300 * 1000)
                        }
                        Option {
                            text: qsTr("Never", "One of the possible values of the 'Lock Database' timeout option. Will be displayed as 'Lock Database    Never'") + Retranslate.onLocaleOrLanguageChanged
                            value: -1
                            selected: (appSettings.autoLockTimeout == -1)
                        }
                    }
                    Divider {}
                    DropDown {
                        id: clipboardTimeout
                        title: qsTr("Clear Clipboard", "A setting specifying the time interval after which the clipboard will be automatically cleared. Will be shown with one of its values, e.g. 'Clear Clipboard    After 10 seconds'") + Retranslate.onLocaleOrLanguageChanged
                        onSelectedOptionChanged: {
                            if (selectedOption) {
                                appSettings.clipboardTimeout = selectedOption.value;
                            }
                        }
                        Option {
                            text: qsTr("After 10 seconds", "One of the possible values of the 'Clear Clipboard' timeout option. Will be displayed as 'Clear Clipboard    After 10 seconds'") + Retranslate.onLocaleOrLanguageChanged
                            value: 10 * 1000;
                            selected: (appSettings.clipboardTimeout == 10 * 1000)
                        }
                        Option {
                            text: qsTr("After 20 seconds", "One of the possible values of the 'Clear Clipboard' timeout option. Will be displayed as 'Clear Clipboard    After 20 seconds'") + Retranslate.onLocaleOrLanguageChanged
                            value: 20 * 1000;
                            selected: (appSettings.clipboardTimeout == 20 * 1000)
                        }
                        Option {
                            text: qsTr("After 30 seconds", "One of the possible values of the 'Clear Clipboard' timeout option. Will be displayed as 'Clear Clipboard    After 30 seconds'") + Retranslate.onLocaleOrLanguageChanged
                            value: 30 * 1000;
                            selected: (appSettings.clipboardTimeout == 30 * 1000)
                        }
                        Option {
                            text: qsTr("After 1 minute", "One of the possible values of the 'Clear Clipboard' timeout option. Will be displayed as 'Clear Clipboard    After 1 minute'") + Retranslate.onLocaleOrLanguageChanged
                            value: 60 * 1000;
                            selected: (appSettings.clipboardTimeout == 60 * 1000)
                        }
                        Option {
                            text: qsTr("After 2 minutes", "One of the possible values of the 'Clear Clipboard' timeout option. Will be displayed as 'Clear Clipboard    After 2 minutes'") + Retranslate.onLocaleOrLanguageChanged
                            value: 2 * 60 * 1000;
                            selected: (appSettings.clipboardTimeout == 2 * 60 * 1000)
                        }
                        Option {
                            text: qsTr("Never", "One of the possible values of the 'Clear Clipboard' timeout option. Will be displayed as 'Clear Clipboard    Never'") + Retranslate.onLocaleOrLanguageChanged
                            value: -1;
                            selected: (appSettings.clipboardTimeout == -1)
                        }
                    }
                    Divider {}
                }
            }
        },
        ComponentDefinition {
            id: smartCopySettingsPageComponent
            PageWithWatchdog {
                onCreationCompleted: {
                    minimizeAppOnCopy.checked = appSettings.minimizeAppOnCopy;
                    minimizeAppOnCopy.checkedChanged.connect(appSettings.setMinimizeAppOnCopy);
                }
                
                titleBar: TitleBar {
                    title: qsTr("Smart Copy", "Title of the settings group which configures advanced copying to clipboard") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    Container {
                        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                        horizontalAlignment: HorizontalAlignment.Fill
                        Label {
                            horizontalAlignment: HorizontalAlignment.Left
                            verticalAlignment: VerticalAlignment.Center
                            text: qsTr("Minimize on Copy", "An on/off setting to make backup copies of the database every time it is saved.") + Retranslate.onLocaleOrLanguageChanged
                            textStyle.base: SystemDefaults.TextStyles.PrimaryText
                            layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                            multiline: true
                        }
                        ToggleButton {
                            id: minimizeAppOnCopy
                            horizontalAlignment: HorizontalAlignment.Right
                            verticalAlignment: VerticalAlignment.Top
                        }
                    }
                    Label {
                        horizontalAlignment: HorizontalAlignment.Fill
                        text: qsTr("This option will automatically minimize the application whenever you copy an entry field to clipboard.", "Description of the 'Minimize on Copy' setting.")
                        textStyle.base: SystemDefaults.TextStyles.SubtitleText
                        textStyle.color: Color.Gray
                        multiline: true
                    }
                    Divider {}
                }
            }
        },
        ComponentDefinition {
            id: quickUnlockSettingsPageComponent
            PageWithWatchdog {
                titleBar: TitleBar {
                    title: qsTr("Quick Unlock", "Title of the settings group related to the Quick Unlock function (see thesaurus)") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    DropDown {
                        id: quickUnlockType
                        onSelectedOptionChanged: {
                            if (selectedOption) {
                                appSettings.quickUnlockType = selectedOption.value;
                            }
                        }
                        title: qsTr("Quick Password", "A setting specifying the type of the password to use with Quick Unlock.  Example: 'Quick Password    First 3 symbols of the password'") + Retranslate.onLocaleOrLanguageChanged
                        Option {
                            text: Common.getQuickUnlockTypeDescription(Settings.QUICK_UNLOCK_FIRST_3) + Retranslate.onLocaleOrLanguageChanged
                            value: Settings.QUICK_UNLOCK_FIRST_3
                            selected: (appSettings.quickUnlockType == Settings.QUICK_UNLOCK_FIRST_3)
                        }
                        Option {
                            text: Common.getQuickUnlockTypeDescription(Settings.QUICK_UNLOCK_FIRST_4) + Retranslate.onLocaleOrLanguageChanged
                            value: Settings.QUICK_UNLOCK_FIRST_4
                            selected: (appSettings.quickUnlockType == Settings.QUICK_UNLOCK_FIRST_4)
                        }
                        Option {
                            text: Common.getQuickUnlockTypeDescription(Settings.QUICK_UNLOCK_FIRST_5) + Retranslate.onLocaleOrLanguageChanged
                            value: Settings.QUICK_UNLOCK_FIRST_5
                            selected: (appSettings.quickUnlockType == Settings.QUICK_UNLOCK_FIRST_5)
                        }
                        Option {
                            text: Common.getQuickUnlockTypeDescription(Settings.QUICK_UNLOCK_LAST_3) + Retranslate.onLocaleOrLanguageChanged
                            value: Settings.QUICK_UNLOCK_LAST_3
                            selected: (appSettings.quickUnlockType == Settings.QUICK_UNLOCK_LAST_3)
                        }
                        Option {
                            text: Common.getQuickUnlockTypeDescription(Settings.QUICK_UNLOCK_LAST_4) + Retranslate.onLocaleOrLanguageChanged
                            value: Settings.QUICK_UNLOCK_LAST_4
                            selected: (appSettings.quickUnlockType == Settings.QUICK_UNLOCK_LAST_4)
                        }
                        Option {
                            text: Common.getQuickUnlockTypeDescription(Settings.QUICK_UNLOCK_LAST_5) + Retranslate.onLocaleOrLanguageChanged
                            value: Settings.QUICK_UNLOCK_LAST_5
                            selected: (appSettings.quickUnlockType == Settings.QUICK_UNLOCK_LAST_5)
                        }
                    }
                    Divider {}
                    ScrollView {
                        verticalAlignment: VerticalAlignment.Fill
                        scrollRole: ScrollRole.Main
                        Label {
                            text: qsTr("Quick Unlock is a function which enables you to re-access your database using a short \"quick password\".\nPlease enable Quick Unlock before opening your database with the full master password. Thus KeePassB will remember the Quick Password - a few characters of the master password (actually, their SHA-256 hash).\nIn this mode, the database is only partially locked: it remains decrypted in the memory, but the user interface is covered by Quick Unlock screen asking for the Quick Password. If the entered password matches the one remembered, you get access to the database. Otherwise, the database is completely closed and cleared from the memory and will require the full master password again. There is only one attempt to enter correct Quick Password.\nNOTE: For safety reasons, the bottom-left Lock button always closes the database completely, even in Quick Unlock mode.", "Information about Quick Lock function.") + Retranslate.onLocaleOrLanguageChanged
                            multiline: true
                            textFormat: TextFormat.Html
                            textStyle.base: SystemDefaults.TextStyles.BodyText
                        }
                    }
                }
            }
        },
        ComponentDefinition {
            id: listsSettingsPageComponent
            PageWithWatchdog {
                titleBar: TitleBar {
                    title: qsTr("Lists", "Title of a group of settings which define how various lists of items should be displayed.") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    DropDown {
                        id: entryListDetails
                        title: qsTr("Entry List Details", "A setting which defines additional information (details) to be displayed for each entry in a list (also see thesaurus for 'Entry'). Example: 'Entry List Details    Last Modification Time'") + Retranslate.onLocaleOrLanguageChanged
                        onSelectedOptionChanged: {
                            if (selectedOption) {
                                appSettings.entryListDetail = selectedOption.value;
                            }
                        }
                        Option {
                            text: qsTr("None", "One of the possible values of the 'Entry List Details' setting. Will be displayed as 'Entry List Details    None', meaning that no entry details will be shown in any lists.") + Retranslate.onLocaleOrLanguageChanged
                            value: 0
                            selected: appSettings.entryListDetail == 0
                        }
                        Option {
                            text: qsTr("User Name", "One of the possible values of the 'Entry List Details' setting; it refers to login information rather then person name. Will be displayed as 'Entry List Details    User Name'.") + Retranslate.onLocaleOrLanguageChanged
                            value: 1
                            selected: appSettings.entryListDetail == 1
                        }
                        Option {
                            text: qsTr("Password", "One of the possible values of the 'Entry List Details' setting. Will be displayed as 'Entry List Details    Password'.") + Retranslate.onLocaleOrLanguageChanged
                            value: 2
                            selected: appSettings.entryListDetail == 2
                        }
                        Option {
                            text: qsTr("URL", "One of the possible values of the 'Entry List Details' setting; it means 'internet address' or 'internet link'. Will be displayed as 'Entry List Details    URL'.") + Retranslate.onLocaleOrLanguageChanged
                            value: 4
                            selected: appSettings.entryListDetail == 4
                        }
                        Option {
                            text: qsTr("Notes", "One of the possible values of the 'Entry List Details' setting; it refers to comments or additional text information contained in an entry. Will be displayed as 'Entry List Details    Notes'.") + Retranslate.onLocaleOrLanguageChanged
                            value: 8
                            selected: appSettings.entryListDetail == 8
                        }
                        Option {
                            text: qsTr("Last Modified Time", "One of the possible values of the 'Entry List Details' setting; it refers to the most recent time when the entry was modified. Will be displayed as 'Entry List Details    Last Modified Time'. ") + Retranslate.onLocaleOrLanguageChanged
                            value: 16
                            selected: appSettings.entryListDetail == 16
                        }
                    }
                    Divider {}
                    DropDown {
                        id: alphaSorting
                        title: qsTr("List Sorting", "A setting which defines sorting of entries in groups. Example: 'List Sorting    Alphabetical'.") + Retranslate.onLocaleOrLanguageChanged
                        onSelectedOptionChanged: {
                            if (selectedOption) {
                                appSettings.alphaSorting = selectedOption.value;
                            }
                        }
                        Option {
                            text: qsTr("None", "One of the possible values of the 'List Sorting' setting. Will be displayed as 'List Sorting    None', meaning no sorting, or as-is item order.") + Retranslate.onLocaleOrLanguageChanged
                            value: false
                            selected: !appSettings.alphaSorting
                        }
                        Option {
                            text: qsTr("Alphabetical", "One of the possible values of the 'List Sorting' setting. Will be displayed as 'List Sorting    Alphabetical'.") + Retranslate.onLocaleOrLanguageChanged
                            value: true
                            selected: appSettings.alphaSorting
                        }
                    }
                    Divider {}
                }
            }
        },
        ComponentDefinition {
            id: savingSettingsPageComponent
            PageWithWatchdog {
                onCreationCompleted: {
                    backupDatabaseOnSave.checked = appSettings.backupDatabaseOnSave;
                    backupDatabaseOnSave.checkedChanged.connect(appSettings.setBackupDatabaseOnSave);
                }
                
                titleBar: TitleBar {
                    title: qsTr("Saving", "Title of a group of settings related to database editing/saving function") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    Container {
                        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                        horizontalAlignment: HorizontalAlignment.Fill
                        Label {
                            horizontalAlignment: HorizontalAlignment.Left
                            verticalAlignment: VerticalAlignment.Center
                            text: qsTr("Backup Database on Save", "An on/off setting to make backup copies of the database everytime it is saved.") + Retranslate.onLocaleOrLanguageChanged
                            textStyle.base: SystemDefaults.TextStyles.PrimaryText
                            layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                            multiline: true
                        }
                        ToggleButton {
                            id: backupDatabaseOnSave
                            horizontalAlignment: HorizontalAlignment.Right
                            verticalAlignment: VerticalAlignment.Top
                        }
                    }
                    Label {
                        horizontalAlignment: HorizontalAlignment.Fill
                        text: qsTr("Backup the database file before saving any changes. Backup copies are timestamped and stored along with the original database.", "Description of the 'Backup Database on Save' setting.")
                        textStyle.base: SystemDefaults.TextStyles.SubtitleText
                        textStyle.color: Color.Gray
                        multiline: true
                    }
                    Label {
                        horizontalAlignment: HorizontalAlignment.Fill
                        text: qsTr("Database saving is currently an experimental function. It is STONGLY advised to keep this option enabled in order to avoid accidental data loss.", "Description of the 'Backup Database on Save' setting.")
                        textStyle.base: SystemDefaults.TextStyles.SubtitleText
                        textStyle.fontStyle: FontStyle.Italic
                        textStyle.color: Color.Red
                        multiline: true
                    }
                    Divider {}
                }
            }
        },
        ComponentDefinition {
            id: searchSettingsPageComponent
            PageWithWatchdog {
                onCreationCompleted: {
                    searchInDeleted.checked = appSettings.searchInDeleted;
                    searchInDeleted.checkedChanged.connect(appSettings.setSearchInDeleted);
                    searchAfterUnlock.checked = appSettings.searchAfterUnlock;
                    searchAfterUnlock.checkedChanged.connect(appSettings.setSearchAfterUnlock);
                }
                
                titleBar: TitleBar {
                    title: qsTr("Search", "Title of a group of settings related to search function") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    Container {
                        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                        horizontalAlignment: HorizontalAlignment.Fill
                        Label {
                            horizontalAlignment: HorizontalAlignment.Left
                            verticalAlignment: VerticalAlignment.Center
                            text: qsTr("Search at Start", "An on/off setting which defines whether the Search function should be automatically activated when a database is opened/unlocked.") + Retranslate.onLocaleOrLanguageChanged
                            textStyle.base: SystemDefaults.TextStyles.PrimaryText
                            layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                            multiline: true
                        }
                        ToggleButton {
                            id: searchAfterUnlock
                            horizontalAlignment: HorizontalAlignment.Right
                            verticalAlignment: VerticalAlignment.Top
                        }
                    }
                    Label {
                        leftMargin: 10
                        rightMargin: 10
                        
                        horizontalAlignment: HorizontalAlignment.Fill
                        text: qsTr("This option will automatically activate Search whenever a database is unlocked.", "Description of the 'Search at Start' setting.")
                        textStyle.base: SystemDefaults.TextStyles.SubtitleText
                        textStyle.color: Color.Gray
                        multiline: true
                    }
                    Divider {}
                    Container {
                        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                        horizontalAlignment: HorizontalAlignment.Fill
                        Label {
                            horizontalAlignment: HorizontalAlignment.Left
                            verticalAlignment: VerticalAlignment.Center
                            text: qsTr("Search in Deleted Entries", "An on/off setting which defines whether the Search should look into entries marked as deleted.") + Retranslate.onLocaleOrLanguageChanged
                            textStyle.base: SystemDefaults.TextStyles.PrimaryText
                            layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                            multiline: true
                        }
                        ToggleButton {
                            id: searchInDeleted
                            horizontalAlignment: HorizontalAlignment.Right
                            verticalAlignment: VerticalAlignment.Top
                        }
                    }
                    Divider {}
                }
            }
        },
        ComponentDefinition {
            id: recentFilesSettingsPageComponent
            PageWithWatchdog {
                titleBar: TitleBar {
                    title: qsTr("Recent Files", "Title of a group of settings related to handling of recently opened files.") + Retranslate.onLocaleOrLanguageChanged
                }
                Container {
                    Container {
                        topPadding: 10
                        bottomMargin: 10
                        leftPadding: 10
                        rightPadding: 10
                        DropDown {
                            id: trackRecentFiles
                            title: qsTr("Keep Track Of", "A setting which defines which type of recently used files should be remembered. Example: 'Keep Track Of: Database and Key Files'") + Retranslate.onLocaleOrLanguageChanged
                            onSelectedOptionChanged: {
                                if (selectedOption) {
                                    if (selectedOption.value != appSettings.trackRecentFiles)
                                        changesConfirmationToast.show();
                                    appSettings.trackRecentFiles = selectedOption.value;
                                }
                            }
                            Option {
                                text: qsTr("Nothing", "One of the possible values of the 'Keep Track Of' setting. Will be displayed as 'Keep Track Of: Nothing', meaning that no history of recently used files will be kept.") + Retranslate.onLocaleOrLanguageChanged
                                value: Settings.TRACK_RECENT_FILES_NONE
                                selected: appSettings.trackRecentFiles == Settings.TRACK_RECENT_FILES_NONE
                            }
                            Option {
                                text: qsTr("Databases", "One of the possible values of the 'Keep Track Of' setting. Will be displayed as 'Keep Track Of: Databases', meaning that history of recently used files will include only database files, and no key files.") + Retranslate.onLocaleOrLanguageChanged
                                value: Settings.TRACK_RECENT_FILES_DB_ONLY
                                selected: appSettings.trackRecentFiles == Settings.TRACK_RECENT_FILES_DB_ONLY
                            }
                            Option {
                                text: qsTr("Databases and Key Files", "One of the possible values of the 'Keep Track Of' setting. Will be displayed as 'Keep Track Of: Databases and Key Files', meaning that history of recently used files will include both database files and their associated key files.") + Retranslate.onLocaleOrLanguageChanged
                                value: Settings.TRACK_RECENT_FILES_DB_AND_KEY
                                selected: appSettings.trackRecentFiles == Settings.TRACK_RECENT_FILES_DB_AND_KEY
                            }
                        }
                    }
                    Divider{}
                    StandardListItem {
                        id: clearRecentFiles
                        title: qsTr("Clear History", "Button/action which clears the history of recent files.") + Retranslate.onLocaleOrLanguageChanged
                        description: qsTr("Forget all recent files", "Explanation/subtitle for the 'Clear History' button.") + Retranslate.onLocaleOrLanguageChanged
                        horizontalAlignment: HorizontalAlignment.Fill
                        onTouch: {
                            if (event.isUp()) {
                                appSettings.clearRecentFiles();
                                changesConfirmationToast.show();
                            }
                        }
                    }
                }
            }
        }
    ]
}

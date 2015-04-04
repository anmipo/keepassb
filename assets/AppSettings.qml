import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common
Page {
    titleBar: TitleBar {
        title: qsTr("Settings", "Title of the configuration/preferences screen") + Retranslate.onLocaleOrLanguageChanged
    }
    onCreationCompleted: {
        searchInDeleted.checked = appSettings.searchInDeleted;
        searchInDeleted.checkedChanged.connect(appSettings.setSearchInDeleted);
        searchAfterUnlock.checked = appSettings.searchAfterUnlock;
        searchAfterUnlock.checkedChanged.connect(appSettings.setSearchAfterUnlock);
        backupDatabaseOnSave.checked = appSettings.backupDatabaseOnSave;
        backupDatabaseOnSave.checkedChanged.connect(appSettings.setBackupDatabaseOnSave);
        minimizeAppOnCopy.checked = appSettings.minimizeAppOnCopy;
        minimizeAppOnCopy.checkedChanged.connect(appSettings.setMinimizeAppOnCopy);
    }
    ScrollView {
        scrollRole: ScrollRole.Main
        Container {
            onTouchCapture: {
                app.restartWatchdog();
            }
            layout: StackLayout { orientation: LayoutOrientation.TopToBottom }
            leftPadding: 10
            rightPadding: 10
            Header {
                title: qsTr("Timeouts", "Title of the settings group which configures several time intervals") + Retranslate.onLocaleOrLanguageChanged
            }
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
            Header {
                title: qsTr("Smart Copy", "Title of the settings group which configures advanced copying to clipboard") + Retranslate.onLocaleOrLanguageChanged
            }
            Container {
                topMargin: 10
                layout: StackLayout { 
                    orientation: LayoutOrientation.LeftToRight
                }
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
            Header {
                title: qsTr("Quick Unlock", "Title of the settings group related to the Quick Unlock function (see thesaurus)") + Retranslate.onLocaleOrLanguageChanged
            }
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
            Label {
                text: qsTr("Learn more", "A button which opens a help page about the Quick Unlock function") + Retranslate.onLocaleOrLanguageChanged
                textStyle.color: Color.create("#FF0073BC")
                topMargin: 10
                bottomMargin: 30
                textFormat: TextFormat.Html
                textStyle.base: SystemDefaults.TextStyles.SubtitleText
                multiline: true
                gestureHandlers: TapHandler {
                    onTapped: {
                        var quickUnlockHelpPage = quickUnlockHelpComponent.createObject();
                        naviPane.push(quickUnlockHelpPage);
                    }
                }
            }
            Header {
                title: qsTr("Lists", "Title of a group of settings which define how various lists of items should be displayed.") + Retranslate.onLocaleOrLanguageChanged
            }
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
            Header {
                title: qsTr("Saving", "Title of a group of settings related to database editing/saving function") + Retranslate.onLocaleOrLanguageChanged
            }
            Container {
                topMargin: 10
                layout: StackLayout { 
                    orientation: LayoutOrientation.LeftToRight
                }
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
            Divider{}
            Header {
                title: qsTr("Search", "Title of a group of settings related to search function") + Retranslate.onLocaleOrLanguageChanged
            }
            Container {
                topMargin: 10
                layout: StackLayout { 
                    orientation: LayoutOrientation.LeftToRight
                }
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
                horizontalAlignment: HorizontalAlignment.Fill
                text: qsTr("This option will automatically activate Search whenever a database is unlocked.", "Description of the 'Search at Start' setting.")
                textStyle.base: SystemDefaults.TextStyles.SubtitleText
                textStyle.color: Color.Gray
                multiline: true
            }
            Divider{}
            Container {
                topMargin: 10
                bottomMargin: 10
                layout: StackLayout { 
                    orientation: LayoutOrientation.LeftToRight
                }
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
            Header {
                title: qsTr("Recent Files", "Title of a group of settings related to handling of recently opened files.") + Retranslate.onLocaleOrLanguageChanged
            }
            Button {
                id: clearRecentFiles
                text: qsTr("Clear History", "Button/action which clears the history of recent files.") + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                onClicked: {
                    appSettings.clearRecentFiles();
                    restartToApplyToast.show();
                }
            }
            Divider { }
        }
    }
    attachedObjects: [
        ComponentDefinition {
            id: quickUnlockHelpComponent
            source: "QuickUnlockHelp.qml"
        },
        SystemToast {
            id: restartToApplyToast
            body: qsTr("Changes will apply after restart", "Notification after the user changes some settings which require restarting the app.")
        }
    ]
}

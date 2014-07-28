import bb.cascades 1.2
import org.keepassb 1.0
import "common.js" as Common
Page {
    titleBar: TitleBar {
        title: qsTr("Settings") + Retranslate.onLocaleOrLanguageChanged
    }
    onCreationCompleted: {
        searchInDeleted.checked = appSettings.searchInDeleted;
        searchInDeleted.checkedChanged.connect(appSettings.setSearchInDeleted);
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
                title: qsTr("Timeouts") + Retranslate.onLocaleOrLanguageChanged
            }
            DropDown {
                id: lockTimeout
                title: qsTr("Lock database") + Retranslate.onLocaleOrLanguageChanged
                onSelectedOptionChanged: {
                    if (selectedOption) {
                        appSettings.autoLockTimeout = selectedOption.value;
                    }
                }
                Option {
                    text: qsTr("When minimized") + Retranslate.onLocaleOrLanguageChanged
                    value: 0
                    selected: (appSettings.autoLockTimeout == 0)
                }
                Option {
                    text: qsTr("After 30 seconds") + Retranslate.onLocaleOrLanguageChanged
                    value: 30 * 1000
                    selected: (appSettings.autoLockTimeout == 30 * 1000)
                }
                Option {
                    text: qsTr("After 1 minute") + Retranslate.onLocaleOrLanguageChanged
                    value: 60 * 1000
                    selected: (appSettings.autoLockTimeout == 60 * 1000)
                }
                Option {
                    text: qsTr("After 2 minutes") + Retranslate.onLocaleOrLanguageChanged
                    value: 2 * 60 * 1000
                    selected: (appSettings.autoLockTimeout == 120 * 1000)
                }
                Option {
                    text: qsTr("After 5 minutes") + Retranslate.onLocaleOrLanguageChanged
                    value: 5 * 60 * 1000
                    selected: (appSettings.autoLockTimeout == 300 * 1000)
                }
                Option {
                    text: qsTr("Never") + Retranslate.onLocaleOrLanguageChanged
                    value: -1
                    selected: (appSettings.autoLockTimeout == -1)
                }
            }
            DropDown {
                id: clipboardTimeout
                title: qsTr("Clear clipboard") + Retranslate.onLocaleOrLanguageChanged
                onSelectedOptionChanged: {
                    if (selectedOption) {
                        appSettings.clipboardTimeout = selectedOption.value;
                    }
                }
                Option {
                    text: qsTr("After 10 seconds") + Retranslate.onLocaleOrLanguageChanged
                    value: 10 * 1000;
                    selected: (appSettings.clipboardTimeout == 10 * 1000)
                }
                Option {
                    text: qsTr("After 20 seconds") + Retranslate.onLocaleOrLanguageChanged
                    value: 20 * 1000;
                    selected: (appSettings.clipboardTimeout == 20 * 1000)
                }
                Option {
                    text: qsTr("After 30 seconds") + Retranslate.onLocaleOrLanguageChanged
                    value: 30 * 1000;
                    selected: (appSettings.clipboardTimeout == 30 * 1000)
                }
                Option {
                    text: qsTr("After 1 minute") + Retranslate.onLocaleOrLanguageChanged
                    value: 60 * 1000;
                    selected: (appSettings.clipboardTimeout == 60 * 1000)
                }
                Option {
                    text: qsTr("Never") + Retranslate.onLocaleOrLanguageChanged
                    value: -1;
                    selected: (appSettings.clipboardTimeout == -1)
                }
            }
            Header {
                title: qsTr("Quick Unlock") + Retranslate.onLocaleOrLanguageChanged
            }
            DropDown {
                id: quickUnlockType
                onSelectedOptionChanged: {
                    if (selectedOption) {
                        appSettings.quickUnlockType = selectedOption.value;
                    }
                }
                title: qsTr("Quick password") + Retranslate.onLocaleOrLanguageChanged
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
            Header {
                title: qsTr("Lists") + Retranslate.onLocaleOrLanguageChanged
            }
            DropDown {
                id: entryListDetails
                title: qsTr("Entry list details") + Retranslate.onLocaleOrLanguageChanged
                onSelectedOptionChanged: {
                    if (selectedOption) {
                        appSettings.entryListDetail = selectedOption.value;
                    }
                }
                Option {
                    text: qsTr("None") + Retranslate.onLocaleOrLanguageChanged
                    value: 0
                    selected: appSettings.entryListDetail == 0
                }
                Option {
                    text: qsTr("User name") + Retranslate.onLocaleOrLanguageChanged
                    value: 1
                    selected: appSettings.entryListDetail == 1
                }
                Option {
                    text: qsTr("Password") + Retranslate.onLocaleOrLanguageChanged
                    value: 2
                    selected: appSettings.entryListDetail == 2
                }
                Option {
                    text: qsTr("URL") + Retranslate.onLocaleOrLanguageChanged
                    value: 4
                    selected: appSettings.entryListDetail == 4
                }
                Option {
                    text: qsTr("Notes") + Retranslate.onLocaleOrLanguageChanged
                    value: 8
                    selected: appSettings.entryListDetail == 8
                }
                Option {
                    text: qsTr("Last modified time") + Retranslate.onLocaleOrLanguageChanged
                    value: 16
                    selected: appSettings.entryListDetail == 16
                }
            }
            DropDown {
                id: alphaSorting
                title: qsTr("Sort lists") + Retranslate.onLocaleOrLanguageChanged
                onSelectedOptionChanged: {
                    if (selectedOption) {
                        appSettings.alphaSorting = selectedOption.value;
                    }
                }
                Option {
                    text: qsTr("No sorting") + Retranslate.onLocaleOrLanguageChanged
                    value: false
                    selected: !appSettings.alphaSorting
                }
                Option {
                    text: qsTr("Alphabetically") + Retranslate.onLocaleOrLanguageChanged
                    value: true
                    selected: appSettings.alphaSorting
                }
            }
            Header {
                title: qsTr("Search") + Retranslate.onLocaleOrLanguageChanged
            }
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
                    text: qsTr("Search in Recycle Bin") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.PrimaryText
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }
                }
                ToggleButton {
                    id: searchInDeleted
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Top
                }
            }
            Divider { }
        }
    }
}

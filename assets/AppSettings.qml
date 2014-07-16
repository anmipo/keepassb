import bb.cascades 1.2
import org.keepassb 1.0

Page {
    titleBar: TitleBar {
        title: qsTr("Settings") + Retranslate.onLocaleOrLanguageChanged
    }
    onCreationCompleted: {
        app.restartWatchdog();
        searchInDeleted.checked = appSettings.searchInDeleted;
        searchInDeleted.checkedChanged.connect(appSettings.setSearchInDeleted);
    }
    ScrollView {
        scrollRole: ScrollRole.Main
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.TopToBottom
            }
            leftPadding: 10
            rightPadding: 10
            Label {
                text: qsTr("Lock database") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
            }
            DropDown {
                id: lockTimeout
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
            Divider { }
            Label {
                text: qsTr("Clear clipboard") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
            }
            DropDown {
                id: clipboardTimeout
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
            Divider { }
            Label {
                text: qsTr("Sort lists") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
            }
            DropDown {
                id: alphaSorting
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
            Label {
                id: alphaSortingNotice
                text: qsTr("Sorting order changes will be applied the next time you open a database.") + Retranslate.onLocaleOrLanguageChanged
                visible: !database.locked
                textStyle.base: SystemDefaults.TextStyles.SubtitleText
                multiline: true
            }
            Divider { }
            Label {
                text: qsTr("Entry list details") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
            }
            DropDown {
                id: entryListDetails
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
            }
            Divider { }
            Container {
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
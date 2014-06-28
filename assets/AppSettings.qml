import bb.cascades 1.2

Page {
    titleBar: TitleBar {
        title: qsTr("Settings") + Retranslate.onLocaleOrLanguageChanged
    }
    onCreationCompleted: {
        searchInDeleted.checked = appSettings.searchInDeleted;
        clipboardTimeout.selectedIndex = appSettings.clipboardTimeoutIndex;
        searchInDeleted.checkedChanged.connect(appSettings.setSearchInDeleted);
        clipboardTimeout.selectedIndexChanged.connect(appSettings.setClipboardTimeoutIndex);
    }
    Container {
        layout: StackLayout {
            orientation: LayoutOrientation.TopToBottom
        }
        Container {
            layout: StackLayout { 
                orientation: LayoutOrientation.LeftToRight
            }
            horizontalAlignment: HorizontalAlignment.Fill
            StandardListItem {
                horizontalAlignment: HorizontalAlignment.Left
                verticalAlignment: VerticalAlignment.Center
                title: qsTr("Search in Recycle Bin") + Retranslate.onLocaleOrLanguageChanged
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 1
                }
            }
            ToggleButton {
                id: searchInDeleted
                horizontalAlignment: HorizontalAlignment.Right
                verticalAlignment: VerticalAlignment.Center
            }
        }
        DropDown {
            id: clipboardTimeout
            title: qsTr("Clear clipboard") + Retranslate.onLocaleOrLanguageChanged
            // values of these options must be in sync with Settings::CLIPBOARD_TIMEOUT_INDEX_TO_SECONDS
            Option {
                text: qsTr("Never") + Retranslate.onLocaleOrLanguageChanged
                value: -1;
            }
            Option {
                text: qsTr("after 10 seconds") + Retranslate.onLocaleOrLanguageChanged
                value: 10;
            }
            Option {
                text: qsTr("after 30 seconds") + Retranslate.onLocaleOrLanguageChanged
                value: 30;
            }
            Option {
                text: qsTr("after 1 minute") + Retranslate.onLocaleOrLanguageChanged
                value: 60;
            }
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
                text: qsTr("Never") + Retranslate.onLocaleOrLanguageChanged
                value: -1
                selected: (appSettings.autoLockTimeout == -1)
            }
            Option {
                text: qsTr("Immediately") + Retranslate.onLocaleOrLanguageChanged
                value: 0
                selected: (appSettings.autoLockTimeout == 0)
            }
            Option {
                text: qsTr("after 10 seconds") + Retranslate.onLocaleOrLanguageChanged
                value: 10
                selected: (appSettings.autoLockTimeout == 10)
            }
            Option {
                text: qsTr("after 20 seconds") + Retranslate.onLocaleOrLanguageChanged
                value: 20
                selected: (appSettings.autoLockTimeout == 20)
            }
            Option {
                text: qsTr("after 30 seconds") + Retranslate.onLocaleOrLanguageChanged
                value: 30
                selected: (appSettings.autoLockTimeout == 30)
            }
            Option {
                text: qsTr("after 1 minute") + Retranslate.onLocaleOrLanguageChanged
                value: 60
                selected: (appSettings.autoLockTimeout == 60)
            }
            Option {
                text: qsTr("after 5 minutes") + Retranslate.onLocaleOrLanguageChanged
                value: 300
                selected: (appSettings.autoLockTimeout == 300)
            }
        }
    }
}
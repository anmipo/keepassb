import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0

Sheet {
    id: quickUnlockSheet
    peekEnabled: false
    
    // autofocus on password field
    function autoFocus() {
        quickPassEdit.requestFocus()
    }

    Page {
        titleBar: TitleBar {
            title: qsTr("Quick unlock") + Retranslate.onLocaleOrLanguageChanged
            acceptAction: ActionItem {
                id: unlockAction
                title: qsTr("Unlock") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    var pass = quickPassEdit.text;
                    quickPassEdit.text = "";
                    if (app.quickUnlock(pass)) {
                        quickUnlockSheet.close();
                    } else {
                        wrongCodeToast.show();
                        database.lock();
                    }
                }
            }
            dismissAction: ActionItem {
                title: qsTr("Lock") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    quickPassEdit.text = "";
                    database.lock();
                }
            }
            visibility: ChromeVisibility.Visible
        }
        Container {
            leftPadding: 10
            rightPadding: 10
            Label {
                text: {
                    switch (appSettings.quickUnlockType) {
                        case Settings.QUICK_UNLOCK_FIRST_3:
                            return qsTr("Enter first three symbols of your password to unlock:");
                        case Settings.QUICK_UNLOCK_FIRST_4:
                            return qsTr("Enter first four symbols of your password to unlock:");
                        case Settings.QUICK_UNLOCK_FIRST_5:
                            return qsTr("Enter first five symbols of your password to unlock:");
                        case Settings.QUICK_UNLOCK_LAST_3:
                            return qsTr("Enter last three symbols of your password to unlock:");
                        case Settings.QUICK_UNLOCK_LAST_4:
                            return qsTr("Enter last four symbols of your password to unlock:");
                        case Settings.QUICK_UNLOCK_LAST_5:
                            return qsTr("Enter last five symbols of your password to unlock:");
                    }
                }
                multiline: true
            }
            TextField {
                id: quickPassEdit
                inputMode: TextFieldInputMode.Password
                input.onSubmitted: unlockAction.triggered();
                input.submitKey: SubmitKey.EnterKey
            }
        }
        attachedObjects: [
            SystemToast {
                id: wrongCodeToast
                body: qsTr("Wrong unlock code") + Retranslate.onLocaleOrLanguageChanged
            }
        ]
    }
}
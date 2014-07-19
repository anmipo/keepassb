import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common

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
            visibility: ChromeVisibility.Visible
        }
        Container {
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            TextField {
                id: quickPassEdit
                hintText: Common.getQuickUnlockTypeDescription(appSettings.quickUnlockType) + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
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
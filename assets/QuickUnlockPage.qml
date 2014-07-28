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

    function tryQuickUnlock() {
        var pass = quickPassEdit.text;
        quickPassEdit.text = "";
        if (app.quickUnlock(pass)) {
            quickUnlockSheet.close();
        } else {
            wrongCodeToast.show();
            database.lock();
        }
    }
    
    Page {
        titleBar: TitleBar {
            title: qsTr("Quick Unlock") + Retranslate.onLocaleOrLanguageChanged
            dismissAction: ActionItem {
                title: qsTr("Cancel") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    quickPassEdit.text = "";
                    database.lock();
                }
            }
            visibility: ChromeVisibility.Visible
        }
        Container {
            topPadding: 20
            leftPadding: 10
            rightPadding: 10
            Label {
                text: qsTr("Enter quick password:") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.BodyText
            }
            TextField {
                id: quickPassEdit
                hintText: Common.getQuickUnlockTypeDescription(appSettings.quickUnlockType) + Retranslate.onLocaleOrLanguageChanged
                horizontalAlignment: HorizontalAlignment.Fill
                inputMode: TextFieldInputMode.Password
                input.onSubmitted: tryQuickUnlock()
                input.submitKey: SubmitKey.EnterKey
            }
            Button {
                topMargin: 20
                text: qsTr("Unlock") + Retranslate.onLocaleOrLanguageChanged
                onClicked: tryQuickUnlock()
                horizontalAlignment: HorizontalAlignment.Fill
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
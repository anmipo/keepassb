import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common

Sheet {
    id: quickUnlockSheet
    peekEnabled: false
    property string dbName
        
    // autofocus on password field
    function autoFocus() {
        quickPassEdit.requestFocus()
    }

    function tryQuickUnlock() {
        var pass = quickPassEdit.text;
        quickPassEdit.text = "";
        if (app.quickUnlock(pass)) {
            quickUnlockSheet.close();
            app.restartWatchdog();
        } else {
            wrongCodeToast.show();
            database.lock();
        }
    }
    
    onOpened: {
        dbName = Common.prettifyFilePath(database.getDatabaseFilePath());
    }
    Page {
        titleBar: TitleBar {
            title: qsTr("Quick Unlock", "Title of the Quick Unlock page") + Retranslate.onLocaleOrLanguageChanged
            dismissAction: ActionItem {
                title: qsTr("Cancel", "Cancel (close, back) button") + Retranslate.onLocaleOrLanguageChanged
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
                text: qsTr("Database: <i>%1</i>", "Currently opened database file. Example: 'Database: documents/database.dat'").arg(dbName) + Retranslate.onLocaleOrLanguageChanged
                multiline: true
                textFormat: TextFormat.Html
                textStyle.base: SystemDefaults.TextStyles.BodyText
            }
            Label {
                topMargin: 30
                text: qsTr("Enter quick password:", "An invitation to enter quick password; the latter should be translated similarly to the 'Quick Password' from Settings") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
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
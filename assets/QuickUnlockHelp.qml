import bb.cascades 1.2

Page {
    id: quickUnlockHelp
    titleBar: TitleBar {
        title: qsTr("About Quick Unlock", "Title of a help page about Quick Unlock function (see thesaurus).") + Retranslate.onLocaleOrLanguageChanged
        scrollBehavior: TitleBarScrollBehavior.Sticky
    }
    ScrollView {
        scrollRole: ScrollRole.Main
        Container {
            onTouchCapture: {
                app.restartWatchdog();
            }
            topPadding: 20
            leftPadding: 20
            rightPadding: 20
            bottomPadding: 20
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            Label {
                text: qsTr("Quick Unlock is a function which enables you to re-access your database using a short \"quick password\".\nPlease enable Quick Unlock before opening your database with the full master password. Thus KeePassB will remember the Quick Password - a few characters of the master password (actually, their SHA-256 hash).\nIn this mode, the database is only partially locked: it remains decrypted in the memory, but the user interface is covered by Quick Unlock screen asking for the Quick Password. If the entered password matches the one remembered, you get access to the database. Otherwise, the database is completely closed and cleared from the memory and will require the full master password again. There is only one attempt to enter correct Quick Password.\nNOTE: For safety reasons, the bottom-left Lock button always closes the database completely, even in Quick Unlock mode.", "Information about Quick Lock function.") + Retranslate.onLocaleOrLanguageChanged
                multiline: true
                textFormat: TextFormat.Html
                textStyle.base: SystemDefaults.TextStyles.BodyText
            }
        }
    }
}

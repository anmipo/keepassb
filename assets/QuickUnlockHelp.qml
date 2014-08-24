import bb.cascades 1.2

Page {
    id: quickUnlockHelp
    titleBar: TitleBar {
        title: qsTr("About Quick Unlock") + Retranslate.onLocaleOrLanguageChanged
        scrollBehavior: TitleBarScrollBehavior.Sticky
    }
    ScrollView {
        scrollRole: ScrollRole.Main
        Container {
            topPadding: 20
            leftPadding: 20
            rightPadding: 20
            bottomPadding: 20
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            Label {
                text: qsTr("To activate Quick Unlock, please enable it when you enter your full master password. KeePassB will remember a Quick Password — a hash of some characters of the master password.\n\nA quick-locked database remains decrypted in the memory, but is covered by a Quick Unlock screen. If the entered quick password matches the one previously remembered, the cover screen disappears and reveals the database. Otherwise, the database becomes completely closed and will require the full password again.\n\nNOTE: A timeout or the main-menu Lock button will activate the quick lock (if enabled). However, the bottom-left Lock button always applies the full lock.")
                multiline: true
                textFormat: TextFormat.Html
                textStyle.base: SystemDefaults.TextStyles.BodyText
            }
        }
    }
}

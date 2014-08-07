import bb.cascades 1.2

Page {
    onCreationCompleted: {
        app.restartWatchdog();
    }

    actions: [
        ActionItem {
            title: qsTr("Contact Developer") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_email.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                invokeSendEmail.trigger("bb.action.SENDEMAIL");
            }
            attachedObjects: [
                Invocation {
                    id: invokeSendEmail
                    query {
                        uri: "mailto:blackberry@popleteev.com?subject=KeePassB%202"
                        invokeActionId: "bb.action.SENDEMAIL"
                        invokeTargetId: "sys.pim.uib.email.hybridcomposer"
                    }
                }
            ]
        }
    ]
    Container {
        layout: DockLayout { }
        ScrollView {
            verticalAlignment: VerticalAlignment.Top
            scrollRole: ScrollRole.Main
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                leftPadding:   20
                rightPadding:  20
                topPadding:    50
                bottomPadding: 20
                ImageView {
                    imageSource: "asset:///images/cover.png"
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Label{
                    text: qsTr("KeePassB") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.BigText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Label{
                    text: qsTr("Version 2.0.2 beta") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.PrimaryText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Label{
                    text: qsTr("©2014 Andrei Popleteev") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.PrimaryText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Divider {
                    topMargin: 30
                    bottomMargin: 30
                }
                Label{
                    text: qsTr("Credits") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    horizontalAlignment: HorizontalAlignment.Left
                }
                Label{
                    text: qsTr("Thanks to Dominik Reichl for <a href='http://keepass.info'>KeePass</a>.\nThanks to Nam T. Nguyen for <a href='https://bitbucket.org/namn/browsepass/'>BrowsePass</a>, a nice JavaScript-based KeePass&nbsp;2 port.\n\rSome icons are based on free samples of the <a href='http://www.smashingmagazine.com/2013/02/02/freebie-user-interface-kit-icons/'>Linecons Icon Set</a> (free Smashing Magazine release) and <a href='http://getothercircles.com'>The OtherCircles</a> icon set by Luboš Volkov.") + Retranslate.onLocaleOrLanguageChanged
                    multiline: true
                    textFormat: TextFormat.Html
                    textStyle.base: SystemDefaults.TextStyles.BodyText
                    topMargin: 20
                    horizontalAlignment: HorizontalAlignment.Left
                }
                Label{
                    topMargin: 30
                    text: qsTr("Privacy Policy") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    horizontalAlignment: HorizontalAlignment.Left
                }
                Label{
                    text: qsTr("KeePassB does not collect any personal data nor share it with anyone.") + Retranslate.onLocaleOrLanguageChanged
                    multiline: true
                    textFormat: TextFormat.Plain
                    textStyle.base: SystemDefaults.TextStyles.BodyText
                    topMargin: 20
                    horizontalAlignment: HorizontalAlignment.Left
                }
            }
        }
    }
}

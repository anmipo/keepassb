import bb.cascades 1.2

Page {
    onCreationCompleted: {
        app.restartWatchdog();
    }

    actions: [
        ActionItem {
            title: qsTr("Contact Developer", "Menu item: action to write an email to the application developer") + Retranslate.onLocaleOrLanguageChanged
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
                    text: Application.applicationName
                    textStyle.base: SystemDefaults.TextStyles.BigText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Label{
                    text: qsTr("Version %1", "Application version in 'About' screen").arg(Application.applicationVersion) + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.PrimaryText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Label{
                    text: qsTr("\u00A92014 Andrei Popleteev", "Developer name, do not translate") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.PrimaryText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Divider {
                    topMargin: 30
                    bottomMargin: 30
                }
                Label{
                    text: qsTr("Credits", "Title of the help page with acknowledgements and licensing terms of used components.") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    horizontalAlignment: HorizontalAlignment.Left
                }
                Label{
                    text: qsTr("Thanks to Dominik Reichl for <a href='http://keepass.info'>KeePass</a>.\nThanks to Nam T. Nguyen for <a href='https://bitbucket.org/namn/browsepass/'>BrowsePass</a>, a nice JavaScript-based KeePass&nbsp;2 port.\n\rSome icons are based on free samples of the <a href='http://www.smashingmagazine.com/2013/02/02/freebie-user-interface-kit-icons/'>Linecons Icon Set</a> and <a href='http://getothercircles.com'>The OtherCircles</a> icon set by Lubo\u0161 Volkov.", "Content of the 'Credits' help section ") + Retranslate.onLocaleOrLanguageChanged
                    multiline: true
                    textFormat: TextFormat.Html
                    textStyle.base: SystemDefaults.TextStyles.BodyText
                    topMargin: 20
                    horizontalAlignment: HorizontalAlignment.Left
                }
                Label{
                    topMargin: 30
                    text: qsTr("Privacy Policy", "Title of a help page") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    horizontalAlignment: HorizontalAlignment.Left
                }
                Label{
                    text: qsTr("KeePassB does not collect any personal data nor share it with anyone.", "Content of the 'Privacy Policy' help page") + Retranslate.onLocaleOrLanguageChanged
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

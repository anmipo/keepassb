import bb.cascades 1.2

Page {
    onCreationCompleted: {
        app.restartWatchdog();
    }

    titleBar: TitleBar {
        title: qsTr("About") + Retranslate.onLocaleOrLanguageChanged
    }
    Container {
        layout: DockLayout { }
        ScrollView {
            verticalAlignment: VerticalAlignment.Center
            scrollRole: ScrollRole.Main
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                leftPadding:   20
                rightPadding:  20
                topPadding:    20
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
                    text: qsTr("Version 2.0 alpha") + Retranslate.onLocaleOrLanguageChanged // TODO update version
                    textStyle.base: SystemDefaults.TextStyles.PrimaryText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Label{
                    text: qsTr("©2014 Andrei Popleteev") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.PrimaryText
                    horizontalAlignment: HorizontalAlignment.Center
                }
                Label{
                    text: qsTr("Credits") + Retranslate.onLocaleOrLanguageChanged
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    topMargin: 50
                    horizontalAlignment: HorizontalAlignment.Left
                }
                Label{
                    text: qsTr("Thanks to Dominik Reichl, the author of KeePass.\n\rThanks to Nam T. Nguyen for <a href='https://bitbucket.org/namn/browsepass/'>BrowsePass</a>, which helped me to understand the KeePass2 format.\n\rSome icons are from the free samples from <a href='http://www.smashingmagazine.com/2013/02/02/freebie-user-interface-kit-icons/'>Linecons Icon Set</a> (Smashing Magazine release) and <a href='http://getothercircles.com'>GetOtherCircles.com</a> by Luboš Volkov.") + Retranslate.onLocaleOrLanguageChanged
                    multiline: true
                    textFormat: TextFormat.Html
                    textStyle.base: SystemDefaults.TextStyles.BodyText
                    topMargin: 20
                    horizontalAlignment: HorizontalAlignment.Left
                }
            }
        }
    }
}

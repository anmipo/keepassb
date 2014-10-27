/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.device 1.2
import bb.system 1.2
import org.keepassb 1.0

ScrollView {
    scrollRole: ScrollRole.Main
    Container {
        Container {
            layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
            ImageView {
                imageSource: "asset:///pwicons-dark/" + entry.iconId + ".png"
                verticalAlignment: VerticalAlignment.Center
            }
            Label {
                text: entry.title
                multiline: true
                textFormat: TextFormat.Plain
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
                textStyle.color: Color.White
                verticalAlignment: VerticalAlignment.Center
                layoutProperties: StackLayoutProperties { spaceQuota: 1 }
            }
        }
        LabelTextButton {
            id: usernameField
            labelText: qsTr("User Name", "Label of the username field; refers to login information rather then person's own name.") + Retranslate.onLocaleOrLanguageChanged
            valueText: entry.userName
        }
        LabelTextButton {
            id: passwordField
            labelText: qsTr("Password", "Label of the password field.") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_copy.png"
            valueText: entry.password
            passwordMasking: true
        }
        LabelTextButton {
            id: urlField
            labelText: qsTr("URL", "Label of the entry field containing a link/internet address.") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_copy.png"
            valueText: entry.url
            gestureHandlers: TapHandler {
                onTapped: {
                    Qt.openUrlExternally(urlField.valueText);
                }
            } 
        }
        LabelTextButton {
            id: notesText
            autoSize.maxLineCount: -1
            labelText: qsTr("Notes", "Label of the entry field containing comments or additional text information.") + Retranslate.onLocaleOrLanguageChanged
            valueText: entry.notes
        }
    }       
}
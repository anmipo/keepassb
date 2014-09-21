
import bb.cascades 1.2

/*
 * Length: 8 to 16
 * Uppser case, lower-case, digits, minus, underline, space, specials, brackets
 * look-alike
 */
Sheet {
    property string password
    function updatePassword() {
        
    }
    Page {
        onCreationCompleted: {
            updatePassword()
        }
        titleBar: TitleBar {
            title: qsTr("Password Generator", "Title of a page which helps the user to create random passwords")
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action")
                onTriggered: {
                    close();
                }
            }
            acceptAction: ActionItem {
                title: qsTr("OK", "A button/action")
                onTriggered: {
                    //TODO implement this
                }
            }
        }
        actions: [
            ActionItem {
                title: qsTr("Refresh", "A button/action which generates a new password")
                imageSource: "asset:///images/ic_refresh.png"
                ActionBar.placement: ActionBarPlacement.OnBar
            }
        ]
        Container {
            topPadding: 30
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10
            Label {
                id: passwordLabel
                bottomMargin: 30
                text: "fqwWRE123Lpoei%%dsfa"
                horizontalAlignment: HorizontalAlignment.Center
                verticalAlignment: VerticalAlignment.Center
                textStyle.base: SystemDefaults.TextStyles.TitleText
                textFit.mode: LabelTextFitMode.FitToBounds
                textStyle.fontWeight: FontWeight.Bold
            }
            Divider{}
            DropDown {
                id: presetDropDown
                title: qsTr("Preset", "Label for a selection of password presets/templates")
                selectedOption: presetDefault
                options: [
                    Option {
                        id: presetDefault
                        text: qsTr("Default", "One of the values of Preset selector, means using default setting for password generation. Will look like 'Preset    Default'")
                    },
                    Option {
                        id: presetCustom
                        text: qsTr("Custom", "One of the values of Preset selector. Custom here means user-defined. Will look like 'Preset    Custom'")
                    },
                    Option {
                        id: preset40Hex
                        text: qsTr("40-bit Hex", "One of the values of the Preset selector. Results in creation of hexadecimal number 40 binary bits long. Will look like 'Preset    40-bit Hex'.")
                    },
                    Option {
                        id: preset128Hex
                        text: qsTr("128-bit Hex", "One of the values of the Preset selector. Results in creation of hexadecimal number 128 binary bits long. Will look like 'Preset    128-bit Hex'.")
                    },
                    Option {
                        id: preset256Hex
                        text: qsTr("256-bit Hex", "One of the values of the Preset selector. Results in creation of hexadecimal number 256 binary bits long. Will look like 'Preset    256-bit Hex'.")
                    },
                    Option {
                        id: presetMacAddress
                        text: qsTr("Random MAC Address", "One of the values of the Preset selector. 'MAC Address' is a special technical term, see http://en.wikipedia.org/wiki/MAC_address. Will look like 'Preset    Random MAC Address'.")
                    }
                ]
            }
            Container {
                id: customPreset
                visible: presetDropDown.selectedOption == presetCustom
                Container {
                    bottomPadding: 10
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Button {
                        text: "abc"
                    }
                    Button {
                        text: "ABC"
                    }
                    Button {
                        text: "123"
                    }
                    Button {
                        text: "@%)"
                    }
                }
                Divider{}
                Label {
                    text: "Password Length: 20"
                }
                Slider {
                    fromValue: 5
                    toValue: 60
                    value: 20
                }
                Divider {
                    bottomMargin: 20
                }
                CheckBox {
                    text: qsTr("Exclude look-alikes (I,l,1,O,0)")
                }
            }
        }
    }
}
/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import org.keepassb 1.0

Sheet {
    id: pwGenSheet
    property string password: "password_template"
    // emitted when the user taps OK
    signal newPasswordReady(string pwd)
    
    function updatePassword() {
        var preset = presetDropDown.selectedOption;
        var pwGen = app.getPasswordGenerator();
        var newPassword;
        if (preset == presetCustom) {
            newPassword = pwGen.makeCustomPassword(appSettings.pwGenLength, appSettings.pwGenFlags);
        } else if (preset == preset40Hex) {
            newPassword = pwGen.makeHexPassword(40 / 8);
        } else if (preset == preset128Hex) {
            newPassword = pwGen.makeHexPassword(128 / 8);
        } else if (preset == preset256Hex) {
            newPassword = pwGen.makeHexPassword(256 / 8);
        } else if (preset == presetMacAddress) {
            newPassword = pwGen.makeMacAddress();
        } else {
            newPassword = pwGen.makeCustomPassword(20, // password length
                PasswordGenerator.PWGEN_INCLUDE_LOWER | PasswordGenerator.PWGEN_INCLUDE_UPPER | 
                PasswordGenerator.PWGEN_INCLUDE_DIGITS | PasswordGenerator.PWGEN_EXCLUDE_SIMILAR);
        }
        password = newPassword;
        
        app.restartWatchdog();
    }
    
    onCreationCompleted: {
        // close without saving when DB is being locked
        database.dbLocked.connect(_close);
        updatePassword();
    }
    
    onClosed: {
        database.dbLocked.disconnect(_close);
        destroy();
    }
    
    function _close() {
        close();
    }
    
    PageWithWatchdog {
        titleBar: TitleBar {
            title: qsTr("Password Generator", "Title of a page which helps the user to create random passwords")
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action")
                onTriggered: {
                    app.restartWatchdog();
                    close();
                }
            }
            acceptAction: ActionItem {
                title: qsTr("OK", "A button/action to accept the proposed password and insert it into the entry")
                onTriggered: {
                    newPasswordReady(password);
                    app.restartWatchdog();
                    close();
                }
            }
        }
        actions: [
            ActionItem {
                title: qsTr("Refresh", "A button/action which generates a new password")
                imageSource: "asset:///images/ic_refresh.png"
                ActionBar.placement: ActionBarPlacement.OnBar
                onTriggered: updatePassword()
            }
        ]
        Container {
            topPadding: 30
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10
            Label {
                id: passwordLabel
                text: password
                multiline: true
                bottomMargin: 30
                textStyle.textAlign: TextAlign.Center
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
                textStyle.fontFamily: "\"DejaVu Sans Mono\", Monospace"
                textFormat: TextFormat.Plain
            }
            Divider{}
            DropDown {
                id: presetDropDown
                title: qsTr("Preset", "Label for a selection of password presets/templates")
                options: [
                    Option {
                        id: presetDefault
                        text: qsTr("Default", "One of the values of Preset selector. Generates password with default/standard settings. Will look like 'Preset    Default'.")
                        value: PasswordGenerator.PWGEN_PRESET_DEFAULT
                        selected: (appSettings.pwGenPreset == PasswordGenerator.PWGEN_PRESET_DEFAULT)
                    },
                    Option {
                        id: presetCustom
                        text: qsTr("Custom", "One of the values of Preset selector. Custom here means user-defined. Will look like 'Preset    Custom'")
                        value: PasswordGenerator.PWGEN_PRESET_CUSTOM
                        selected: (appSettings.pwGenPreset == PasswordGenerator.PWGEN_PRESET_CUSTOM)
                    },
                    Option {
                        id: preset40Hex
                        text: qsTr("40-bit Hex", "One of the values of the Preset selector. Results in creation of hexadecimal number 40 binary bits long. Will look like 'Preset    40-bit Hex'.")
                        value: PasswordGenerator.PWGEN_PRESET_HEX40
                        selected: (appSettings.pwGenPreset == PasswordGenerator.PWGEN_PRESET_HEX40)
                    },
                    Option {
                        id: preset128Hex
                        text: qsTr("128-bit Hex", "One of the values of the Preset selector. Results in creation of hexadecimal number 128 binary bits long. Will look like 'Preset    128-bit Hex'.")
                        value: PasswordGenerator.PWGEN_PRESET_HEX128
                        selected: (appSettings.pwGenPreset == PasswordGenerator.PWGEN_PRESET_HEX128)
                    },
                    Option {
                        id: preset256Hex
                        text: qsTr("256-bit Hex", "One of the values of the Preset selector. Results in creation of hexadecimal number 256 binary bits long. Will look like 'Preset    256-bit Hex'.")
                        value: PasswordGenerator.PWGEN_PRESET_HEX256
                        selected: (appSettings.pwGenPreset == PasswordGenerator.PWGEN_PRESET_HEX256)
                    },
                    Option {
                        id: presetMacAddress
                        text: qsTr("Random MAC Address", "One of the values of the Preset selector. 'MAC Address' is a special technical term, see http://en.wikipedia.org/wiki/MAC_address. Will look like 'Preset    Random MAC Address'.")
                        value: PasswordGenerator.PWGEN_PRESET_MAC_ADDRESS
                        selected: (appSettings.pwGenPreset == PasswordGenerator.PWGEN_PRESET_MAC_ADDRESS)
                    }
                ]
                onSelectedOptionChanged: {
                    if (selectedOption) {
                        appSettings.pwGenPreset = selectedOption.value;
                    }
                    updatePassword();
                }
            }
            ScrollView {
                scrollRole: ScrollRole.Main
                Container {
                    id: customPreset
                    visible: presetDropDown.selectedOption == presetCustom
                    topPadding: 0
                    Label {
                        text: qsTr("Password Length: <b>%1</b>", "Text showing current password length. %1 will be replaced by a number")
                                    .arg(Math.round(passwordLength.immediateValue))
                        textFormat: TextFormat.Html
                    }
                    Slider {
                        id: passwordLength
                        fromValue: 8.0
                        toValue: 50.0
                        value: appSettings.pwGenLength
                        topMargin: 20
                        bottomMargin: 20
                        onValueChanged: {
                            appSettings.pwGenLength = Math.round(value);
                            updatePassword();
                        }
                    }
                    Header {
                        title: qsTr("Include Characters", "Title of a group with on/off options which define which characters CAN be used in generated passwords.")
                    }
                    CheckBox {
                        id: includeLowerCase
                        text: qsTr("Lower Case Letters (abc...)", "An option/checkbox which can be On or Off. Do not translate the brackets content.")
                        checked: appSettings.pwGenFlags & PasswordGenerator.PWGEN_INCLUDE_LOWER
                        onCheckedChanged: {
                            if (checked)
                                appSettings.pwGenFlags |= PasswordGenerator.PWGEN_INCLUDE_LOWER;
                            else
                                appSettings.pwGenFlags &= ~PasswordGenerator.PWGEN_INCLUDE_LOWER;
                            updatePassword();
                        }
                    }
                    CheckBox {
                        id: includeUpperCase
                        text: qsTr("Upper Case Letters (ABC...)", "An option/checkbox which can be On or Off. Do not translate the brackets content.")
                        checked: appSettings.pwGenFlags & PasswordGenerator.PWGEN_INCLUDE_UPPER
                        onCheckedChanged: {
                            if (checked)
                                appSettings.pwGenFlags |= PasswordGenerator.PWGEN_INCLUDE_UPPER;
                            else
                                appSettings.pwGenFlags &= ~PasswordGenerator.PWGEN_INCLUDE_UPPER;
                            updatePassword()
                       }
                    }
                    CheckBox {
                        id: includeSpecials
                        text: qsTr("Special Symbols (@#$...)", "An option/checkbox which can be On or Off. Do not translate the brackets content.")
                        checked: appSettings.pwGenFlags & PasswordGenerator.PWGEN_INCLUDE_SPECIALS
                        onCheckedChanged: {
                            if (checked)
                                appSettings.pwGenFlags |= PasswordGenerator.PWGEN_INCLUDE_SPECIALS;
                            else
                                appSettings.pwGenFlags &= ~PasswordGenerator.PWGEN_INCLUDE_SPECIALS;
                            updatePassword();
                        }
                    }
                    CheckBox {
                        id: includeDigits
                        text: qsTr("Digits (123...)", "An option/checkbox which can be On or Off. Do not translate the brackets content.")
                        checked: appSettings.pwGenFlags & PasswordGenerator.PWGEN_INCLUDE_DIGITS
                        onCheckedChanged: {
                            if (checked)
                                appSettings.pwGenFlags |= PasswordGenerator.PWGEN_INCLUDE_DIGITS;
                            else
                                appSettings.pwGenFlags &= ~PasswordGenerator.PWGEN_INCLUDE_DIGITS;
                            updatePassword();
                        }
                    }
                    Header {
                        title: qsTr("Exclude Characters", "Title of a group with on/off options which define which characters CANNOT be used in generated passwords.")
                    }
                    CheckBox {
                        id: excludeSimilar
                        text: qsTr("Similar Characters (I,l,1,O,0)", "An option/checkbox which excludes visually similar symbols from the generated password. Symbols inside the brackets should not be translated.")
                        checked: appSettings.pwGenFlags & PasswordGenerator.PWGEN_EXCLUDE_SIMILAR;
                        onCheckedChanged: {
                            if (checked)
                                appSettings.pwGenFlags |= PasswordGenerator.PWGEN_EXCLUDE_SIMILAR;
                            else
                                appSettings.pwGenFlags &= ~PasswordGenerator.PWGEN_EXCLUDE_SIMILAR;
                            updatePassword();
                        }
                    }
                }
            }
        }
    }
}
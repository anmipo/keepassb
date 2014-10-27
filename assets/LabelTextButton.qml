import bb.cascades 1.2

Container {
    property alias imageSource: button.imageSource
    property alias labelText: label.title
    property string valueText
    property alias inputMode: edit.inputMode
    // sets visibility of the "show/hide password" switch  
    property bool passwordMasking: false
    property alias autoSize: edit.autoSize
    
    leftMargin: 10
    topMargin: 10
    rightMargin: 10
    bottomMargin: 10

    Header {
        id: label
        title: "Label title"
        bottomMargin: 10
    }
    Container {
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        ReadOnlyTextField {
            id: edit
            text: (showPasswordCheck.checked ? valueText : "********")
            autoSize.maxLineCount: 5
            textStyle.fontFamily: "\"DejaVu Sans Mono\", Monospace"
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Center
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }
            input.flags: TextInputFlag.SpellCheckOff
            focusPolicy: FocusPolicy.None
            onTouch: {
                if ((event.touchType == TouchType.Up) && passwordMasking) {
                    showPasswordCheck.checked = !showPasswordCheck.checked; 
                }
            }
        }
        ImageToggleButton {
            id: showPasswordCheck
            property int padding: 30
            visible: passwordMasking
            imageSourceDefault: "asset:///images/password_hidden.png"
            imageSourceChecked: "asset:///images/password_visible.png"
            checked: !passwordMasking
            verticalAlignment: VerticalAlignment.Center
            rightMargin: padding
        }
        Button {
            id: button
            imageSource: "asset:///images/ic_copy.png"
            preferredWidth: 50
            verticalAlignment: VerticalAlignment.Top
            horizontalAlignment: HorizontalAlignment.Right
            enabled: (valueText.length > 0)
            onClicked: {
                Qt.app.copyWithTimeout(valueText);
            }
        }
    }
    Divider {} 
}

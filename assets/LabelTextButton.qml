import bb.cascades 1.2

Container {
    property alias imageSource: button.imageSource
    property alias labelText: label.title
    property string valueText: "hello1lIi"
    property alias inputMode: edit.inputMode
    // sets visibility of the "show/hide password" switch  
    property bool passwordMasking: false
    property string passwordMaskText: "********"
    
    leftMargin: 10
    topMargin: 10
    rightMargin: 10
    bottomMargin: 10

    onValueTextChanged: {
        if (passwordMasking)
            setEditMasked(showPasswordCheck.checked);
    }
    onCreationCompleted: {
        setEditMasked(passwordMasking);
    }
    function setEditMasked(masked) {
        edit.text = (masked ? passwordMaskText : valueText);        
    }
    
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
            text: "Edit text"
            autoSize.maxLineCount: 5
            textStyle.fontFamily: "\"DejaVu Sans Mono\", Monospace"
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Center
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }
            input.flags: TextInputFlag.SpellCheckOff
            focusPolicy: FocusPolicy.None
        }
        ImageToggleButton {
            id: showPasswordCheck
            property int padding: 30
            visible: passwordMasking
            imageSourceDefault: "asset:///images/password_hidden.png"
            imageSourceChecked: "asset:///images/password_visible.png"
            checked: false
            verticalAlignment: VerticalAlignment.Center
            rightMargin: padding
            onCheckedChanged: setEditMasked(!checked)
        }
        Button {
            id: button
            accessibility.name: qsTr("Copy") + Retranslate.onLocaleOrLanguageChanged 
            imageSource: "asset:///images/ic_copy.png"
            preferredWidth: 50
            verticalAlignment: VerticalAlignment.Top
            horizontalAlignment: HorizontalAlignment.Right
            onClicked: {
                Qt.app.copyWithTimeout(edit.text);
            }
        }
    }
    Divider {}
    attachedObjects: [
        ComponentDefinition {
            source: "asset:///ReadOnlyTextField.qml"
        }
    ]    
}

import bb.cascades 1.2

Container {
    property alias imageSource: button.imageSource
    property alias labelText: label.title
    property alias valueText: edit.text
    property alias inputMode: edit.inputMode
    
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
            text: "Edit text"
            autoSize.maxLineCount: 5
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Center
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }
            input.flags: TextInputFlag.SpellCheckOff
            focusPolicy: FocusPolicy.None
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

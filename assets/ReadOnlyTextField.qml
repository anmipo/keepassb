import bb.cascades 1.2

TextArea {
    editable: false
    autoSize.maxLineCount: 1
    input.flags: TextInputFlag.SpellCheckOff
    inputMode: TextAreaInputMode.Text
    textFormat: TextFormat.Plain
}

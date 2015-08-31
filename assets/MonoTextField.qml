import bb.cascades 1.2

TextField {
    /** Trim text when this control looses focus */
    property bool trimOnBlur: false
    
    input.flags: TextInputFlag.SpellCheckOff
    textStyle.fontFamily: "\"DejaVu Sans Mono\", Monospace"
    textFormat: TextFormat.Plain
    onTextChanging: {
        app.restartWatchdog();
    }
    onFocusedChanged: {
        if (!focused && trimOnBlur) {
            text = text.trim();
        }
    }
}

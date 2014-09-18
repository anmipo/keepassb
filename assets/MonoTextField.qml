import bb.cascades 1.2

TextField {
    input.flags: TextInputFlag.SpellCheckOff
    textStyle.fontFamily: "\"DejaVu Sans Mono\", Monospace"
    textFormat: TextFormat.Plain
    onTextChanging: {
        app.restartWatchdog();
    }
}

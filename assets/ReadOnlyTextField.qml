/*
 * Copyright (c) 2014-2016 Andrei Popleteev. 
 * Licensed under the MIT license.
 */

import bb.cascades 1.2

TextArea {
    editable: false
    autoSize.maxLineCount: 1
    input.flags: TextInputFlag.SpellCheckOff
    inputMode: TextAreaInputMode.Text
    textFormat: TextFormat.Plain
}

/*
 * Copyright (c) 2014-2016 Andrei Popleteev. 
 * Licensed under the MIT license.
 */

import bb.cascades 1.2

Container {
    function getLockStateImage() {
        if (database.locked)
            return "asset:///images/cover-locked.png";
        else if (app.quickLocked) 
            return "asset:///images/cover-quick-locked.png";
        else 
            return "asset:///images/cover-unlocked.png";
    } 

    layout: DockLayout { }
    background: Color.create("#262626") //TODO get color from the theme
    ImageView {
        imageSource: getLockStateImage()
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Center
    }
}

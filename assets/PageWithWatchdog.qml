/*
 * Copyright (c) 2014-2016 Andrei Popleteev. 
 * Licensed under the MIT license.
 */

import bb.cascades 1.2

Page {
    default property alias contentWithWatchdog: container.controls  
    Container {
        id: container
        layout: DockLayout { }
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Fill
        onTouchCapture: {
            app.restartWatchdog();
        }
        // page's contentWithWatchdog goes here
    }
}

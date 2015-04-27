import bb.cascades 1.2

Page {
    default property alias contentWithWatchdog: container.controls  
    Container {
        id: container
        onTouchCapture: {
            app.restartWatchdog();
        }
        // page's contentWithWatchdog goes here
    }
}

import bb.cascades 1.2

Container {
    property bool locked: true
    
    layout: DockLayout { }
    background: Color.create("#262626") //TODO get color from the theme
    ImageView {
        imageSource: database.locked ? "asset:///images/cover-locked.png" : "asset:///images/cover-unlocked.png"  
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Center
    }
}

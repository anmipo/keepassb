import bb.cascades 1.2

Container {
    layout: DockLayout { }
    background: Color.create("#262626") //TODO get color from the theme
    ImageView {
        imageSource: "asset:///images/cover.png"
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Center
    }
}

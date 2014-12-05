import bb.cascades 1.2

CustomListItem {
    property alias title: titleLabel.text
    property alias imageSource: iconView.imageSource
    property alias counterText: counterLabel.text
    property alias description: descriptionLabel.text
    property string itemType: "entry" // either "group" or "entry"
    
    highlightAppearance: HighlightAppearance.Default
    dividerVisible: true
    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        verticalAlignment: VerticalAlignment.Fill
        leftPadding: 15
        rightPadding: 15
        ImageView {
            id: iconView
            imageSource: "asset:///images/ic_expired_item.png"
            minWidth: 80
            minHeight: 80
            rightMargin: 20
            horizontalAlignment: HorizontalAlignment.Left
            verticalAlignment: VerticalAlignment.Center
        }
        Container {
            layout: StackLayout { orientation: LayoutOrientation.TopToBottom }
            layoutProperties: StackLayoutProperties { spaceQuota: 1 } 
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Fill
            Label {
                id: titleLabel
                text: "Title"
                textFormat: TextFormat.Plain
                textStyle.base: SystemDefaults.TextStyles.PrimaryText
                textStyle.fontWeight: (itemType == "group") ? FontWeight.Bold : FontWeight.Default
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Center
                bottomMargin: 0
            }
            Label {
                id: descriptionLabel
                text: "Details"
                visible: (itemType == "entry")
                textStyle.fontStyle: FontStyle.Italic
                textStyle.color: Color.DarkGray
                topMargin: 0
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Top
            }
        }
        Label {
            id: counterLabel
            text: "(0)"
            visible: (itemType == "group")
            textStyle.color: Color.DarkGray
            textStyle.base: SystemDefaults.TextStyles.BodyText
            horizontalAlignment: HorizontalAlignment.Right
            verticalAlignment: VerticalAlignment.Center 
        }
    }
}
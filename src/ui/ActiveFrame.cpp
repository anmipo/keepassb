/*
 * ActiveFrame.cpp
 *
 *  Created on: 21 Jun 2014
 *      Author: Andrei
 */

#include "ui/ActiveFrame.h"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Container>

using namespace bb::cascades;

ActiveFrame::ActiveFrame() : SceneCover(this) {

    QmlDocument* qml = QmlDocument::create("asset:///ActiveFrameCover.qml").parent(this);
    Container* mainContainer = qml->createRootObject<Container>();
    setContent(mainContainer);

    // Initiate the update when the app moves to the background.
    QObject::connect(Application::instance(), SIGNAL(thumbnail()), this, SLOT(update()));
}

ActiveFrame::~ActiveFrame() {
    // nothing to do here
}


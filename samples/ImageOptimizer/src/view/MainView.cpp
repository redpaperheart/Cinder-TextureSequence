//
//  MainView.cpp
//
//  Created by Daniel Scheibel on 2/9/15.
//

#include "MainView.h"

MainView::MainView(){}

void MainView::setup()
{
    m = Model::getInstance();
    
    m->signal_nextStateChanged.connect( std::bind(&MainView::onNextStateChange, this) );
    
}

void MainView::resize()
{
 
}

void MainView::onNextStateChange()
{
    Model::State nextState = m->getNextState();
    
    if(nextState == Model::STATE_NONE) return;
    ci::app::console() << "MainView::onNextStateChange " << Model::stateToString(m->getNextState()) << std::endl;
    
    switch (nextState) {
        case Model::STATE_NONE:
            onStateChangeComplete();
            break;
        case Model::STATE_GLOBAL:
            onStateChangeComplete();
            break;
        case Model::STATE_REGIONAL:
            onStateChangeComplete();
            break;
        case Model::STATE_LOCAL:
            onStateChangeComplete();
            break;
        default:
            break;
    }
}

void MainView::onStateChangeComplete()
{
    ci::app::console() << "MainView::onStateChangeComplete " << std::endl;
    signal_stateChangeComplete.emit();
}

void MainView::update()
{

}

void MainView::draw()
{
    
}


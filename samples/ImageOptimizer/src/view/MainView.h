//
//  MainView.h
//
//  Created by Daniel Scheibel on 2/9/15.
//

#pragma once

#include "Model.h"


using namespace ci;

class MainView
{
  public:
    MainView();
    void setup();
    void update();
    void draw();
    void resize();
    
    void onNextStateChange();
    void onStateChangeComplete();
    
//    void showParams(bool visible)   { mScene.showParams(visible); }
    signals::Signal<void()> signal_stateChangeComplete;
    
  protected:
    Model           *m;
    
};
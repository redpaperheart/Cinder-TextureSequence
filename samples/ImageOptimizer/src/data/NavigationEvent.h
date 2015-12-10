//
//  NavigationEvent.h
//
//  Created by Daniel Scheibel on 2/9/15.
//

#pragma once

#include "Model.h"

class NavigationEvent {
  public:
    NavigationEvent(){};
    NavigationEvent( Model::State s ):state(s){};
    ~NavigationEvent(){};
    
    Model::State state;
    
  protected:
    std::string mId;
};

//
//  UIView.h
//
//  Created by Daniel Scheibel on 2/9/15.
//

#pragma once

#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"

#include "Model.h"

#include "rph/statehandler/StateInOut.h"
#include "rph/DisplayObjectContainer.h"

using namespace ci;

class UIView : public rph::StateInOut, public rph::DisplayObjectContainer {
  public:
    UIView();
    virtual void setup(std::string id);
    virtual void update();
    virtual void draw();
    virtual void drawHelpers();
    
    void setDebugTextPos(ivec2 pos) { mDebugTextPos = pos; }
    
protected:
    Model *m;
    std::string mId;
    gl::TextureRef              mCurTexRef;
    ivec2                       mDebugTextPos;
    static gl::TextureFontRef   debugTF;
    
};

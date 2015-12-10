//
// UIView_Nav.h
//
//  Created by Daniel Scheibel on 3/3/15.
//

#pragma once

#include "UIView.h"
#include "cinder/Timeline.h"

using namespace ci;

class UIView_NavItem : public UIView{
  public:
    UIView_NavItem();
    
    virtual void setup( std::string aId, std::string tex, vec2 pos = vec2(0) );
    virtual void update() override;
    virtual void draw() override;
    
protected:
    virtual void _animateIn() override;
    virtual void _animateOut() override;
    
    void drawCircle(float radius, float rotationOffset = 0.0f);
    
    bool mIsRotating = false;
    Anim<float> mInnerRadius = 14;
    Anim<float> mOuterRadius = 26;
    
    static gl::BatchRef mDotBatch;
    static gl::BatchRef mCircleBatch;

};

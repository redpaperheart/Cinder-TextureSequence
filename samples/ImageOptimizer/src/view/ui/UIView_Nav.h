//
// UIView_Nav.h
//
//  Created by Daniel Scheibel on 3/3/15.
//

#pragma once

#include "UIView.h"
#include "UIView_NavItem.h"

using namespace ci;

class UIView_Nav : public UIView{
  public:
    UIView_Nav();
    
    virtual void setup(std::string id) override;
    virtual void update() override;
    virtual void draw() override;
    
    void changeState(Model::State state);
    
protected:
    virtual void _animateIn() override;
    virtual void _animateOut() override;
    
    Model::State mCurState;
    
    UIView_NavItem mLocalNavItem;
    UIView_NavItem mRegionalNavItem;
    UIView_NavItem mGlobalNavItem;
    
    ColorA mLineColor;
};

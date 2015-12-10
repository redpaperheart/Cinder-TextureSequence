//
// UIView_Nav.h
//
//  Created by Daniel Scheibel on 3/3/15.
//

#include "UIView_Nav.h"

UIView_Nav::UIView_Nav() : UIView()
{
    
}

void UIView_Nav::setup( std::string id )
{
    UIView::setup( id );
    
    mLineColor = ColorA::hex(0xc1ae87);
    
    mGlobalNavItem.setup("global", "img/nav/global_on.png");
    mRegionalNavItem.setup("regional", "img/nav/regional_on.png");
    mLocalNavItem.setup("local", "img/nav/local_on.png");
    
    mGlobalNavItem.animateIn();
    mRegionalNavItem.animateOut(true);
    mLocalNavItem.animateOut(true);
}

void UIView_Nav::changeState(Model::State state){
    if(mCurState == state)return;
    
    switch (state) {
        case Model::STATE_GLOBAL:
            mGlobalNavItem.animateIn();
            mRegionalNavItem.animateOut();
            mLocalNavItem.animateOut();
            break;
        case Model::STATE_REGIONAL:
            mGlobalNavItem.animateOut();
            mRegionalNavItem.animateIn();
            mLocalNavItem.animateOut();
            break;
        case Model::STATE_LOCAL:
            mGlobalNavItem.animateOut();
            mRegionalNavItem.animateOut();
            mLocalNavItem.animateIn();
            break;
        default:
            break;
    }
    mCurState = state;
}

void UIView_Nav::_animateIn()
{
    _onAnimateIn();
}

void UIView_Nav::_animateOut()
{
    _onAnimateOut();
}

void UIView_Nav::update()
{
}

void UIView_Nav::draw()
{
    if( m->bDrawHelpers ) drawHelpers();
    
    gl::ScopedDepth depth(false);
    gl::ScopedBlendAlpha blend;
    
    // align on line center
    gl::ScopedMatrices navMat;
    gl::translate(WIDTH - m->mNavBarMargin.x, HEIGHT / 2.0f + m->mNavBarMargin.y);
    
    // draw lines
    gl::ScopedColor color(mLineColor);
    gl::drawLine(vec2(0, -m->mNavBarLineToDot), vec2(0, m->mNavBarLineToDot - m->mNavBarHalfHeight));
    gl::drawLine(vec2(0, m->mNavBarLineToDot), vec2(0, -m->mNavBarLineToDot + m->mNavBarHalfHeight));

    // draw animated nav items
    mRegionalNavItem.draw();
    
    gl::translate(0, m->mNavBarHalfHeight);
    mLocalNavItem.draw();
    
    gl::translate(0, -m->mNavBarHeight);
    mGlobalNavItem.draw();
  }

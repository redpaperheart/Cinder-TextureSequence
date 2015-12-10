//
//  UIView.h
//
//  Created by Daniel Scheibel on 2/9/15.
//

#include "UIView.h"

gl::TextureFontRef UIView::debugTF;

UIView::UIView()
{
    if (!debugTF) debugTF = ci::gl::TextureFont::create( ci::Font( "Arial", 12 ) );
    mDebugTextPos = ci::vec2(0,0);
}

void UIView::setup( std::string id )
{
    mId = id;
    m = Model::getInstance();
}

void UIView::update()
{
    if( mCurState == STATE_OUT ) return;
}

void UIView::draw()
{
    if( m->bDrawHelpers) drawHelpers();
    if( mCurState == STATE_OUT ) return;
    if( mCurTexRef ) ci::gl::draw(mCurTexRef);
}

void UIView::drawHelpers()
{
    gl::ScopedMatrices mat;
    gl::color( mCurState == STATE_OUT ? Color::gray(0.5) : Color::white());
    debugTF->drawString(mId + " " + stateToString( mCurState ), mDebugTextPos);
}
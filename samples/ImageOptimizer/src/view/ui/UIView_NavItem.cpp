//
// UIView_Nav.h
//
//  Created by Daniel Scheibel on 3/3/15.
//

#include "UIView_NavItem.h"

gl::BatchRef UIView_NavItem::mDotBatch = nullptr;
gl::BatchRef UIView_NavItem::mCircleBatch = nullptr;

UIView_NavItem::UIView_NavItem() : UIView()
{
    // off color:   a89a71
    // on color:    fef4d7
}

void UIView_NavItem::setup( std::string aId, std::string tex, vec2 pos )
{
    UIView::setup( aId );
    
    mColorA = ColorA::hexA(0xfef4d7ff);
   
    mCurTexRef = gl::Texture::create(loadImage(app::loadAsset( tex )), gl::Texture::Format().mipmap());
    mOffset = vec2(-mCurTexRef->getWidth(), -mCurTexRef->getHeight()/2.0f );
    mScale = vec2(1);
    mPos = pos;
    
    if (!mDotBatch) {
        mDotBatch = gl::Batch::create(geom::Circle().radius(3),
                                      gl::getStockShader(gl::ShaderDef().color()));
    }
    if (!mCircleBatch) {
        mCircleBatch = gl::Batch::create(geom::WireCircle().subdivisions(24).radius(1),
                                         gl::getStockShader(gl::ShaderDef().color()));
    }
}

void UIView_NavItem::_animateIn()
{
    app::timeline().apply(&mColorA, ColorA::hexA(0xfffef4d7), 0.7f, EaseOutQuad());
    app::timeline().apply(&mScale, vec2(1.0f), 0.7f, EaseOutQuad());
    app::timeline().apply(&mInnerRadius, 0.0f, 0.3f, EaseOutQuad());
    app::timeline().apply(&mOuterRadius, 0.0f, 0.3f, EaseOutQuad()).finishFn([this] {
        mIsRotating = true;
    });
    app::timeline().appendTo(&mInnerRadius, 14.0f, 1.0f, EaseOutQuad() );
    app::timeline().appendTo(&mOuterRadius, 26.0f, 1.0f, EaseOutQuad() );
    _onAnimateIn();
}

void UIView_NavItem::_animateOut()
{
    app::timeline().apply(&mColorA, ColorA::hexA(0x99a89a71 ), 0.7f, EaseOutQuad());
//    app::timeline().apply(&mColorA, ColorA(1, 1, 1, 0.5), 0.7f, EaseOutQuad());
    app::timeline().apply(&mScale, vec2(0.6f), 0.7f, EaseOutQuad());
    app::timeline().apply(&mInnerRadius, 0.0f, 0.5f, EaseOutQuad());
    app::timeline().apply(&mOuterRadius, 0.0f, 0.7f, EaseOutQuad()).finishFn([this] {
        mIsRotating = false;
    });;
    app::timeline().appendTo(&mOuterRadius, 14.0f, 0.5f, EaseOutQuad());
    _onAnimateOut();
}

void UIView_NavItem::update()
{
}


void UIView_NavItem::draw()
{
    if( m->bDrawHelpers ) drawHelpers();
    
    // draw central dot
    //gl::ScopedColor dotCol(ColorA::hex(0xfff5d6));
    gl::ScopedColor dotCol( mColorA );
    mDotBatch->draw();
    
    // draw rotating circles
    //gl::ScopedColor col(ColorA::hex(0xd2bf87));
    gl::ScopedColor col( mColorA );
    drawCircle(mInnerRadius);
    drawCircle(mOuterRadius, M_PI_2);
    
    // draw text label
    gl::ScopedBlendAdditive blend;
    gl::ScopedColor textcolor(mColorA);
    gl::ScopedMatrices navMat;
    gl::translate( vec2(-mOuterRadius - 20, 0) );
    gl::scale(mScale);
    gl::translate(mOffset);
    gl::draw(mCurTexRef);
}

void UIView_NavItem::drawCircle( float radius, float rotationOffset)
{
    gl::ScopedMatrices m;
    gl::scale(vec2(radius));
    
    if (mIsRotating) {
        gl::rotate(app::getElapsedSeconds() * 0.5 + rotationOffset, 0, 1, 0);
    }
    
//    gl::lineWidth( 100.0f );
//    ci::gl::drawStrokedCircle( vec2(0), radius );
    
    mCircleBatch->draw();
}


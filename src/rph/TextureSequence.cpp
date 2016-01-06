/*
 Copyright (c) 2015 Red Paper Heart Inc.
 
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "TextureSequence.h"

namespace rph {

    TextureSequence::TextureSequence() {}

    TextureSequence::~TextureSequence()
    {
        mTextureRefs.clear();
        mTexRefOffsets.clear();
    }
    
    void TextureSequence::setup( const std::vector<ci::gl::TextureRef> &textureRefs, const float &fps )
    {
        mTextureRefs.clear();
        mTextureRefs = textureRefs;
        mNumFrames = mTextureRefs.size();
        setFramerate( fps );
    }
    
    void TextureSequence::setup(const std::vector<ci::gl::TextureRef> &textureRefs, const ci::JsonTree &json, const float &fps)
    {
        mTextureRefs.clear();
        mTextureRefs = textureRefs;
        mNumFrames = mTextureRefs.size();
        setFramerate( fps );
        setupMetaInfo( json);
        
    }
    
    void TextureSequence::setupMetaInfo( const ci::JsonTree &json ){
        mTexRefOffsets.clear();
        // loop through the json sequence and fill the mOffsets vector
        for( auto itr = json["sequence"].begin(); itr != json["sequence"].end(); itr++){
            ci::ivec2 tempVec;
            tempVec.x = (*itr)["x"].getValue<int>();
            tempVec.y = (*itr)["y"].getValue<int>();
            mTexRefOffsets.push_back(tempVec);
        }
        
        // test if num of Images matches Number of Images Info in the json file
        if(mTextureRefs.size() != mTexRefOffsets.size()){
            ci::app::console() << "WARNING: Number of images and json info does not match." << std::endl;
        }
        
    }
    
    
    void TextureSequence::setFramerate( float fps )
    {
        mFps = fps;
        mTimePerFrame = 1.0/mFps;
    }
    
    /**
     *  -- Begins playback of sequence
     */
    void TextureSequence::play( bool reverse )
    {
        if(mTextureRefs.size() > 0){
            ci::app::console() << "play"<< std::endl;
            mPlayReverse = reverse;
            mPlaying = true;
            mComplete = false;
            mStartTime = ci::app::getElapsedSeconds();
            mStartFrame = mPlayheadPosition;
        }
    }

    /**
     *  -- Stops/Pauses playback
     */
    void TextureSequence::stop()
    {
        mPlaying = false;
        ci::app::console() << "stop"<< std::endl;
    }

    /**
     *  -- Stops/Pauses playback
     */
    void TextureSequence::togglePlayback()
    {
        if (isPlaying()) stop();
        else play();
    }
    
    /**
     *  -- Stops playback and resets the playhead to zero
     */
    void TextureSequence::reset()
    {
        mPlayheadPosition = 0;
        mPlaying = false;
    }

    /**
     *  -- Seek to a new position in the sequence
     */
    void TextureSequence::setPlayheadPosition( int newPosition )
    {
        mPlayheadPosition = ci::math<int>::max( 0, ci::math<int>::min( newPosition, mNumFrames - 1 ) );
    }
    
    /**
     *  -- Seek to a new position in the sequence
     */
    void TextureSequence::setPlayheadPositionByPerc( float perc )
    {
        perc = ci::math<float>::max( 0.0f, ci::math<float>::min( perc, 1.0f ) );
        setPlayheadPosition( perc * (mNumFrames - 1) );
    }

    /**
     *  -- TODO
     */
    void TextureSequence::step( int frameInc ) {}
    
    /**
     *  -- Call on each frame to update the playhead, this method jumps frames if current framerate is lower fps of sequence
     */
    void TextureSequence::update()
    {
        if( mPlaying ){
            int newPosition = mStartFrame + (mPlayReverse?-1:1) * ((ci::app::getElapsedSeconds() - mStartTime)/mTimePerFrame);
            update( newPosition );
        }
    }
    
    /**
     *  -- 
     */
    void TextureSequence::update( int newPosition )
    {
        if( newPosition > mNumFrames - 1 ){
            if( mLooping ){
                mComplete = false;
                mPlayheadPosition = newPosition % mNumFrames;
            } else {
                mPlayheadPosition = mNumFrames - 1;
                mComplete = true;
                stop();
            }
            
        } else if( newPosition < 0 ) {
            if( mLooping ){
                mComplete = false;
                mPlayheadPosition = (mNumFrames + (newPosition % mNumFrames))%mNumFrames ;
                
            } else {
                mPlayheadPosition = 0;
                mComplete = true;
                stop();
            }
        } else {
            mComplete = false;
            mPlayheadPosition = newPosition;
        }
    }
    
    ci::ivec2 const TextureSequence::getCurrentOffset()
    {
        if( mTexRefOffsets.size() > 0 ){
            return mTexRefOffsets[ mPlayheadPosition ];
        } else {
            return ci::ivec2(0);
        }
    }
    
    ci::gl::TextureRef const TextureSequence::getCurrentTexture()
    {
        if( mTextureRefs.size() > 0 ){
            return mTextureRefs[ mPlayheadPosition ];
        } else {
            return ci::gl::Texture::create(0,0);    // maybe we should return a null pointer?
        }
    }
    
    void TextureSequence::draw()
    {
        ci::gl::translate( getCurrentOffset() );
        ci::gl::draw( getCurrentTexture() );
    }
}
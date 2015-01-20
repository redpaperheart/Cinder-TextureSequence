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

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"

namespace rph{

    class TextureSequence{
    public:
        TextureSequence();
        ~TextureSequence();
        
        virtual void setup(const std::vector<ci::gl::TextureRef> &textureRefs, const float &fps = 0.0f);
        virtual void update();

        virtual void play(bool reverse = false);
        virtual void stop();
        virtual void reset();

        void stepForward( int frameInc = 1 );
        void stepBackward( int frameInc = 1 );
        
        void setFramerate( float fps );
        void setPlayheadPosition( int newPosition );
        void setPlayheadPositionByPerc( float perc );
        void setLoop( bool doLoop )                 { mLooping = doLoop; }
        
        bool isPlaying()                            { return mPlaying; }                    // returns true if sequence is currently playing
        bool isEmpty()                              { return mTextureRefs.empty(); };       // returns true if sequence is set up
        bool isDone()const                          { return mComplete; }                   // returns true if sequence played thru and looping = false;
        bool isLooping( )                           { return mLooping; }
        
        int getNumFrames()const                     { return mNumFrames; }
        int getPlayheadPosition() const             { return mPlayheadPosition; }
        float getFramerate()                        { return mFps; }
        
        ci::gl::TextureRef const getCurrentTexture();
        
    protected:
        std::vector<ci::gl::TextureRef> mTextureRefs;
        
        int mPlayheadPosition = 0;
        int mNumFrames = 0;
        
        float mFps = 0.0f;
        float mTimePerFrame = 0.0f;
        float mStartTime = 0.0f;
        float mStartFrame = 0.0f;
        
        bool mLooping = false;
        bool mPlaying = false;
        bool mComplete = false;
        bool mPlayReverse = false;
    };
}
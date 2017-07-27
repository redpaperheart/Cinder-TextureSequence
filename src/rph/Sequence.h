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
#include "cinder/TriMesh.h"

using namespace ci;

namespace rph {
    
    template<typename T>
    class Sequence {
        
    public:
        Sequence()  {}
        ~Sequence() {}
        
        //! Setup with a vector of objects and optional framerate
        virtual void setup( const std::vector<T> &frames, const float &fps = 30.0f )
        {
            mFrames.clear();
            mFrames = frames;
            mNumFrames = mFrames.size();
            setFramerate( fps );
            mHasOffsets = false;
        }
        
        //! Setup with a vector of objects, a vector of offsets and optional framerate
        virtual void setup( const std::vector<T> &frames, const std::vector<vec3> &offsets, const float &fps = 30.0f )
        {
            setup(frames, fps);
            mOffsets.clear();
            mOffsets = offsets;
            mHasOffsets = true;
        }
        
        //! Call on each frame to update the playhead
        virtual void update()
        {
            if( mPlaying ){
                int frameToBe = mStartFrame + (mPlayReverse?-1:1) * ((app::getElapsedSeconds() - mStartTime)/mTimePerFrame);
                int newPosition = frameToBe;

                if( newPosition > mNumFrames - 1 ){
                    if( mLooping ){
                        mComplete = false;
                        mPlayheadPosition = newPosition % mNumFrames;
                    } else {
                        mComplete = true;
                        stop();
                    }
                } else if( newPosition < 0 ) {
                    if( mLooping ){
                        mComplete = false;
                        mPlayheadPosition = (mNumFrames + (newPosition % mNumFrames))%mNumFrames ;
                        
                    } else {
                        mComplete = true;
                        stop();
                    }
                } else {
                    mComplete = false;
                    mPlayheadPosition = newPosition;
                }
            }
        }

        //! Begins playback of sequence
        virtual void play( bool reverse = false )
        {
            if(!mFrames.empty()){
                mPlayReverse = reverse;
                mPlaying = true;
                mComplete = false;
                mStartTime = app::getElapsedSeconds();
                mStartFrame = mPlayheadPosition;
            }
        }
        
        //! Pauses playback
        virtual void stop()
        {
            mPlaying = false;
        }
        
        //! Plays/pauses playback
        virtual void togglePlayback()
        {
            if (isPlaying()) stop();
            else play();
        }
        
        //! Stops playback and resets the playhead to zero
        virtual void reset()
        {
            mPlayheadPosition = 0;
            mPlaying = false;
        }
        
        //! Set playback frame rate
        void setFramerate( float fps )
        {
            mFps = fps;
            mTimePerFrame = 1.0 / mFps;
        }
        
        //! Seek to a new position in the sequence [frames]
        void setPlayheadPosition( int newPosition )
        {
            mPlayheadPosition = math<int>::max( 0, math<int>::min( newPosition, mNumFrames - 1 ) );
        }
        
        //! Seek to a new position in the sequence [0.0 - 1.0]
        void setPlayheadPositionByPerc( float perc )
        {
            perc = math<float>::max( 0.0f, math<float>::min( perc, 1.0f ) );
            setPlayheadPosition( perc * (mNumFrames - 1) );
        }
        
        //! Get object at the current playhead position
        T const getCurrentFrame()
        {
            if( mFrames.size() > 0 ){
                return mFrames.at( mPlayheadPosition );
            } else {
                return NULL;
            }
        }
        
        //! Get offset of the current frame (for trimmed/optimized img sequences)
        vec3 getCurrentOffset()
        {
            if( mOffsets.size() > 0 ){
                return mOffsets.at( mPlayheadPosition );
            } else {
                return vec3(0);
            }
        }
        
        //! TODO
        void stepForward( int frameInc = 1 )        {}
        void stepBackward( int frameInc = 1 )       {}
        
        void setLoop( bool doLoop )                 { mLooping = doLoop; }
        
        bool isPlaying()                            { return mPlaying; }                    // returns true if sequence is currently playing
        bool isEmpty()                              { return mFrames.empty(); }             // returns true if sequence is set up
        bool isDone() const                         { return mComplete; }                   // returns true if sequence played thru and looping = false;
        bool isLooping()                            { return mLooping; }
        bool hasOffsets()                           { return mHasOffsets; }
        
        int getNumFrames() const                    { return mNumFrames; }
        int getPlayheadPosition() const             { return mPlayheadPosition; }
        float getFramerate()                        { return mFps; }
        float getDuration()                         { return float(mNumFrames) / mFps; }
        
        void setSize(vec2 xy)                       { mSize = vec3(xy.x, xy.y, 0); }
        void setSize(vec3 xyz)                      { mSize = xyz; }
        vec3 getSize()                              { return mSize; }
        
    protected:
        
        std::vector<T> mFrames;
        std::vector<vec3> mOffsets;
        vec3 mSize = vec3(0);
        
        int mPlayheadPosition = 0;
        int mNumFrames = 0;
        
        float mFps = 30.0f;
        float mTimePerFrame = 0.0f;
        float mStartTime = 0.0f;
        float mStartFrame = 0.0f;
        
        bool mLooping = false;
        bool mPlaying = false;
        bool mComplete = false;
        bool mPlayReverse = false;
        bool mHasOffsets = false;
    };
    
    // Some handy typedefs
    typedef Sequence<gl::TextureRef>    TextureSequence;
    typedef Sequence<TriMeshRef>        TriMeshSequence;
}

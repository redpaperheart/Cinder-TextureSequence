#include "TextureSequence.h"

namespace rph {


    TextureSequence::TextureSequence(){}

    TextureSequence::~TextureSequence(){
        mTextureRefs.clear();
    }
    
    void TextureSequence::setup(const std::vector<ci::gl::TextureRef> &textureRefs, const float &fps ){
        mTextureRefs.clear();
        mTextureRefs = textureRefs;
        mNumFrames = mTextureRefs.size();
        setFramerate( fps );
    }
    
    void TextureSequence::setFramerate( float fps){
        mFps = fps;
        mTimePerFrame = 1.0/mFps;
    }
    
    /**
     *  -- Begins playback of sequence
     */
    void TextureSequence::play(bool reverse) {
        
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
    void TextureSequence::stop(){
        mPlaying = false;
        ci::app::console() << "stop"<< std::endl;
    }

    /**
     *  -- Stops playback and resets the playhead to zero
     */
    void TextureSequence::reset(){
        mPlayheadPosition = 0;
        mPlaying = false;
    }

    /**
     *  -- Seek to a new position in the sequence
     */
    void TextureSequence::setPlayheadPosition( int newPosition ){
        mPlayheadPosition = ci::math<int>::max( 0, ci::math<int>::min( newPosition, mNumFrames - 1 ) );
    }
    /**
     *  -- Seek to a new position in the sequence
     */
    void TextureSequence::setPlayheadPositionByPerc( float perc ){
        perc = ci::math<float>::max( 0.0f, ci::math<float>::min( perc, 1.0f ) );
        setPlayheadPosition( perc * (mNumFrames - 1) );
    }

    void TextureSequence::stepForward( int frameInc ){
        
    }
    void TextureSequence::stepBackward(int frameInc ){
        
    }
    
    /**
     *  -- Call on each frame to update the playhead
     */
    void TextureSequence::update(){
        
        if( mPlaying ){
            int frameToBe = mStartFrame + (mPlayReverse?-1:1) * ((ci::app::getElapsedSeconds() - mStartTime)/mTimePerFrame);
            int frameInc = frameToBe - mPlayheadPosition;
            //int newPosition = mPlayheadPosition + frameInc;
            int newPosition = frameToBe;
            
            ci::app::console() << "1 . Frame to be: " << frameToBe << ", newPosition: "<< newPosition<< ", playheadPos: "<< mPlayheadPosition << ", frameInc: " << frameInc << std::endl;
            
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
            
            ci::app::console() << "2 . Frame to be: " << frameToBe << ", newPosition: "<< newPosition<< ", playheadPos: "<< mPlayheadPosition << std::endl;
        }
        
    }

    ci::gl::TextureRef const TextureSequence::getCurrentTexture() {
        if(mTextureRefs.size() > 0){
            return mTextureRefs[ mPlayheadPosition ];
        }else{
            return ci::gl::Texture::create(0,0);
        }
        
    }

}
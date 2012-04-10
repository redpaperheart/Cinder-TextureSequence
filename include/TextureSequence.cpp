#include "TextureSequence.h"

TextureSequence::TextureSequence() : playheadPosition( 0 ), playheadFrameInc( 1 ), paused( false ), playing( true ), looping( true ), mFps(0.0f){
    
}

TextureSequence::~TextureSequence(){
    textures.clear();
}


/**
 *  -- Begins playback of sequence
 */
void TextureSequence::play() {
    paused = false;
    playing = true;
}

/**
 *  -- Pauses playback
 */
void TextureSequence::pause(){
    paused = true;
    playing = false;
}

/**
 *  -- Stops playback and resets the playhead to zero
 */
void TextureSequence::stop(){
    playheadPosition = 0;
    playing = false;
    paused = false;
}

/**
 *  -- Call on each frame to update the playhead
 */
void TextureSequence::update(){
    if(mFps && (getElapsedSeconds()-mStartTime) < 1/mFps ){
        return;
    }
    if( !paused && playing ){
        int newPosition = playheadPosition + playheadFrameInc;
        if( newPosition > totalFrames - 1 ){
            if( looping ){
                complete = false;
                playheadPosition = newPosition - totalFrames;
            } else {
                complete = true;
            }
            
        } else if( newPosition < 0 ) {
            if( looping ){
                complete = false;
                playheadPosition = totalFrames - abs( newPosition );
            } else {
                complete = true;
            }
        } else {
            complete = false;
            playheadPosition = newPosition;
        }
    }
    mStartTime = getElapsedSeconds();
}

/**
 *  -- Seek to a new position in the sequence
 */
void TextureSequence::setPlayheadPosition( int newPosition ){
    playheadPosition = max( 0, min( newPosition, totalFrames ) );
}
/**
 *  -- Seek to a new position in the sequence
 */
void TextureSequence::setPlayheadPositionByPerc( float perc ){
    perc = max( 0.0f, min( perc, 1.0f ) );
    setPlayheadPosition( perc * totalFrames );
}



void TextureSequence::createFromTextureList(const vector<Texture *> &textureList, const float &fps ){
    mStartTime = getElapsedSeconds();
    mFps = fps;
    textures.clear();
    textures = textureList;
    totalFrames = textures.size();
}

/**
 *  -- Loads all files contained in the supplied director and creates Textures from them
 */
void TextureSequence::createFromDir(const string &filePath, const float &fps, gl::Texture::Format format ){
    mStartTime = getElapsedSeconds();
    mFps = fps;
    textures.clear();
    fs::path p( filePath );
    for ( fs::directory_iterator it( p ); it != fs::directory_iterator(); ++it ){
        if ( fs::is_regular_file( *it ) ){
            // -- Perhaps there is a better way to ignore hidden files
            string fileName = it->path().filename().string();
            if( !( fileName.compare( ".DS_Store" ) == 0 ) ){
                textures.push_back( new Texture( loadImage( filePath + fileName ), format ) );
            }
        }
    }
    totalFrames = textures.size();
}

/**
 *  -- Loads all of the images in the supplied list of file paths
 */
void TextureSequence::createFromPathList(const vector<string> &paths, const float &fps ){
    mStartTime = getElapsedSeconds();
    mFps = fps;
    textures.clear();
    for ( int i = 0; i < paths.size(); ++i ){
        textures.push_back( new Texture( loadImage( paths[i] ) ) );
    }
    totalFrames = textures.size();
}


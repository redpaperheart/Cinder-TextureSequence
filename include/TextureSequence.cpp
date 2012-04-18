#include "TextureSequence.h"

TextureSequence::TextureSequence() : playheadPosition( 0 ), playheadFrameInc( 1 ), paused( false ), playing( true ), looping( true ), mFps( 0.0f ), playReverse( false ){
    
}

TextureSequence::~TextureSequence(){
    textures.clear();
}

void TextureSequence::setFlipped( bool horizontal, bool vertical ){
    flipHorizontal = horizontal;
    flipVertical = vertical;
    // create a matrix here to apply before drawing;
}

/**
 *  -- Begins playback of sequence
 */
void TextureSequence::play(bool reverse) {
    playReverse = reverse;
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
 *  -- Seek to a new position in the sequence
 */
void TextureSequence::setPlayheadPosition( int newPosition ){
    playheadPosition = max( 0, min( newPosition, totalFrames - 1 ) );
}
/**
 *  -- Seek to a new position in the sequence
 */
void TextureSequence::setPlayheadPositionByPerc( float perc ){
    perc = max( 0.0f, min( perc, 1.0f ) );
    setPlayheadPosition( perc * (totalFrames - 1) );
}

/**
 *  -- Call on each frame to update the playhead
 */
void TextureSequence::update(){
    if(mFps && (getElapsedSeconds()-mStartTime) < 1/mFps ){
        return;
    }
    if( !paused && playing ){
        int newPosition = playheadPosition + (playReverse ? -playheadFrameInc : playheadFrameInc);
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

//void TextureSequence::draw(){
//    gl::pushMatrices();    
//    gl::draw(*getCurrentTexture());
//    gl::popMatrices();
//}



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
                try{
                    //try loading from resource folder
                    textures.push_back( new Texture( loadImage( loadResource( filePath + fileName ) ), format ) );
                }catch(...){
                    try { 
                        // try to load relative to app
                        textures.push_back( new Texture( loadImage( loadFile( filePath + fileName ) ), format ) );
                    }
                    catch(...) { 
                        try {
                            // try to load from URL
                            textures.push_back( new Texture( loadImage( loadUrl( filePath + fileName ) ), format ) );
                        }
                        catch(...) {
                            console() << getElapsedSeconds() << ":" << "TextureSequence failed to load:" << (filePath + fileName) << endl;
                        }
                    }
                }
                //textures.push_back( new Texture( loadImage( filePath + fileName ), format ) );
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


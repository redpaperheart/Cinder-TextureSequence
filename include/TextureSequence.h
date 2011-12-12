#pragma once

#include <string>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Filesystem.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace std;

class TextureSequence{
public:
    TextureSequence();
    ~TextureSequence();
    
    void stop();
    void play();
    void pause();
    void update();
    void createFromDir( string path );
    void createFromPathList( vector<string> paths );
    void createFromTextureList( vector<Texture *> textureList );
    
    int getTotalFrames()const{ return totalFrames; } 
    
    int getPlayheadFrameInc() const { return playheadFrameInc; }
    void setPlayheadFrameInc( int frames ) { playheadFrameInc = frames; }
    
    int getPlayheadPosition() const { return playheadPosition; }
    void setPlayheadPosition( int newPosition );
    
    void setLooping( bool doLoop ) { looping = doLoop; }
    
    Texture* const getCurrentTexture() { return textures[ playheadPosition ]; }
    
    bool isPlaying() { return playing; }
    bool isPaused() { return paused; }
    
protected:
    int playheadPosition;
    int playheadFrameInc;
    vector<Texture *> textures;
    
    int totalFrames;
    bool looping;
    bool paused;
    bool playing;
    bool complete;
};
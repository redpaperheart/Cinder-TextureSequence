#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include "rph/Sequence.h"
#include "rph/SequenceLoader.h"

using namespace ci;
using namespace ci::app;

class BasicSampleApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    void fileDrop(FileDropEvent event) override;
    
    bool bDrawOutlines = false;
  
    rph::Sequence<gl::TextureRef> mSequence;
};

void BasicSampleApp::setup()
{
    // load a set of textureRefs
    std::vector<gl::TextureRef> textureRefs = rph::SequenceLoader::loadTextureFolder( getAssetPath("RPHDripSequence") );
    
    // and setup texture sequence
    mSequence.setup( textureRefs, 30 );
    mSequence.setLoop( true );
    mSequence.play();
}

void BasicSampleApp::update()
{
    mSequence.update();
}

void BasicSampleApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );

    // draw texture
    {
        gl::ScopedMatrices mat;
        gl::translate( mSequence.getCurrentOffset() );
        gl::draw( mSequence.getCurrentFrame() );
        
        if (bDrawOutlines) {
            gl::ScopedColor outlineCol(0.5, 0.5, 0.5);
            gl::drawStrokedRect( mSequence.getCurrentFrame()->getBounds() );
        }
    }
    
    // draw playhead
    float width = (getWindowWidth() - 20) / (float)mSequence.getNumFrames();
    
    for ( int i = 0; i < mSequence.getNumFrames(); i++ ) {
        if( i == mSequence.getPlayheadPosition() )  gl::color(1, 0, 0);
        else if( i % 2 == 0 )                       gl::color(1, 1, 1);
        else                                        gl::color(0.9, 0.9, 0.9);
        
        Rectf rect(i*width+10, getWindowHeight()-20, i*width+width+10, getWindowHeight()-10);
        gl::drawSolidRect( rect );
    }
    
    // draw debug strings
    gl::drawString( toString( (int)getAverageFps() ), vec2(20, 20));
    gl::drawString( (mSequence.isLooping() ? "looping" : " not looping"), vec2(20, 40));
    gl::drawString( "drop an image folder to visualize a different sequence", vec2(20, getWindowHeight() - 40));
}

void BasicSampleApp::keyDown( KeyEvent event )
{
    switch (event.getChar()) {
        case ' ':
            mSequence.togglePlayback();
            break;
        case 'r':
            mSequence.play(true);
            break;
        case 'l':
            mSequence.setLoop( !mSequence.isLooping() );
            break;
        case 'o':
            bDrawOutlines = !bDrawOutlines;
            break;
    }
}

void BasicSampleApp::fileDrop(FileDropEvent event)
{
    const fs::path& path = event.getFile(0);
    
    if (fs::is_directory(path)) {
        // use the SequenceLoader to setup the sequence automatically
        // it will take care of using the JSON offset file there is one
        mSequence = rph::SequenceLoader::makeFromFolder(path);
        mSequence.setFramerate( 30 );
        mSequence.setLoop( true );
        mSequence.play();
    }
}

CINDER_APP( BasicSampleApp, RendererGl )

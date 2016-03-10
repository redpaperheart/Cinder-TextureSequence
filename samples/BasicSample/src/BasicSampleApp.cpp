#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include "rph/TextureSequence.h"

using namespace ci;
using namespace ci::app;

class BasicSampleApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    
    std::vector<gl::TextureRef> loadTexturesFromDir(fs::path dir, gl::Texture::Format format = gl::Texture::Format() );
    
    rph::TextureSequence mSequence;
};

void BasicSampleApp::setup()
{
    // load a set of textureRefs
    std::vector<gl::TextureRef> textureRefs = loadTexturesFromDir( getAssetPath("RPHDripSequence") );
    
    // setup texture sequence
    mSequence.setup( textureRefs, 30 );
    mSequence.setLoop(true);
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
        gl::translate( getWindowCenter() - vec2(mSequence.getCurrentTexture()->getSize()) * 0.5f);
        gl::draw( mSequence.getCurrentTexture() );
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
    gl::drawString( toString( getAverageFps() ), vec2(20, 20));
    gl::drawString( (mSequence.isLooping() ? "looping" : " not looping"), vec2(20, 40));
}

void BasicSampleApp::keyDown( KeyEvent event )
{
    switch (event.getChar()) {
        case ' ':
            if( mSequence.isPlaying() ) mSequence.stop();
            else                        mSequence.play();
            break;
        case 'r':
            mSequence.play(true);
            break;
        case 'l':
            mSequence.setLoop( !mSequence.isLooping() );
            break;
    }
}

// Loads all files contained in the supplied directory and creates Textures from them

std::vector<gl::TextureRef> BasicSampleApp::loadTexturesFromDir(fs::path dir, gl::Texture::Format format )
{
    std::vector<gl::TextureRef> textureRefs;
    
    if ( !fs::exists( dir ) ) {
        app::console() << "LoadImageDirectory - ERROR - ("<< dir << ") Folder does not Exist!" << std::endl;
        return textureRefs;
    }
    for ( fs::directory_iterator it( dir ); it != fs::directory_iterator(); ++it ){
        if ( fs::is_regular_file( *it ) ) {
            
            // Perhaps there is a better way to ignore hidden files
            if ( it->path().extension() != ".DS_Store" ) {
                try {
                    textureRefs.push_back( gl::Texture::create( loadImage( loadFile( it->path() ) ), format ) );
                }
                catch (...) {
                    console() << " LoadImageDirectory failed to load: " << it->path().filename() << std::endl;
                }
            }
        }
    }
    return textureRefs;
}

CINDER_APP( BasicSampleApp, RendererGl )

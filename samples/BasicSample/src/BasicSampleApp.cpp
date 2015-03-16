#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include "rph/Sequence.h"

using namespace ci;
using namespace ci::app;

class BasicSampleApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
    void keyDown(KeyEvent event);
    
    std::vector<gl::TextureRef> createTextureRefsFromDir(fs::path dir, gl::Texture::Format format = gl::Texture::Format() );
  
    std::vector<Rectf>              mFrames;     // rectangles to draw the timeline
    rph::Sequence<gl::TextureRef>   mSequence;  // stored texture sequence
};

void BasicSampleApp::setup()
{
    // load a set of textureRefs
    std::vector<gl::TextureRef> textureRefs = createTextureRefsFromDir("sequences/RedPaperHeartDrip");
    
    // setup texture sequence
    mSequence.setup( textureRefs, 30 );
    mSequence.setLoop(true);
    mSequence.play();
    
    // save rectangles to draw the playhead
    float width = (getWindowWidth() - 20) / (float)mSequence.getNumFrames();
    for( int i=0; i < mSequence.getNumFrames(); i++ ){
         mFrames.push_back( Rectf(i*width+10, getWindowHeight()-20, i*width+width+10, getWindowHeight()-10) );
    }
    
    gl::enableAlphaBlending();
}

void BasicSampleApp::update()
{
    mSequence.update();
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    // draw playhead
    for( int i=0; i < mFrames.size(); i++ ){
        if( i % 2 == 0 )    gl::color(1, 1, 1);
        else                gl::color(0.9, 0.9, 0.9);
        if( i == mSequence.getPlayheadPosition() ) gl::color(1, 0, 0);
        gl::drawSolidRect( mFrames[i] );
    }
    
    // draw texture
    gl::pushMatrices();
        gl::translate( getWindowCenter() - vec2(mSequence.getCurrentFrame()->getSize()) * 0.5f);
        gl::draw( mSequence.getCurrentFrame() );
    gl::popMatrices();
    
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

/**
 *  -- Loads all files contained in the supplied director and creates Textures from them
 */
std::vector<gl::TextureRef> BasicSampleApp::createTextureRefsFromDir(fs::path dir, gl::Texture::Format format )
{
    std::vector<gl::TextureRef> textureRefs;
    if( !fs::exists( dir ) ){
        dir = app::App::getResourcePath() / dir;
        if( !fs::exists(dir) ){
            app::console() << "LoadImageDirectory - ERROR - ("<< dir << ") Folder does not Exist!" << std::endl;
            return textureRefs;
        }
    }
    for ( fs::directory_iterator it( dir ); it != fs::directory_iterator(); ++it ){
        if ( fs::is_regular_file( *it ) ){
            
            // -- Perhaps there is a better way to ignore hidden files
            std::string fileName = it->path().filename().string();
            if( !( fileName.compare( ".DS_Store" ) == 0 ) ){
                fileName = dir.string() +"/"+ fileName;
                try{
                    //try loading from resource folder
                    textureRefs.push_back( gl::Texture::create( loadImage( loadResource( fileName ) ), format ) );
                }catch(...){
                    try {
                        // try to load relative to app
                        textureRefs.push_back( gl::Texture::create( loadImage( loadFile( fileName ) ), format ) );
                    }
                    catch(...) {
                        try {
                            // try to load from URL
                            textureRefs.push_back( gl::Texture::create( loadImage( loadUrl( fileName ) ), format ) );
                        }
                        catch(...) {
                            console() << getElapsedSeconds() << ":" << "Failed to load:" << ( fileName) << std::endl;
                        }
                    }
                }
            }
        }
    }
    return textureRefs;
}

/**
 *  -- Loads all of the images in the supplied list of file paths.
 *  -- TODO: This might be a good start to quickly get it to work on windows.
 */
//void TextureSequence::createFromPathList(const vector<string> &paths, const float &fps ){
//    mStartTime = getElapsedSeconds();
//    mFps = fps;
//    textures.clear();
//    for ( int i = 0; i < paths.size(); ++i ){
//        textures.push_back( Texture( loadImage( paths[i] ) ) );
//    }
//    totalFrames = textures.size();
//}

CINDER_APP_NATIVE( BasicSampleApp, RendererGl )

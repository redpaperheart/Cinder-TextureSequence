#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include "rph/TextureSequence.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public AppNative {
  public:
    void prepareSettings(cinder::app::AppBasic::Settings *settings);
	void setup();
	void update();
	void draw();
    
    void keyDown( ci::app::KeyEvent event );
    
    std::vector<ci::gl::TextureRef> createTextureRefsFromDir(ci::fs::path dir, gl::Texture::Format format = ci::gl::Texture::Format() );
    
    rph::TextureSequence mSequence;
    
    std::vector<Rectf> mFrames;
};

void BasicSampleApp::prepareSettings(cinder::app::AppBasic::Settings *settings)
{
    settings->setWindowSize( 800, 600 );
    settings->setFrameRate( 30.0 );
    
}

void BasicSampleApp::setup(){
    std::vector<ci::gl::TextureRef> textureRefs = createTextureRefsFromDir("sequences/RedPaperHeartDrip");
    mSequence.setup( textureRefs, 30 );
    //mSequence.setLoop(true);
    mSequence.play();
    
    float width = ci::app::getWindowWidth()-20;
    width /= mSequence.getNumFrames();
    for( int i=0; i < mSequence.getNumFrames(); i++){
         mFrames.push_back( Rectf(i*width+10, getWindowHeight()-20, i*width+width+10, getWindowHeight()-10) );
    }
    gl::enableAlphaBlending();
}

void BasicSampleApp::keyDown( ci::app::KeyEvent event )
{
    
        if( event.getCode() == ci::app::KeyEvent::KEY_SPACE){
            if(mSequence.isPlaying()){
                mSequence.stop();
            }else{
                mSequence.play();
            }
        } else if( event.getCode() == ci::app::KeyEvent::KEY_r ){
            mSequence.play(true);
            
        } else if( event.getCode() == ci::app::KeyEvent::KEY_l ){
            mSequence.setLoop( !mSequence.isLooping() );
            
        }
}

void BasicSampleApp::update(){
    mSequence.update();
}

void BasicSampleApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    gl::color(1,1,1);
    
    gl::pushMatrices();
    for( int i=0; i < mFrames.size(); i++){
        if(i%2==0) gl::color(1,1,1); else gl::color(0.9,0.9,0.9);
        if(i == mSequence.getPlayheadPosition()) gl::color(1,0,0);
        gl::drawSolidRect( mFrames[i] );
    }
    gl::popMatrices();
    
    gl::pushMatrices();
        gl::translate(ci::app::getWindowCenter() - vec2(mSequence.getCurrentTexture()->getSize()) * 0.5f);
        gl::draw( mSequence.getCurrentTexture() );
    gl::popMatrices();
    
    gl::drawString( ci::toString( getAverageFps() ), vec2(20,20));
    gl::drawString( (mSequence.isLooping() ? "looping" : " not looping"), vec2(20,40));
}


/**
 *  -- Loads all files contained in the supplied director and creates Textures from them
 */
std::vector<ci::gl::TextureRef> BasicSampleApp::createTextureRefsFromDir(ci::fs::path dir, gl::Texture::Format format ){
    std::vector<ci::gl::TextureRef> textureRefs;
    if( !ci::fs::exists( dir ) ){
        //ci::app::console() << "rph::TextureStore::loadImageDirectory - WARNING - ("<< dir << ") Folder does not Exist!" << std::endl;
        dir = ci::app::App::getResourcePath() / dir;
        if( !ci::fs::exists(dir) ){
            ci::app::console() << "rph::TextureStore::loadImageDirectory - ERROR - ("<< dir << ") Folder does not Exist!" << std::endl;
            return textureRefs;
        }
    }
    for ( fs::directory_iterator it( dir ); it != fs::directory_iterator(); ++it ){
        if ( fs::is_regular_file( *it ) ){
            
            // -- Perhaps there is a better way to ignore hidden files
            string fileName = it->path().filename().string(); 
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
                            console() << getElapsedSeconds() << ":" << "TextureSequence failed to load:" << ( fileName) << endl;
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

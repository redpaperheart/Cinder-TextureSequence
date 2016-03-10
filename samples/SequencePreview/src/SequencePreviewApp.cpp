#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Json.h"

#include "TextureSequence.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SequencePreviewApp : public App {
  public:
	void prepareSettings( Settings *settings);
    void setup() override;
	void update() override;
	void draw() override;
    void cleanup() override;
    void fileDrop( FileDropEvent event ) override;
    void addNewSequence( const fs::path& path );
    
    ci::gl::TextureRef load( const std::string &url, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
    std::vector<ci::gl::TextureRef> loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
    
    bool                    mLoop = true;
    bool                    mDrawTexOutline = false;
    bool                    mDrawBgColor = true;
    float                   mBgTexAlpha = 0.5f;
    ci::gl::TextureRef      mBgTexRef = NULL;
    ci::Color               mBgColor = ci::Color(0,0,0);
    
    rph::TextureSequence    *mSequence = NULL;
    
    ci::params::InterfaceGl mParams;
};

void SequencePreviewApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 960, 960 );
    settings->setFrameRate( 60.0 );
}

void SequencePreviewApp::setup(){
    
    mParams = ci::params::InterfaceGl( "Settings", vec2(300,200));
    mParams.addParam( "Draw Texture Outline", &mDrawTexOutline );
    mParams.addParam( "Draw BG", &mDrawBgColor );
    mParams.addParam( "mBGTexAlpha", &mBgTexAlpha, "min=0 max=1 step=0.01" );
    mParams.addSeparator();
    mParams.addParam( "BG Color", &mBgColor);
    
    mBgTexRef = ci::gl::Texture::create( ci::loadImage( loadAsset("bg.png" ) ) );
}

void SequencePreviewApp::fileDrop( FileDropEvent event ){
    
    ci::app::console() << "You dropped files @ " << event.getPos() << " and the files were: " << std::endl;
    
    for( size_t s = 0; s < event.getNumFiles(); ++s ){
        const fs::path& path = event.getFile( s );
        ci::app::console() << path << std::endl;

        if(ci::fs::is_directory(path)){
            double t = ci::app::getElapsedSeconds();
            mSequence = new rph::TextureSequence();
            
            // check if there's json file in the folder you just droppped.
            
            if( ci::fs::exists( path / "sequence.json" ) ){
                console() << "JSON EXISTS" << endl;
                // load the json
                const JsonTree jsonFile = ci::JsonTree( ci::loadFile( path/"sequence.json") );
                mSequence->setup( loadImageDirectory( path ), jsonFile );
            }else{
                mSequence->setup( loadImageDirectory( path ) );
                console() << "JSON DOES NOT EXISTS" << endl;
            }
            
            mSequence->setLoop(true);
            mSequence->play();
            console() << "Loaded and set up after: " << toString( ci::app::getElapsedSeconds() - t) << " sec" << endl;
        }
        else{
            
            console() << "!! WARNING :: not a folder: " <<  path << endl;
        }
    }
}

void SequencePreviewApp::addNewSequence( const fs::path& path )
{
    // remove and kill the old sequence
    
    // create and add a new one.
    mSequence = new rph::TextureSequence();
    mSequence->setup( loadImageDirectory( path ) );
    mSequence->setLoop(true);
    mSequence->play();
    
}

void SequencePreviewApp::update()
{
    if(mSequence){
        mSequence->update();
    }
}

void SequencePreviewApp::draw()
{
	gl::clear( mBgColor );
    
    if(mBgTexRef && mDrawBgColor){
        gl::color(ColorA(1,1,1,mBgTexAlpha));
        gl::draw( mBgTexRef );
    }
    
    if(mSequence){
        gl::ScopedMatrices m;
        
        //gl::translate( getWindowCenter() -  vec2(mSequence->getCurrentTexture()->getSize())/vec2(2.0f));
        gl::color(ColorA(1,1,1,1));
        
        
        gl::translate( mSequence->getCurrentOffset() );
        gl::draw( mSequence->getCurrentTexture() );
        
        if(mDrawTexOutline){
            gl::color(ColorA(1,0,0,1));
            gl::drawStrokedRect(mSequence->getCurrentTexture()->getBounds());
        }
    }
    
    mParams.draw();
}


ci::gl::TextureRef SequencePreviewApp::load( const std::string &url, ci::gl::Texture::Format fmt )
{
    try{
        ci::gl::TextureRef t = ci::gl::Texture::create( ci::loadImage( url ), fmt );
        return t;
    }
    catch(...){}
    ci::app::console() << ci::app::getElapsedSeconds() << ": error loading texture '" << url << "'!" << std::endl;
    return NULL;
}

std::vector<ci::gl::TextureRef> SequencePreviewApp::loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt)
{
    std::vector<ci::gl::TextureRef> textureRefs;
    textureRefs.clear();
    for ( ci::fs::directory_iterator it( dir ); it != ci::fs::directory_iterator(); ++it ){
        if ( ci::fs::is_regular_file( *it ) ){
            // -- Perhaps there is a better way to ignore hidden files
            std::string fileName = it->path().filename().string();
            
            // this might give you an error when trying to load the json file
            
            if(  fileName.compare( fileName.size()-4 , 4 , ".png" ) == 0  ){
                ci::gl::TextureRef t = load( dir.string() +"/"+ fileName , fmt );
                textureRefs.push_back( t );
            }
        }
    }
    return textureRefs;
}

void SequencePreviewApp::cleanup(){
    
};

CINDER_APP( SequencePreviewApp, RendererGl )


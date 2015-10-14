#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"




using namespace ci;
using namespace ci::app;
using namespace std;

class ImageOptimizerApp : public App {
public:
    
    static void prepareSettings( Settings *settings);
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    void fileDrop( FileDropEvent event ) override;
    void update() override;
    void draw() override;
    
    void loadImages(const fs::path& path );
    void renderSceneToFbo(std::vector<ci::gl::TextureRef> textureRefs);
    
    void trim( SurfaceRef surf, SurfaceRef surfTrim );
    
    SurfaceRef  mSurfaceOrigin;
    SurfaceRef  mSurfaceTrim;
    Area mTrimArea;
    gl::TextureRef  mTexture;
    gl::TextureRef  mImgTexure;
    gl::FboRef      mFbo;
    
    ci::gl::TextureRef load( const std::string &url, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
    std::vector<ci::gl::TextureRef> loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
};

void ImageOptimizerApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 960, 960 );
    settings->setFrameRate( 60.0 );
}

void ImageOptimizerApp::setup()
{
    gl::enableAlphaBlending();
}


void ImageOptimizerApp::renderSceneToFbo(std::vector<ci::gl::TextureRef> textureRefs)
{
    std::vector<ci::gl::TextureRef> textures = textureRefs;
    
    // save size of loaded images
    int width = textures[0]->getWidth();
    int height = textures[0]->getHeight();
    
    // create fbo
    mFbo = gl::Fbo::create(width, height, true);
    
    {
        // bind fbo
        gl::ScopedFramebuffer fbScp( mFbo );
        
        // set viewport and matrices
        gl::ScopedViewport scpVp( ivec2( 0 ), mFbo->getSize() );
        gl::ScopedMatrices matricesFbo;
        gl::setMatricesWindow(width, height);
        
        // clear fbo and draw images
        gl::clear(ColorA(0, 0, 0, 0));
        gl::color(1, 1, 1);
        
        // draw images on fbo
        for (gl::TextureRef tex: textures) {
            gl::draw(tex);
        }

    }
    
    // read pixels from fbo
    SurfaceRef srf = Surface::create(mFbo->readPixels8u( mFbo->getBounds() ));
    
//    trim(srf, mSurfaceTrim);
    mTexture = gl::Texture::create( *srf);
    

}

void ImageOptimizerApp::mouseDown( MouseEvent event )
{
}

void ImageOptimizerApp::keyDown(KeyEvent event){
    if (event.getChar() == 's') {
        Surface tempSurf = mSurfaceTrim->clone(mTrimArea);
        writeImage( getAppPath() / "trimmed.png", tempSurf );
    }
}

void ImageOptimizerApp::fileDrop(FileDropEvent event){
    
    stringstream ss;
    ss << "You dropped files @ " << event.getPos() << " and the files were: " << endl;
    
    for( size_t s = 0; s < event.getNumFiles(); ++s ){
        const fs::path& path = event.getFile( s );
        ss << event.getFile( s ) << endl;
        if(ci::fs::is_directory(path)){
            
            //pass the foler of images to FBO
            renderSceneToFbo(loadImageDirectory( event.getFile( s ) ));
            console() << ss.str() << endl;
        }else{
            console() << "!! WARNING :: not a folder: " <<  ss.str() << endl;
        }
    }
}

ci::gl::TextureRef ImageOptimizerApp::load( const std::string &url, ci::gl::Texture::Format fmt )
{
    try{
        ci::gl::TextureRef t = ci::gl::Texture::create( ci::loadImage( url ), fmt );
        return t;
    }
    catch(...){}
    ci::app::console() << ci::app::getElapsedSeconds() << ": error loading texture '" << url << "'!" << std::endl;
    return NULL;
}

std::vector<ci::gl::TextureRef> ImageOptimizerApp::loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt){
    
    std::vector<ci::gl::TextureRef> textureRefs;
    textureRefs.clear();
    for ( ci::fs::directory_iterator it( dir ); it != ci::fs::directory_iterator(); ++it ){
        if ( ci::fs::is_regular_file( *it ) ){
            // -- Perhaps there is a better way to ignore hidden files
            std::string fileName = it->path().filename().string();
            if( !( fileName.compare( ".DS_Store" ) == 0 ) ){
                ci::gl::TextureRef t = load( dir.string() +"/"+ fileName , fmt );
                textureRefs.push_back( t );
            }
        }
    }
    return textureRefs;
}

void ImageOptimizerApp::trim( SurfaceRef surf, SurfaceRef surfTrim ){
    int trimTop = 0; // number of lines to cut
    int trimBottom = 0;
    int trimLeft = 0;
    int trimRight = 0;
    
    ci::ColorA overlayColor = ColorA(1,0,0,0.3f);
    mSurfaceOrigin = surf;
    
    bool stop = false;
    for (int y = 0; y < mSurfaceOrigin->getHeight(); y++) {
        for (int x =0; x < mSurfaceOrigin->getWidth(); x++) {
            ci::ColorA c = mSurfaceOrigin->getPixel( vec2(x, y) );
            if ( c.a > 0.0f ) {
                trimTop = y;
                stop = true;
                break;
            }else{
                mSurfaceTrim->setPixel( vec2(x, y), overlayColor);
            }
        }
        if( stop ) break;
    }
    
    stop = false;
    for (int y = mSurfaceOrigin->getHeight() -1; y >=  0; y--) {
        for (int x =0; x < mSurfaceOrigin->getWidth(); x++) {
            ci::ColorA c = mSurfaceOrigin->getPixel( vec2(x, y) );
            if ( c.a > 0.0f ) {
                trimBottom = y+1;
                stop = true;
                break;
            }else{
                mSurfaceTrim->setPixel( vec2(x, y), overlayColor );
            }
        }
        if( stop ) break;
    }
    
    stop = false;
    for (int x = 0; x < mSurfaceOrigin-> getWidth(); x++) {
        for (int y =0; y < mSurfaceOrigin->getHeight(); y++) {
            ci::ColorA c = mSurfaceOrigin->getPixel( vec2(x, y) );
            if ( c.a > 0.0f ) {
                trimLeft = x;
                stop = true;
                break;
            }else{
                mSurfaceTrim->setPixel( vec2(x, y), overlayColor );
            }
        }
        if( stop ) break;
    }
    
    stop = false;
    for (int x = mSurfaceOrigin->getWidth()-1; x >= 0; x--) {
        for (int y =0; y < mSurfaceOrigin->getHeight(); y++) {
            ci::ColorA c = mSurfaceOrigin->getPixel( vec2(x, y) );
            if ( c.a > 0.0f ) {
                trimRight = x+1;
                stop = true;
                break;
            }else{
                mSurfaceTrim->setPixel( vec2(x, y), overlayColor );
            }
        }
        if( stop ) break;
    }
    
    mTrimArea = Area(trimLeft, trimTop, trimRight, trimBottom);
    
    cout<<"trimTop: " << trimTop << "trimBottom: " << trimBottom << "trimLeft: " <<trimLeft <<"trimRight: "<< trimRight<< std::endl;
    
    
}


void ImageOptimizerApp::update()
{
    
}

void ImageOptimizerApp::draw()
{
    gl::clear(Color(0,0,0));
    gl::color(1,1,1);
    
    // draw texture from fbo on screen
//    gl::draw( mFbo->getColorTexture() );
    
    //    gl::pushMatrices();
    //    gl::translate(20,20);
    //    Surface tempSurf = mSurfaceTrim->clone(mTrimArea);
    //    gl::draw( gl::Texture::create(tempSurf) );
    //    gl::popMatrices();
    
    gl::draw(mTexture);
    gl::color(0,0,1);
    gl::drawStrokedRect( ci::Rectf(mTrimArea) );
    
}

CINDER_APP( ImageOptimizerApp, RendererGl, ImageOptimizerApp::prepareSettings )

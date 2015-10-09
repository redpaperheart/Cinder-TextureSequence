#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
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
    void trim( SurfaceRef surf, SurfaceRef surfTrim );
    
    SurfaceRef  mSurfaceOrigin;
    SurfaceRef  mSurfaceTrim;
    gl::TextureRef  mTexture;
    Area mTrimArea;
    
    //int  trimTop, trimBottom, trimLeft, trimRight;
    
};

void ImageOptimizerApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 960, 960 );
    settings->setFrameRate( 60.0 );
}

void ImageOptimizerApp::setup()
{
    mSurfaceOrigin = Surface::create( loadImage( loadAsset("testImage2.png" )) );
    mSurfaceTrim = Surface::create( loadImage( loadAsset("testImage2.png" )) );
    
    trim( mSurfaceOrigin, mSurfaceTrim );
    mTexture = gl::Texture::create( *mSurfaceTrim );
    
    gl::enableAlphaBlending();
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
    
    //    try {
    //        fs::path path = getOpenFilePath( "", ImageIo::getLoadExtensions() );
    //        if( ! path.empty() ) {
    //            mSurfaceOrigin = Surface::create( loadImage( path ) );
    //            mSurfaceTrim = Surface::create( loadImage( path ) );
    //
    //        }
    //    }
    //    catch( Exception &exc ) {
    //        CI_LOG_EXCEPTION( "failed to load image.", exc );
    //    }
    
}


void ImageOptimizerApp::trim( SurfaceRef surf, SurfaceRef surfTrim ){
    
    int trimTop = 0; // number of lines to cut
    int trimBottom = 0;
    int trimLeft = 0;
    int trimRight = 0;
    
    ci::ColorA overlayColor = ColorA(1,0,0,0.3f);
    
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
    gl::draw( mTexture );
    
    //    gl::pushMatrices();
    //    gl::translate(20,20);
    //    Surface tempSurf = mSurfaceTrim->clone(mTrimArea);
    //    gl::draw( gl::Texture::create(tempSurf) );
    //    gl::popMatrices();
    
    gl::color(0,0,1);
    gl::drawStrokedRect( ci::Rectf(mTrimArea) );
    
}

CINDER_APP( ImageOptimizerApp, RendererGl, ImageOptimizerApp::prepareSettings )

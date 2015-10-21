#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Json.h"




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
    void renderSceneToFbo();
    void trim();
    void save();
    void saveJson();
    

    Area mTrimArea;
    gl::TextureRef     mResultTexture;
    gl::FboRef      mFbo;
    
    std::vector<gl::TextureRef> mTextures;
    std::vector<SurfaceRef> mSurfaces;
    std::vector<Area>trimOffsets;
    
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


void ImageOptimizerApp::renderSceneToFbo()
{
    // save size of loaded images
    int width = mTextures[0]->getWidth();
    int height = mTextures[0]->getHeight();
    
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
        for (gl::TextureRef tex: mTextures) {
            gl::draw(tex);
        }

    }
    
    // read pixels from fbo
//    SurfaceRef srf = Surface::create(mFbo->readPixels8u( mFbo->getBounds() ));
//    mTexture = gl::Texture::create( *srf);
    
    //read overlayed texuture from fbo
    mResultTexture = mFbo->getColorTexture();
}

void ImageOptimizerApp::mouseDown( MouseEvent event )
{
}

void ImageOptimizerApp::keyDown(KeyEvent event){

    if (event.getChar() == 't') {
            trim();// trim to export
    }
    
    if (event.getChar() == 's') {
        save();// trim to export
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
            loadImageDirectory( event.getFile( s ) );
            renderSceneToFbo();// visualize
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
                
                // load dropped images
                std::string path = dir.string() + "/" + fileName;
                SurfaceRef surf = Surface::create(loadImage(path));
                gl::TextureRef tex = gl::Texture::create(*surf);
                
                // save them in vector
                mTextures.push_back(tex);
                mSurfaces.push_back(surf);
            }
        }
    }
    return textureRefs;
}

void ImageOptimizerApp::trim(){
    // number of lines to cut
    int trimTop = 0;
    int trimBottom = 0;
    int trimLeft = 0;
    int trimRight = 0;
    
    //get the pixels need to be trimmed for each surface with 4 loops
    for (SurfaceRef surf:mSurfaces) {
        bool stop = false;
        //go thru pixels from top
        for (int y = 0; y < surf->getHeight(); y++) {
            for (int x =0; x < surf->getWidth(); x++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                //stop at the first non-transparent pixel
                if ( c.a > 0.0f ) {
                    trimTop = y;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //go thru from bottom
        stop = false;
        for (int y = surf->getHeight() -1; y >=  0; y--) {
            for (int x =0; x < surf->getWidth(); x++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                if ( c.a > 0.0f ) {
                    trimBottom = y+1;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //go thru from left
        stop = false;
        for (int x = 0; x < surf-> getWidth(); x++) {
            for (int y =0; y < surf->getHeight(); y++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                if ( c.a > 0.0f ) {
                    trimLeft = x;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //from right
        stop = false;
        for (int x = surf->getWidth()-1; x >= 0; x--) {
            for (int y =0; y < surf->getHeight(); y++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                if ( c.a > 0.0f ) {
                    trimRight = x+1;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //pixel offsets need to be trimmed for each image
        mTrimArea = Area(trimLeft, trimTop, trimRight, trimBottom);
        //push to vector holds all the trim offsets
        trimOffsets.push_back(mTrimArea);
    }
    cout<<"trimTop: " << trimTop << "trimBottom: " << trimBottom << "trimLeft: " <<trimLeft <<"trimRight: "<< trimRight<< std::endl;
}

void ImageOptimizerApp::save(){
    //go thru each surface
    for (int i = 0; i < mSurfaces.size();i++) {
        //only clone the non-transparent area based on the offsets
        Surface tempSurf = mSurfaces[i]->clone(trimOffsets[i]);
        //save them to desktop folder trimmed
        fs::path path = getHomeDirectory() / "Desktop" / "trimmed" / (toString(i) + ".png");
        writeImage( path, tempSurf );
    }
    saveJson();
}

void ImageOptimizerApp::saveJson(){
    //save the offsets for each image into a json file
    JsonTree images = JsonTree::makeArray("images");
    for (int i = 0; i < trimOffsets.size(); i ++) {
        JsonTree curImage = JsonTree::makeObject();
        

        curImage.pushBack(JsonTree("trimLeft", trimOffsets[i].x1));
        curImage.pushBack(JsonTree("trimTop", trimOffsets[i].x2));
        curImage.pushBack(JsonTree("trimRight", trimOffsets[i].y1));
        curImage.pushBack(JsonTree("trimBottom", trimOffsets[i].y2));
        
        images.pushBack(curImage);
    }
    images.write( writeFile( app::getAssetPath("trimOffsets.json")), JsonTree::WriteOptions() );
    
    
}

void ImageOptimizerApp::update()
{
    
}

void ImageOptimizerApp::draw()
{
    gl::clear(Color(0,0,0));
    gl::color(1,1,1);
    gl::draw(mResultTexture);
    gl::color(0,0,1);
    for (Area trimArea : trimOffsets) {
        gl::drawStrokedRect(ci::Rectf(trimArea));
    }
}

CINDER_APP( ImageOptimizerApp, RendererGl, ImageOptimizerApp::prepareSettings )

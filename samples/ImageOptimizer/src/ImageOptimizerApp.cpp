#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Json.h"
#include "cinder/params/Params.h"
#include "TextureSequence.h"





using namespace ci;
using namespace ci::app;
using namespace std;

class ImageOptimizerApp : public App {
public:
    
    static void prepareSettings( Settings *settings);
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void fileDrop( FileDropEvent event ) override;
    void update() override;
    void draw() override;
    
    void loadImages(const fs::path& path );
    void renderSceneToFbo();
    void trim();
    void showMaxTrim();
    void minTrim();
    void showBoth();
    void saveMax();
    void saveMin();
    void saveJson(const fs::path& path);
    void showAnimation();
    void play(const fs::path& path);
    
    bool bTrimmed = false, bMinTrim = false, bMaxTrim = false, bPlay = false;

    Area                mTrimArea;
    Area                mOriOutline;
    gl::TextureRef      mResultTexture;
    gl::FboRef          mFbo;
    
    rph::TextureSequence *mSequence = NULL;//texture sequencer
    
    cinder::params::InterfaceGl     mParams;
    
    std::vector<gl::TextureRef>     mTextures;//texture vector holds all the dropped images for overlaped dipsplay
    std::vector<SurfaceRef>         mSurfaces;//surfaces to hold all the images for trimming
    std::vector<Area>               mTrimOffsets;//the amount of pixels to trim for each image
    std::vector<string>             mFileNames;//vector to store origin file names
    
    
    ci::gl::TextureRef load( const std::string &url, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
    std::vector<ci::gl::TextureRef> loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
};

void ImageOptimizerApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 960, 960 );
    settings->setFrameRate( 60.0 );
}

void ImageOptimizerApp::setup()
{
    mParams = ci::params::InterfaceGl( "Settings", vec2(200,200) );
    mParams.addButton( "ShowMax", bind( &ImageOptimizerApp::showMaxTrim, this ) );
    mParams.addButton( "ShowMin", bind( &ImageOptimizerApp::minTrim, this ) );
    mParams.addButton( "ShowBoth", bind( &ImageOptimizerApp::showBoth, this ) );
    mParams.addButton( "play", bind( &ImageOptimizerApp::showAnimation, this ) );
    mParams.addButton( "SaveMaxTrim", bind( &ImageOptimizerApp::saveMax, this ) );
    mParams.addButton( "SaveMinTrim", bind( &ImageOptimizerApp::saveMin, this ) );
    
    gl::enableAlphaBlending();
}


void ImageOptimizerApp::renderSceneToFbo()
{
    // save size of loaded images
    int width = mTextures[0]->getWidth();
    int height = mTextures[0]->getHeight();
    
    mOriOutline = Area(0,0,width, height);
    
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
    
    //read overlayed texuture from fbo
    mResultTexture = mFbo->getColorTexture();
    trim();
    bMaxTrim = true;
    bMinTrim = true;
    
}

void ImageOptimizerApp::mouseDown( MouseEvent event )
{
}


void ImageOptimizerApp::fileDrop(FileDropEvent event){
    mTrimOffsets.clear();
    mTrimArea = Area(0,0,0,0);
    bTrimmed = false;
    stringstream ss;
    ss << "You dropped files @ " << event.getPos() << " and the files were: " << endl;
    
    for( size_t s = 0; s < event.getNumFiles(); ++s ){
        const fs::path& path = event.getFile( s );
        ss << event.getFile( s ) << endl;
        if (ci::fs::is_directory(path)) {
          
            loadImageDirectory( event.getFile( s ) );
            renderSceneToFbo();// visualize
            play( event.getFile( s ) );
            console() << ss.str() << endl;

        }
        else{
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
    
    mTextures.clear();
    mSurfaces.clear();
    mFileNames.clear();
    
    
    std::vector<ci::gl::TextureRef> textureRefs;
    textureRefs.clear();
    for ( ci::fs::directory_iterator it( dir ); it != ci::fs::directory_iterator(); ++it ){
        if ( ci::fs::is_regular_file( *it ) ){
            // -- Perhaps there is  a better way to ignore hidden files
            ci::fs::path fileExtention = it->path().extension();
            std::string fileName = it->path().filename().string();
                //load acceptable images only
                if( fileExtention == ".png" || fileExtention == ".jpg" || fileExtention == ".jpeg" ){
                    // load dropped images
                    std::string path = dir.string() + "/" + fileName;
                    SurfaceRef surf = Surface::create(loadImage(path));
                    gl::TextureRef tex = gl::Texture::create(*surf);
                    
                    // save them in vector
                    mTextures.push_back(tex);
                    mSurfaces.push_back(surf);
                    
                    //save the names in vector
                    mFileNames.push_back(fileName);
                }
        }
    }
    return textureRefs;
}


//trim to the max amount for each image.
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
        mTrimOffsets.push_back(mTrimArea);

    }
    bTrimmed = true;
}

void ImageOptimizerApp::showMaxTrim(){
    bMinTrim = false;
    if (bTrimmed) {
        bMaxTrim = true;
    }else{
        trim();
        bMaxTrim = true;
    }
}

//trim minimum amount of pixels
void ImageOptimizerApp::minTrim(){
    if (!bTrimmed) {
        trim();
    }
    int tempX1 = mTrimOffsets[0].x1;
    int tempY1 = mTrimOffsets[0].y1;
    int tempX2 = 0;
    int tempY2 = 0;
    
    for (int i =1; i < mTrimOffsets.size(); i++) {
        if (mTrimOffsets[i].x1 < tempX1) {
            tempX1 = mTrimOffsets[i].x1;
        }
        
        if (mTrimOffsets[i].y1 < tempY1) {
            tempY1 = mTrimOffsets[i].y1;
        }
    }
    
    for (int i =0; i < mTrimOffsets.size(); i++) {
        if (mTrimOffsets[i].x2 > tempX2) {
            tempX2 = mTrimOffsets[i].x2;
        }
        
        if (mTrimOffsets[i].y2 > tempY2) {
            tempY2 = mTrimOffsets[i].y2;
        }
    }
    
    mTrimArea = Area(tempX1, tempY1, tempX2, tempY2);
    console()<<"mTrimArea"<<mTrimArea<<endl;
    bMaxTrim = false;
    bMinTrim = true;
    
}

void ImageOptimizerApp::showBoth(){
    bMaxTrim = true;
    bMinTrim = true;
}

void ImageOptimizerApp::saveMax(){
    
//    fs::path path = getSaveFilePath( "", ImageIo::getWriteExtensions());
    fs::path path = getFolderPath();
    if( ! path.empty() ){
        //go thru each surface
        for (int i = 0; i < mSurfaces.size();i++) {
            //only clone the non-transparent area based on the offsets
            Surface tempSurf = mSurfaces[i]->clone(mTrimOffsets[i]);
            //save them to desktop folder trimmed
            ci::fs::path tempPath = path;
            tempPath.append(toString(mFileNames[i]));
            writeImage( tempPath, tempSurf );
            tempPath.clear();
        }
        saveJson(path);
    }
}

void ImageOptimizerApp::saveMin(){
    fs::path path = getFolderPath();
    //go thru each surface
    for (int i = 0; i < mSurfaces.size();i++) {
        //only clone the non-transparent area based on the offsets
        Surface tempSurf = mSurfaces[i]->clone(mTrimArea);
        ci::fs::path tempPath = path;
        tempPath.append(toString(mFileNames[i]));
        //save them to desktop folder trimmed
        writeImage( tempPath, tempSurf);
        tempPath.clear();
    }
}

void ImageOptimizerApp::saveJson(const fs::path& path){
    //save the offsets for each image into a json file
    JsonTree doc = JsonTree::makeObject();
    JsonTree sequence = JsonTree::makeArray("sequence");
//    fs::path jsonPath  = getHomeDirectory() / "Desktop" / "trimmed"/ "max" / "sequence.json";
    fs::path jsonPath  = path;
    jsonPath.append("sequence.json");

    for (int i = 0; i < mTrimOffsets.size(); i ++) {
        JsonTree curImage = JsonTree::makeObject();
        curImage.pushBack(JsonTree("x", mTrimOffsets[i].x1));
        curImage.pushBack(JsonTree("y", mTrimOffsets[i].y1));
        curImage.pushBack(JsonTree("fileName", mFileNames[i] ));
        sequence.pushBack(curImage);
    }
    doc.pushBack(sequence);
    doc.write( jsonPath, JsonTree::WriteOptions());
}

void ImageOptimizerApp::showAnimation(){
    bPlay = true;
}

void ImageOptimizerApp::play(const fs::path& path){
    
    mSequence = new rph::TextureSequence();
    mSequence->setup( loadImageDirectory( path ) );
    mSequence->setLoop(true);
    if (bPlay) {
        mSequence->play();
    }
};

void ImageOptimizerApp::update()
{
    if(mSequence){
        mSequence->update();
    }
}

void ImageOptimizerApp::draw()
{
    gl::clear(Color(0,0,0));
    gl::color(1,1,1);
    gl::draw(mResultTexture);
    if (bMinTrim) {
        gl::color(1, 0, 0);
        gl::drawSolidRect(ci::Rectf(mOriOutline.x1,mOriOutline.y1, mOriOutline.x2, mTrimArea.y1));
        gl::drawSolidRect(ci::Rectf(mOriOutline.x1,mOriOutline.y1, mTrimArea.x1, mOriOutline.y2));
        gl::drawSolidRect(ci::Rectf(mOriOutline.x1,mTrimArea.y2, mOriOutline.x2, mOriOutline.y2));
        gl::drawSolidRect(ci::Rectf(mTrimArea.x2,mOriOutline.y1, mOriOutline.x2, mOriOutline.y2));
    }
    if(bMaxTrim){
        gl::color(0,0,1);
        for (Area trimArea : mTrimOffsets) {
            gl::drawStrokedRect(ci::Rectf(trimArea));
        }
    }
    if(mSequence){
        gl::color(ColorA(1,1,1,1));
        gl::draw( mSequence->getCurrentTexture() );
    }
    gl::color(1, 0, 0);
    gl::drawStrokedRect(ci::Rectf(mOriOutline));
    
    // Draw the interface
    mParams.draw();
}

CINDER_APP( ImageOptimizerApp, RendererGl, ImageOptimizerApp::prepareSettings )

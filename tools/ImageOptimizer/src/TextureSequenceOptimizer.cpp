//
//  TextureSequenceOptimizer.cpp
//
//  Created by Daniel Scheibel on 12/8/15.
//

#include "cinder/Json.h"
#include "TextureSequenceOptimizer.h"

void TextureSequenceOptimizer::setup( const fs::path& path )
{
    loadImageDirectory( path );
    renderImagesToFbo();
}

void TextureSequenceOptimizer::renderImagesToFbo()
{
    // save size of loaded images
    int width = mTextureRefs[0]->getWidth();
    int height = mTextureRefs[0]->getHeight();
    
    mOriOutline = Area(0,0,width, height);
    
    // create fbo
    mFboRef = gl::Fbo::create(width, height, true);
    {
        // bind fbo
        gl::ScopedFramebuffer fbScp( mFboRef );
        
        // set viewport and matrices
        gl::ScopedViewport scpVp( ivec2( 0 ), mFboRef->getSize() );
        gl::ScopedMatrices matricesFbo;
        gl::setMatricesWindow(width, height);
        
        // clear fbo and draw images
        gl::clear(ColorA(0, 0, 0, 0));
        gl::color(1, 1, 1);
        
        // draw images on fbo
        for (gl::TextureRef tex: mTextureRefs) {
            gl::draw(tex);
        }
        
    }
    //read overlayed texuture from fbo
    mResultTextureRef = mFboRef->getColorTexture();
    
    trim();
}

//void TextureSequenceOptimizer::resize(){}

gl::TextureRef TextureSequenceOptimizer::load( const std::string &url, gl::Texture::Format fmt )
{
    try{
        gl::TextureRef t = gl::Texture::create( loadImage( url ), fmt );
        return t;
    }catch(...){}
    app::console() << app::getElapsedSeconds() << ": error loading texture '" << url << "'!" << std::endl;
    return NULL;
}

std::vector<gl::TextureRef> TextureSequenceOptimizer::loadImageDirectory(fs::path dir, gl::Texture::Format fmt)
{
    bTrimmedMax = false;
    
    mTextureRefs.clear();
    mSurfaceRefs.clear();
    mFileNames.clear();
    
    std::vector<gl::TextureRef> textureRefs;
    textureRefs.clear();
    for ( fs::directory_iterator it( dir ); it != fs::directory_iterator(); ++it ){
        if ( fs::is_regular_file( *it ) ){
            // -- Perhaps there is  a better way to ignore hidden files
            fs::path fileExtention = it->path().extension();
            std::string fileName = it->path().filename().string();
            //load acceptable images only
            if( fileExtention == ".png" || fileExtention == ".jpg" || fileExtention == ".jpeg" ){
                // load dropped images
                std::string path = dir.string() + "/" + fileName;
                SurfaceRef surf = Surface::create(loadImage(path));
                gl::TextureRef tex = gl::Texture::create(*surf);
                
                // save them in vector
                mTextureRefs.push_back(tex);
                mSurfaceRefs.push_back(surf);
                
                //save the names in vector
                mFileNames.push_back(fileName);
            }
        }
    }
    return textureRefs;
}

void TextureSequenceOptimizer::trim()
{
    trimMax();
    trimMin();
}

//trim to the max amount for each image.
void TextureSequenceOptimizer::trimMax()
{
    mTrimMaxAreas.clear();
    
    // number of lines to cut
    int trimTop = 0;
    int trimBottom = 0;
    int trimLeft = 0;
    int trimRight = 0;
    
    int i = 0;
    //get the pixels need to be trimmed for each surface with 4 loops
    for (SurfaceRef surf : mSurfaceRefs) {
        
        trimTop = trimBottom = trimLeft = trimRight = 0;
        
        bool stop = false;
        //go thru pixels from top
        int count_y = 0;
        for (int y = 0; y < surf->getHeight(); y++) {
            for (int x =0; x < surf->getWidth(); x++) {
                ColorA c = surf->getPixel( vec2(x, y) );
                //stop at the first non-transparent pixel
                if ( c.a > 0.0f ) {
                    trimTop = y;
                    stop = true;
                    break;
                }
            }
            count_y = y;
            if( stop ) break;
        }
        
        //ci::app::console()<< "count_y: "<< count_y << ", surf->getHeight(): " << surf->getHeight() << std::endl;
        if( count_y == surf->getHeight()-1 && trimTop == 0){
            Area a = Area(trimLeft, trimTop, trimRight, trimBottom);
            ci::app::console() << mFileNames[i] << ", "<< a << " Image TRANSPARENT!" << std::endl;
            ci::app::console()<< "----------------" << std::endl;
            mTrimMaxAreas.push_back( a );
            i++;
            continue;
        }
        
        
        //go thru from bottom
        stop = false;
        for (int y = surf->getHeight() -1; y >=  0; y--) {
            for (int x =0; x < surf->getWidth(); x++) {
                ColorA c = surf->getPixel( vec2(x, y) );
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
                ColorA c = surf->getPixel( vec2(x, y) );
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
                ColorA c = surf->getPixel( vec2(x, y) );
                if ( c.a > 0.0f ) {
                    trimRight = x+1;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }

        Area a = Area(trimLeft, trimTop, trimRight, trimBottom);
        ci::app::console() << mFileNames[i] << ", " << a << std::endl;
        ci::app::console()<< "----------------" << std::endl;
        
        //pixel offsets need to be trimmed for each image
        //push to vector holds all the trim offsets
        mTrimMaxAreas.push_back( a );
        i++;
        
    }
    bTrimmedMax = true;
}

//void TextureSequenceOptimizer::showMaxTrim(){
//    bMinTrim = false;
//    if (bTrimmed) {
//        bMaxTrim = true;
//    }else{
//        trim();
//        bMaxTrim = true;
//    }
//}

//trim minimum amount of pixels
void TextureSequenceOptimizer::trimMin()
{
    if (!bTrimmedMax) {
        trimMax();
    }
    int tempX1 = mTrimMaxAreas[0].x1;
    int tempY1 = mTrimMaxAreas[0].y1;
    int tempX2 = 0;
    int tempY2 = 0;
    
    for (int i =1; i < mTrimMaxAreas.size(); i++) {
        if (mTrimMaxAreas[i].x1 < tempX1) {
            tempX1 = mTrimMaxAreas[i].x1;
        }
        
        if (mTrimMaxAreas[i].y1 < tempY1) {
            tempY1 = mTrimMaxAreas[i].y1;
        }
    }
    
    for (int i =0; i < mTrimMaxAreas.size(); i++) {
        if (mTrimMaxAreas[i].x2 > tempX2) {
            tempX2 = mTrimMaxAreas[i].x2;
        }
        
        if (mTrimMaxAreas[i].y2 > tempY2) {
            tempY2 = mTrimMaxAreas[i].y2;
        }
    }
    
    mTrimMinArea = Area(tempX1, tempY1, tempX2, tempY2);
}

//void TextureSequenceOptimizer::showBoth(){
//    bMaxTrim = true;
//    bMinTrim = true;
//}

void TextureSequenceOptimizer::saveMax( fs::path path )
{
    if ( path == fs::path() ) {
        path = app::App::get()->getFolderPath();
        app::console() << "SAVE MAX: " << path << std::endl;
    }
    if( ! path.empty() ){
        
        JsonTree doc = JsonTree::makeObject();
        JsonTree sequence = JsonTree::makeArray("sequence");
        fs::path jsonPath = path;
        jsonPath.append("sequence.json");
        
        //go thru each surface
        for (int i = 0; i < mSurfaceRefs.size(); i++) {
            
            fs::path tempPath = path;
            
            
            //only clone the non-transparent area based on the offsets
            Surface tempSurf;
            
            JsonTree curImage = JsonTree::makeObject();
            
            if( mTrimMaxAreas[i].calcArea() == 0 ){
                app::console() << " Image is completely transparent: " << mFileNames[i] << std::endl;
                
                tempPath.append("transparent.png");
                
                // check if transparent pixel exists
                if( !fs::exists(tempPath) ){
                    // create transparent png if it doesn't exist
                    tempSurf = mSurfaceRefs[i]->clone( Area(0,0,10,10) );
                    writeImage( tempPath, tempSurf );
                }
                
                // point to transparent image
                curImage.pushBack(JsonTree("x", mTrimMaxAreas[i].x1));
                curImage.pushBack(JsonTree("y", mTrimMaxAreas[i].y1));
                curImage.pushBack(JsonTree("fileName", "transparent.png" ));
                
            }else{
                tempSurf = mSurfaceRefs[i]->clone(mTrimMaxAreas[i]);
                tempPath.append(toString(mFileNames[i]));
                writeImage( tempPath, tempSurf );
                curImage.pushBack(JsonTree("x", mTrimMaxAreas[i].x1));
                curImage.pushBack(JsonTree("y", mTrimMaxAreas[i].y1));
                curImage.pushBack(JsonTree("fileName", mFileNames[i] ));
            }
            sequence.pushBack(curImage);
            
            //app::console() << "saving: " << tempPath << " "<< mTrimMaxAreas[i] << std::endl;
            
            tempPath.clear();
        }
        doc.pushBack(sequence);
        doc.write( jsonPath, JsonTree::WriteOptions());
        //saveJson(path);
    }
}

void TextureSequenceOptimizer::saveMin( fs::path path )
{
    if (path == fs::path()) {
        path = app::App::get()->getFolderPath();
    }
    app::console() << "SAVE MIN: " << path << std::endl;
    //go thru each surface
    for (int i = 0; i < mSurfaceRefs.size();i++) {
        //only clone the non-transparent area based on the offsets
        Surface tempSurf = mSurfaceRefs[i]->clone(mTrimMinArea);
        fs::path tempPath = path;
        tempPath.append(toString(mFileNames[i]));
        //save them to desktop folder trimmed
        writeImage( tempPath, tempSurf);
        tempPath.clear();
    }
}

void TextureSequenceOptimizer::saveJson( const fs::path& path )
{
    //save the offsets for each image into a json file
    JsonTree doc = JsonTree::makeObject();
    JsonTree sequence = JsonTree::makeArray("sequence");
    fs::path jsonPath = path;
    jsonPath.append("sequence.json");
    
    for (int i = 0; i < mTrimMaxAreas.size(); i ++) {
        JsonTree curImage = JsonTree::makeObject();
        curImage.pushBack(JsonTree("x", mTrimMaxAreas[i].x1));
        curImage.pushBack(JsonTree("y", mTrimMaxAreas[i].y1));
        curImage.pushBack(JsonTree("fileName", mFileNames[i] ));
        sequence.pushBack(curImage);
    }
    doc.pushBack(sequence);
    doc.write( jsonPath, JsonTree::WriteOptions());
}

//void TextureSequenceOptimizer::showAnimation(){
//    bPlay = true;
//}

//void TextureSequenceOptimizer::play(const fs::path& path){
//    
//    mSequence = new rph::TextureSequence();
//    mSequence->setup( loadImageDirectory( path ) );
//    mSequence->setLoop(true);
//    if (bPlay) {
//        mSequence->play();
//    }
//};


void TextureSequenceOptimizer::update()
{
//    app::console() << "Loaded amd Trimmed. Surfaces: " << mSurfaces.size() << ", Filenames: " << mFileNames.size() << std::endl;
    
//    if(mSequence){
//        mSequence->update();
//    }
}

void TextureSequenceOptimizer::draw()
{
    gl::color(1,1,1);
    gl::draw(mResultTextureRef);

    gl::color( ColorA(1, 0, 0, 0.5f) );
    gl::drawSolidRect(Rectf(mOriOutline.x1,mOriOutline.y1, mOriOutline.x2, mTrimMinArea.y1));
    gl::drawSolidRect(Rectf(mOriOutline.x1,mOriOutline.y1, mTrimMinArea.x1, mOriOutline.y2));
    gl::drawSolidRect(Rectf(mOriOutline.x1,mTrimMinArea.y2, mOriOutline.x2, mOriOutline.y2));
    gl::drawSolidRect(Rectf(mTrimMinArea.x2,mOriOutline.y1, mOriOutline.x2, mOriOutline.y2));

    gl::color(0,0,1);
    for (Area trimArea : mTrimMaxAreas) {
        gl::drawStrokedRect(Rectf(trimArea));
    }
    
//    if(mSequence){
//        gl::color(ColorA(1,1,1,1));
//        gl::draw( mSequence->getCurrentTexture() );
//    }
    
    gl::color(1, 0, 0);
    gl::drawStrokedRect(Rectf(mOriOutline));
    
}


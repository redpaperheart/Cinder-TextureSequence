#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Json.h"
#include "cinder/params/Params.h"

#include "TextureSequenceOptimizer.h"
#include "SequenceData.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ImageOptimizerApp : public App {
public:
    static void prepareSettings( Settings *settings);
    void setup() override;
    void update() override;
    void draw() override;
    
    void fileDrop( FileDropEvent event ) override;
    void analysePath(const fs::path& path, const fs::path& root);
    int  getTextureSequenceFilesize( const fs::path& path );
    
    TextureSequenceOptimizer    mOptimizr;
    
    Rectf                       mBatchMaxOptRect;
    Rectf                       mBatchMinOptRect;
    std::vector<SequenceData*>  mBatchSequenceDirectories;
    
    cinder::params::InterfaceGl mParams;
};

void ImageOptimizerApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 960, 960 );
    settings->setFrameRate( 60.0 );
}

void ImageOptimizerApp::setup()
{
    float space = 10.0f;
    float width = getWindowWidth()/3.0;
    float height = 100.0f;
    mBatchMaxOptRect = Rectf(space,space,width-10, height-space);
    mBatchMaxOptRect.offset(vec2(0,getWindowHeight()-height));
    mBatchMinOptRect = Rectf(space,space,width-10, height-space);
    mBatchMinOptRect.offset(vec2(width,getWindowHeight()-height));
    
    // set up params
    mParams = ci::params::InterfaceGl( "Settings", vec2(200,200) );
//    mParams.addButton( "ShowMax", bind( &TextureSequenceOptimizer::showMaxTrim, mOptimizr ) );
//    mParams.addButton( "ShowMin", bind( &TextureSequenceOptimizer::showMinTrim, mOptimizr ) );
//    mParams.addButton( "ShowBoth", bind( &TextureSequenceOptimizer::showMinMaxTrim, mOptimizr ) );
//    mParams.addButton( "play", bind( &TextureSequenceOptimizer::showAnimation, mOptimizr ) );
    mParams.addButton( "SaveMaxTrim", std::bind( &TextureSequenceOptimizer::saveMax, &mOptimizr, fs::path() ) );
    mParams.addButton( "SaveMinTrim", std::bind( &TextureSequenceOptimizer::saveMin, &mOptimizr, fs::path() ) );
}

void ImageOptimizerApp::analysePath( const fs::path& path, const fs::path& relativePath = "" )
{
    if (ci::fs::is_directory(path)) {
        
        bool isSequence = false;
        bool dirIncludesFiles = false;
        bool dirIncludesSubDirectories = false;
        int fileCount = 0;
        int fileSizeCount = 0;
        int subDirCount = 0;
        
        for ( fs::directory_iterator it( path ); it != ci::fs::directory_iterator(); ++it ){
            
            if ( fs::is_regular_file( *it ) ) {
                fileSizeCount += fs::file_size(*it);
                dirIncludesFiles = true;
                fileCount++;
            }
            else if( fs::is_directory(*it) ){
                dirIncludesSubDirectories = true;
                subDirCount++;
                analysePath( *it, relativePath/path.stem() ); // call recursively
            }
        }
        if (!dirIncludesSubDirectories && dirIncludesFiles) {
            isSequence = true;
            SequenceData *s = new SequenceData();
            s->absolutePathOrigin = path;
            s->relativePath = relativePath/path.stem();
            s->fileCount = fileCount;
            s->dirSizeOrigin = fileSizeCount;
            mBatchSequenceDirectories.push_back(s);
        }
//        console() << "Files: " << fileCount << " Subdirectories: " << subDirCount << " IsSequence:" << isSequence << endl;
    }
}

int ImageOptimizerApp::getTextureSequenceFilesize( const fs::path& path )
{
    int fileSizeTotal = 0;
    if (ci::fs::is_directory(path)) {
        for ( ci::fs::directory_iterator it( path ); it != ci::fs::directory_iterator(); ++it ){
            if ( ci::fs::is_regular_file( *it ) ){
                fileSizeTotal += fs::file_size(*it);
            }
        }
    }
    return fileSizeTotal;
}

void ImageOptimizerApp::fileDrop(FileDropEvent event){
    
    if( mBatchMaxOptRect.contains( event.getPos() ) ){
        console() << "--------------------------" << endl;
        console() << "MAXIMUM OPTIMIZATION BATCH!" << endl;
        console() << "----" << endl;
        
        for( size_t s = 0; s < event.getNumFiles(); ++s ){
            analysePath( event.getFile( s ) );
        }
        
        int totalFileCount = 0;
        int totalFilesizeOrigin = 0;
        int totalFilesizeOptimized = 0;
        
        //ask for where to store it
        fs::path savePath = getFolderPath();
        for (auto it = mBatchSequenceDirectories.begin(); it < mBatchSequenceDirectories.end(); it++){
            console() << (*it)->toString() << endl;
            // set up / reset each sequence and analyse images
            mOptimizr.setup( (*it)->absolutePathOrigin );
            // save the maximum optimized version, pass in location
            (*it)->absolutePathOptimized = savePath/(*it)->relativePath;
            mOptimizr.saveMax( (*it)->absolutePathOptimized );
            // figure out size and calculate size difference
            (*it)->dirSizeOptimized = getTextureSequenceFilesize( (*it)->absolutePathOptimized );
            totalFilesizeOrigin += (*it)->dirSizeOrigin;
            totalFilesizeOptimized += (*it)->dirSizeOptimized;
            totalFileCount += (*it)->fileCount;
            console() << "Saved '"<< (*it)->relativePath << "' origin size: " <<  (*it)->dirSizeOrigin << " optimized: " << (*it)->dirSizeOptimized << " -> " << ci::toString( (1.0f-float((*it)->dirSizeOptimized) / float((*it)->dirSizeOrigin)) * 100 ) << "% smaller" << endl;
        }
        console() << totalFileCount << " images optimized. Original size: " << totalFilesizeOrigin << " Optimized size: " << totalFilesizeOptimized << " -> " << ci::toString( (1.0f-float(totalFilesizeOptimized)/float(totalFilesizeOrigin)) * 100) << "% smaller" << endl;
        console() << "--------------------------" << endl;
        return;
    }
    
    if( mBatchMinOptRect.contains( event.getPos() ) ){
        console() << "MINIMUM OPTIMZATION BATCH!" << endl;
        return;
    }
    
    for( size_t s = 0; s < event.getNumFiles(); ++s ){
        const fs::path& path = event.getFile( s );
        
        if (ci::fs::is_directory(path)) {
            console() << "Dropped: " << path << endl;
            mOptimizr.setup( path );
        }
        else{
            console() << "!! WARNING :: not a folder: " <<  path << endl;
        }
    }
}

void ImageOptimizerApp::update()
{
    mOptimizr.update();
}

void ImageOptimizerApp::draw()
{
    gl::clear(Color(0,0,0));
    
    mOptimizr.draw();
    
    gl::color(1,1,1);
    gl::drawStrokedRect(mBatchMaxOptRect);
    gl::drawString("Drop here for maximum image optimization.", mBatchMaxOptRect.getUpperLeft()+vec2(10));
    
    gl::drawStrokedRect(mBatchMinOptRect);
    gl::drawString("Drop here for minimum image optimization.", mBatchMinOptRect.getUpperLeft()+vec2(10));
    
    mParams.draw();
}

CINDER_APP( ImageOptimizerApp, RendererGl, ImageOptimizerApp::prepareSettings )

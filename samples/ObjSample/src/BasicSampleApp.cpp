// obj model taken from http://www.sci.utah.edu/~wald/animrep/

#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/Texture.h"
#include "cinder/TriMesh.h"
#include "cinder/ObjLoader.h"
#include "cinder/MayaCamUI.h"

#include "rph/Sequence.h"

using namespace ci;
using namespace ci::app;

class BasicSampleApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
    void keyDown(KeyEvent event);
    void mouseDown(MouseEvent event);
    void mouseDrag(MouseEvent event);
    
    std::vector<TriMeshRef> loadObjFolder(fs::path folderPath);
  
    MayaCamUI mMayaCam;
    rph::Sequence<gl::BatchRef> mSequence;
};

void BasicSampleApp::setup()
{
    // load the obj sequence
    std::vector<TriMeshRef> meshes = loadObjFolder("wooddoll");
    
    // create a batch for every mesh
    gl::GlslProgRef shader = gl::getStockShader(gl::ShaderDef().color());
    std::vector<gl::BatchRef> batches;
    
    for (TriMeshRef mesh : meshes) {
        batches.push_back(gl::Batch::create(*mesh, shader));
    }
    
    // setup texture sequence
    mSequence.setup( batches, 30 );
    mSequence.setLoop(true);
    mSequence.play();
   
    // setup camera
    CameraPersp initialCam;
    initialCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.1, 10000 );
    initialCam.setEyePoint(vec3(0.2, 0.3, 1));
    initialCam.setCenterOfInterestPoint(vec3(0.2, 0.3, 0));
    mMayaCam.setCurrentCam( initialCam );
    
    gl::enableAlphaBlending();
}

void BasicSampleApp::update()
{
    mSequence.update();
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthWrite();
    gl::enableDepthRead();
    
    // draw 3d model
    gl::ScopedMatrices matricesCamera;
    gl::setMatrices( mMayaCam.getCamera() );
    gl::color(Color::hex(0xDEAE86));
    mSequence.getCurrentFrame()->draw();
}

void BasicSampleApp::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown( event.getPos() );
}

void BasicSampleApp::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void BasicSampleApp::keyDown( KeyEvent event )
{
    switch (event.getChar()) {
        case ' ':
            mSequence.togglePlayback();
            break;
    }
}

std::vector<TriMeshRef> BasicSampleApp::loadObjFolder(fs::path folderPath)
{
    std::vector<TriMeshRef> meshes;
    fs::path assetFolderPath = app::getAssetPath(folderPath);
    
    try {
        for (fs::directory_iterator it( assetFolderPath ); it != fs::directory_iterator(); ++it ) {
            
            if (fs::is_regular_file(*it)) {
                std::string fileName = it->path().filename().string();
                
                if (!(fileName.compare(".DS_Store") == 0)) {
                    
                    ObjLoader loader((DataSourceRef) app::loadAsset(folderPath / fileName));
                    TriMeshRef mesh = TriMesh::create(loader);
                    
                    // calc normals if needed
                    if( ! loader.getAvailableAttribs().count( geom::NORMAL ) ) {
                        mesh->recalculateNormals();
                    }
                    
                    meshes.push_back(mesh);
                }
            }
        }
    }
    catch(Exception e) {
        app::console() << "unable to load model folder " << folderPath << std::endl;
        app::console() << e.what() << std::endl;
    }
    return meshes;
}

CINDER_APP_NATIVE( BasicSampleApp, RendererGl )

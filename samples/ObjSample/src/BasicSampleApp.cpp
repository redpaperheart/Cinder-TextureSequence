// obj model taken from http://www.sci.utah.edu/~wald/animrep/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Utilities.h"
#include "cinder/TriMesh.h"
#include "cinder/ObjLoader.h"
#include "cinder/CameraUi.h"

#include "rph/Sequence.h"

using namespace ci;
using namespace ci::app;

class BasicSampleApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
    void keyDown(KeyEvent event) override;
    
    std::vector<TriMeshRef> loadObjFolder(fs::path folderPath);
  
    CameraPersp mCam;
    CameraUi    mCamUi;
    rph::Sequence<gl::BatchRef> mSequence;
};

void BasicSampleApp::setup()
{
    // load the obj sequence
    std::vector<TriMeshRef> meshes = loadObjFolder(getAssetPath("wooddoll"));
    
    // create a batch for every mesh
    gl::GlslProgRef shader = gl::getStockShader(gl::ShaderDef().color().lambert());
    std::vector<gl::BatchRef> batches;
    
    for (TriMeshRef mesh : meshes) {
        batches.push_back(gl::Batch::create(*mesh, shader));
    }
    
    // setup texture sequence
    mSequence.setup( batches, 30 );
    mSequence.setLoop(true);
    mSequence.play();
   
    // setup camera
    mCam = CameraPersp(getWindowWidth(), getWindowHeight(), 40.0f, 0.1, 1000);
    mCam.lookAt(vec3(1), vec3(0.1, 0.3, 0.0));
    mCamUi = CameraUi(&mCam, getWindow());
}

void BasicSampleApp::update()
{
    mSequence.update();
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::ScopedColor color(Color::hex(0xDEAE86));
    gl::ScopedDepth depth(true);
    gl::ScopedMatrices matricesCamera;
    gl::setMatrices( mCam );
    
    mSequence.getCurrentFrame()->draw();
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
    
    try {
        for (fs::directory_iterator it( folderPath ); it != fs::directory_iterator(); ++it ) {
            
            if (fs::is_regular_file(*it)) {
                std::string fileName = it->path().filename().string();
                
                if ( it->path().extension() != ".DS_Store" ) {
                    
                    ObjLoader loader((DataSourceRef) loadFile(it->path()));
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

CINDER_APP( BasicSampleApp, RendererGl )

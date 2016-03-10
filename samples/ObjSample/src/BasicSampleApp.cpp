// obj model taken from http://www.sci.utah.edu/~wald/animrep/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/TriMesh.h"
#include "cinder/ObjLoader.h"
#include "cinder/CameraUi.h"

#include "rph/Sequence.h"
#include "rph/SequenceLoader.h"

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
    std::vector<TriMeshRef> meshes = rph::SequenceLoader::loadObjFolder(getAssetPath("wooddoll"));
    
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
    
    {
        gl::ScopedColor color(Color::hex(0xDEAE86));
        gl::ScopedDepth depth(true);
        gl::ScopedMatrices matricesCamera;
        gl::setMatrices(mCam);
        
        mSequence.getCurrentFrame()->draw();
    }
    
    gl::drawString(std::to_string( (int)getAverageFps() ), vec2(20, 20));
    gl::drawString((mSequence.isLooping() ? "looping" : " not looping"), vec2(20, 40));
}

void BasicSampleApp::keyDown( KeyEvent event )
{
    switch (event.getChar()) {
        case ' ':
            mSequence.togglePlayback();
            break;
    }
}

CINDER_APP( BasicSampleApp, RendererGl )

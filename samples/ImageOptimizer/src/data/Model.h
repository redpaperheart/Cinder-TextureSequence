
#pragma once
#include "cinder/Json.h"
#include "cinder/app/App.h"
#include "cinder/Timeline.h"
#include "cinder/Utilities.h"


#define WIDTH 1380
#define HEIGHT 967

using namespace ci;

class Model {
public:
    static Model* getInstance();
    void setup();
    
    
    // save options
    // remove transparent images at the beginning and end of the sequence.
    bool trimTransparentImages = true;
    
    
    
    // State Management
    enum State{
        STATE_NONE,
        STATE_GLOBAL,
        STATE_REGIONAL,
        STATE_LOCAL,
    };
    
    static std::string stateToString(const State &key)
    {
        std::map<State, std::string> enumToString = {
            { STATE_NONE, "State: None" },
            { STATE_GLOBAL, "State 1: Global" },
            { STATE_REGIONAL, "State 2: Regional" },
            { STATE_LOCAL, "State 3: Local" }
        };
        return enumToString[key];
    }
    
    signals::Signal<void()> signal_curStateChanged;
    void setCurState(const State &stateId);
    State getCurState() const { return mCurState; }
    
    signals::Signal<void ()> signal_nextStateChanged;
    void setNextState(const State &stateId);
    State getNextState() const { return mNextState; }
    
    
    void toggleDebug(){ bDebug = !bDebug; }
    bool isTransitioningStates();
    
    float   mFps = 0;
    bool    bDebug = true;
    bool    bShowCursor = true;
    bool    bShowParams = true;
    bool    bDrawHelpers = true;
    bool    bDrawLayoutOverlay = true;
    bool    bDrawSceneHelpers = false;
    
    // Helper functions
    static std::string toTwoDec( float num );
    static std::string getDateString();
    static std::string getTimeString();
    
private:
    // Singleton
    Model(){}; // Private so that it can  not be called
    Model(Model const&){}; // copy constructor is private
    Model& operator=(Model const&){return *m_pInstance;}; // assignment operator is private
    static Model* m_pInstance;
    
    State mCurState;
    State mNextState;
    
};
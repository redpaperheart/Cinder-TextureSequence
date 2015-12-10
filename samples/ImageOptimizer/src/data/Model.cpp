//
//  Model.cpp
//
//  Created by Daniel Scheibel on 2/9/15.
//

#include "Model.h"


// Global static pointer used to ensure a single instance of the class.
Model* Model::m_pInstance = NULL;
Model* Model::getInstance(){
    if (!m_pInstance){ // Only allow one instance of class to be generated.
        m_pInstance = new Model;
        m_pInstance->setup();
    }
    return m_pInstance;
}

void Model::setup()
{
    mCurState = mNextState = STATE_NONE;    
}


void Model::setCurState(const State &state)
{
    mCurState = state;
    signal_curStateChanged.emit();
}

void Model::setNextState(const State &state)
{
    mNextState = state;
    signal_nextStateChanged.emit();
}

bool Model::isTransitioningStates()
{
    return ( mNextState != STATE_NONE );
}


// HELPERS ------------------------------------------------------------------

std::string Model::getDateString()
{
    time_t t = time(0); // get time now
    struct tm *now = localtime( &t );
    int year = (now->tm_year + 1900);
    int month = (now->tm_mon + 1);
    int day = now->tm_mday;
    return toString(year) + ((month<10)?"0":"") + toString(month) + ((day<10)?"0":"") + toString(day);
}

std::string Model::getTimeString()
{
    time_t t = time(0); // get time now
    struct tm *now = localtime( &t );
    return ((now->tm_hour<10)?"0":"") + toString(now->tm_hour) + ((now->tm_min<10)?"0":"") + toString(now->tm_min) + ((now->tm_sec<10)?"0":"") + toString(now->tm_sec);
}

std::string Model::toTwoDec( float num )
{
    std::ostringstream out;
    out << std::fixed << std::setprecision( 2 ) << num;
    return out.str();
}

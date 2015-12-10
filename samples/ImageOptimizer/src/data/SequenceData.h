//
//  SequenceData.h
//  ImageOptimizer
//
//  Created by Daniel Scheibel on 12/8/15.
//
//

#pragma once

using namespace ci;

class SequenceData {
public:
    SequenceData(){};
    
    fs::path absolutePathOrigin;
    fs::path absolutePathOptimized;
    fs::path relativePath;
    
    int fileCount = 0;
    int dirSizeOrigin = 0;
    int dirSizeOptimized = 0;
    
    std::string toString(){
        return relativePath.string() + " - " + ci::toString(fileCount) + " files, " + ci::toString(dirSizeOrigin) +" bytes" ;
    }
};
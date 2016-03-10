//
//  SequenceLoader.hpp
//  BasicSample
//
//  Created by Adrià Navarro López on 3/10/16.
//
//

// Helper class to load sequences of different filetypes

#pragma once

#include "cinder/ObjLoader.h"
#include "rph/Sequence.h"

using namespace ci;

namespace rph {
    class SequenceLoader {
    public:
        static TextureSequence              makeFromJson(const fs::path &folder, gl::Texture::Format fmt = gl::Texture::Format());
        static TextureSequence              makeFromFolder(const fs::path &folder, gl::Texture::Format fmt = gl::Texture::Format());
        static std::vector<gl::TextureRef>  loadTextureFolder(const fs::path &dir, gl::Texture::Format fmt = gl::Texture::Format());
        static std::vector<TriMeshRef>      loadObjFolder(const fs::path &dir, bool calcNormals = true);
        
    private:
        static bool checkFolderExists(const fs::path &dir);
    };
}
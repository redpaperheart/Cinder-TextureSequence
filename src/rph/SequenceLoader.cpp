//
//  SequenceLoader.cpp
//  BasicSample
//
//  Created by Adrià Navarro López on 3/10/16.
//
//

#include "cinder/Json.h"
#include "SequenceLoader.h"

namespace rph {
    
    std::vector<gl::TextureRef> SequenceLoader::loadTextureFolder(const fs::path &dir, gl::Texture::Format fmt )
    {
        std::vector<gl::TextureRef> textures;
        
        if (!checkFolderExists(dir))
            return textures;
        
        for ( fs::directory_iterator it( dir ); it != fs::directory_iterator(); ++it ){
            if ( fs::is_regular_file( *it ) && it->path().extension() != ".DS_Store" ) {
                try {
                    textures.push_back( gl::Texture::create( loadImage( loadFile( it->path() ) ), fmt ) );
                }
                catch (Exception e) {
                    app::console() << " SequenceLoader Failed to load texture: " << it->path().filename() << std::endl;
                    app::console() << e.what() << std::endl;
                }
            }
        }
        return textures;
    }

    std::vector<TriMeshRef> SequenceLoader::loadObjFolder(const fs::path &dir, bool calcNormals)
    {
        std::vector<TriMeshRef> meshes;
        
        if (!checkFolderExists(dir))
            return meshes;
        
        try {
            for ( fs::directory_iterator it( dir ); it != fs::directory_iterator(); ++it ) {
                fs::path ext = it->path().extension();
                
                if ( fs::is_regular_file(*it) && (ext == ".obj" || ext == ".OBJ") ) {
                    ObjLoader loader((DataSourceRef) loadFile(it->path()));
                    TriMeshRef mesh = TriMesh::create(loader);
                    
                    if( calcNormals && ! loader.getAvailableAttribs().count( geom::NORMAL ) ) {
                        mesh->recalculateNormals();
                    }
                    meshes.push_back(mesh);
                }
            }
        }
        catch(Exception e) {
            app::console() << "SequenceLoader failed to load OBJ folder: " << dir << std::endl;
            app::console() << e.what() << std::endl;
        }
        return meshes;
    }
    
    TextureSequence SequenceLoader::makeFromJson(const fs::path &dir, gl::Texture::Format fmt)
    {
        TextureSequence sequence;
        std::vector<vec3> offsets;
        std::vector<gl::TextureRef> textures;
        
        if (!checkFolderExists(dir))
            return sequence;
        
        try {
            const JsonTree json( loadFile(dir / "sequence.json") );
            
            for ( auto frame : json["sequence"].getChildren() ) {
                vec3 offset;
                if (frame.hasChild("x")) offset.x = frame["x"].getValue<float>();
                if (frame.hasChild("y")) offset.y = frame["y"].getValue<float>();
                if (frame.hasChild("z")) offset.z = frame["z"].getValue<float>();
                offsets.push_back(offset);
                
                if (frame.hasChild("fileName")) {
                    fs::path img = dir / frame["fileName"].getValue<std::string>();
                    textures.push_back( gl::Texture::create( loadImage( loadFile( img ) ), fmt ) );
                }
            }
        } catch (Exception e) {
            app::console() << "SequenceLoader failed to load folder with JSON: " << dir << std::endl;
            app::console() << e.what() << std::endl;
        }
        
        if (offsets.size() != textures.size()) {
            app::console() << "SequenceLoader Warning: different number of offsets and textures" << std::endl;
        }

        sequence.setup( textures, offsets );
        return sequence;
    }
    
    TextureSequence SequenceLoader::makeFromFolder(const fs::path &dir, gl::Texture::Format fmt)
    {
        TextureSequence sequence;
        
        if (!checkFolderExists(dir))
            return sequence;
        
        if (fs::exists( dir / "sequence.json" )) {
            return makeFromJson(dir, fmt);
        }
        
        std::vector<gl::TextureRef> textures = loadTextureFolder(dir);
        sequence.setup(textures);
        return sequence;
    }
    
    bool SequenceLoader::checkFolderExists(const fs::path &dir)
    {
        if ( !fs::exists( dir ) ) {
            app::console() << "SequenceLoader ERROR: ("<< dir << ") Folder does not Exist!" << std::endl;
            return false;
        }
        return true;
    }
};

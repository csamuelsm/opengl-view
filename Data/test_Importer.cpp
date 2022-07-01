#include <string>
#include <iostream>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


bool DoTheImportThing( const std::string& pFile)
{
  // Create an instance of the Importer class
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile( pFile, 
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
	aiProcess_ValidateDataStructure  |
        aiProcess_SortByPType);
  
  // If the import failed, report it
  if( !scene)
  {
	  std::cout << importer.GetErrorString() << std::endl;
    return false;
  }
  
  // Now we can access the file's contents. 
  for (int i = 0; i < scene->mNumMeshes; ++i) {
    std::cout << "processing mesh #" << i << std::endl;
    std::cout << "  (" << scene->mMeshes[i]->mName.C_Str() << ")" << std::endl;
  }

  return true;
}

int main(int argc, char*argv[])
{
  return DoTheImportThing(argv[1]);
}

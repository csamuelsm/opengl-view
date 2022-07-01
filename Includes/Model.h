#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Mesh.h>

// some useful casting functions
static glm::vec4
vec4_cast(const aiVector3D &v) { return glm::vec4(v.x, v.y, v.z, 1.0f); }

static glm::vec4
vec4_cast(const aiColor3D &c, const GLfloat alpha) { return glm::vec4(c.r, c.g, c.b, alpha); }

static glm::vec3
vec3_cast(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }

static glm::uvec3
uvec3_cast(const unsigned int* f) { return glm::uvec3(f[0], f[1], f[2]); }

static glm::vec2
vec2_cast(const aiVector3D &v) { return glm::vec2(v.x, v.y); }

class Model
{
public:
    Model()
    {
        _matrix   = glm::mat4(1.0f);
    }

    Model(std::vector<Vertex> vertices, std::vector<Face> faces,
        Material &material, Texture &texture)
    {
        _mesh.push_back(Mesh(vertices, faces, material, texture));
        _matrix   = glm::mat4(1.0f);
    }

    Model(const char *path)
    {
        load_model(path);
        _matrix = glm::mat4(1.0f);
    }
    
    void render(Shader &shader)
    {
        for (unsigned int i = 0; i < _mesh.size(); i++) {
            // apply global model matrix
            _mesh[i].render(shader, _matrix);
        }
    }
    
    void set_matrix(glm::mat4& m)
    {
        _matrix = m;
    }

    glm::mat4& matrix(){
        return _matrix;
    }
    
    size_t number_of_meshes()
    { return _mesh.size(); }
    
    Mesh& mesh(unsigned int i)
    {
        return _mesh[i];
    }

private:
    void load_model(const char *path)
    {
        // Create an instance of the Importer class
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile( path, 
            aiProcess_Triangulate            |
            aiProcess_JoinIdenticalVertices  |
            aiProcess_ValidateDataStructure  |
            aiProcess_SortByPType);
      
        // If the import failed, report it
        if( !scene ) {
            std::cout << importer.GetErrorString() << std::endl;
            exit(EXIT_FAILURE);
        }
              
        // Get maximum number of vertices and face
        GLuint max_mNumVertices(0), max_mNumFaces(0);
        for (int i = 0; i < scene->mNumMeshes; ++i) {
            if (scene->mMeshes[i]->mNumVertices > max_mNumVertices)
                max_mNumVertices = scene->mMeshes[i]->mNumVertices;
            
            if (scene->mMeshes[i]->mNumFaces > max_mNumFaces)
                max_mNumFaces = scene->mMeshes[i]->mNumFaces;
        }
        
        // Reserve space, for efficiency
        std::vector<Vertex> vertices;
        vertices.reserve(max_mNumVertices);
        
        std::vector<Face> faces;
        faces.reserve(max_mNumFaces);

        // Store mesh data        
        for (int i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* mMesh = scene->mMeshes[i];
            
            //std::cout << "processing mesh #" << i << std::endl;
            //std::cout << "  (" << mMesh->mName.C_Str() << ")" << std::endl;
            
            // Vertex data
            for (int j = 0; j < mMesh->mNumVertices; ++j) {
                glm::vec3 p = vec3_cast(mMesh->mVertices[j]);
                glm::vec3 n = vec3_cast(mMesh->mNormals[j]);
                glm::vec2 t = mMesh->HasTextureCoords(0) ? vec2_cast(mMesh->mTextureCoords[0][j]) : glm::vec2(0.0f);
                //std::cout << "  vertex: " << glm::to_string(p) << std::endl;
                //std::cout << "  normal: " << glm::to_string(n) << std::endl;
                //std::cout << "  texture coords: " << glm::to_string(t) << std::endl;
                Vertex v = {p, n, t};
                vertices.push_back(v);
            }
            //std::cout << std::endl;
            
            // Face data
            for (int j = 0; j < mMesh->mNumFaces; ++j) {
                Face f = { uvec3_cast(mMesh->mFaces[j].mIndices) };
                //std::cout << "  face: " << glm::to_string(f.Index) << std::endl;
                faces.push_back(f);
            }
            //std::cout << std::endl;
            
            // Get mesh material (always suppose Phong illumination model)            
            const aiMaterial* mMaterial = scene->mMaterials[mMesh->mMaterialIndex];
            aiColor3D ambient, diffuse, specular;
            GLfloat shininess, opacity;
            
            mMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
            mMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            mMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
            mMaterial->Get(AI_MATKEY_SHININESS, shininess);
            mMaterial->Get(AI_MATKEY_OPACITY, opacity);
            
            Material material = {
                vec4_cast(ambient, opacity),
                vec4_cast(diffuse, opacity),
                vec4_cast(specular, opacity),
                shininess };
                
            // Diffuse texture map (assuming only one map #0)
            Texture texture = {0, -1, -1, NULL};
            if (mMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString Path;
                if (mMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path) == AI_SUCCESS) {
                    int mWidth, mHeight, mNumChannels;
                    std::string FullPath = std::string("Data/") + Path.data;
                    //std::cout << FullPath.c_str() << std::endl;
                    stbi_set_flip_vertically_on_load(true);
                    
                    texture.data = stbi_load(FullPath.c_str(), &mWidth, &mHeight, &mNumChannels, 0);
                    if (texture.data != NULL) {
                         texture.width  = mWidth;
                         texture.height = mHeight;
                    } else {
                         std::cout << "Failed to load texture" << std::endl;
                         exit(EXIT_FAILURE);
                    }
                }
            }
            
            _mesh.push_back(Mesh(vertices, faces, material, texture));
            
            vertices.clear();
            faces.clear();
            stbi_image_free(texture.data);
        }
    }

    std::vector<Mesh> _mesh;
    glm::mat4         _matrix;
};

#endif // MODEL_H

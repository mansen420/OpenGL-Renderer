#ifndef OBJECT_INTERFACE
#define OBJECT_INTERFACE

#define TINYOBJLOADER_IMPLEMENTATION ;
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <string>

namespace object_3D
{
    using namespace glm;
    struct vertex
    {
        vec3 pos_coords;
        vec3 normal_coords;
        vec2 tex_coords;
    };
    using std::vector;
    using std::string;

    enum texture_type_option
    {
        DIFFUSE,
        SPECULAR,
        OTHER
    };
    struct texture
    {
        unsigned int id = 0;   //id of 0 implies non-existence
        texture_type_option type;
    };
    struct material
    {
        texture diffuse_map;
        texture spec_map;
        material() {spec_map.type=SPECULAR, diffuse_map.type=DIFFUSE;}
    };
    struct mesh
    {
        vector<unsigned int> indices; 
        unsigned int VAO_id;
        mesh(){}
        mesh(vector<unsigned int> indices) : indices(indices){}

        void send_index_data()
        {   //glVertexAttribPointer will only have effect on the data sent by the last call to glBufferData
            //VAOs only store the last call to glVertexAttribPointer
            glGenVertexArrays(1, &VAO_id);
            glBindVertexArray(VAO_id);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal_coords));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tex_coords));
            glEnableVertexAttribArray(2);         

            unsigned int EBO_id;
            glGenBuffers(1, &EBO_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

            glBindVertexArray(0);
        }
    
        //be sure to call send_texture_data() before this.
        void draw()
        {
            glBindVertexArray(VAO_id);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    };
    struct object 
    {
        vector<vertex> vertices;
        vector<mesh> meshes;
        vector<material> materials;
        //sends textures to the specified shader program.
        void activate_texture_ids(const unsigned int &program_id)
        {
            int nr_diffuse = 0, nr_spec = 0;
            int texture_unit_limit;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_unit_limit);
            for (size_t i = 0; i < materials.size(); i++)
            {
                if (nr_diffuse + nr_spec > texture_unit_limit)
                {
                    break;
                }
                if (materials[i].diffuse_map.id > 0)
                {
                    string type = "diffuse_maps";
                    string index = "["+std::to_string(nr_diffuse)+"]";
                    glActiveTexture(GL_TEXTURE0 + nr_diffuse + nr_spec);
                    glBindTexture(GL_TEXTURE_2D, materials[i].diffuse_map.id);
                    glUniform1i(glGetUniformLocation(program_id, (type+index).c_str()), nr_diffuse + nr_spec);
                    glUniform1i(glGetUniformLocation(program_id, "nr_valid_diffuse_maps"), ++nr_diffuse);
                }
                if (materials[i].spec_map.id > 0)
                {
                    string type = "spec_maps";
                    string index = "["+std::to_string(nr_spec)+"]";
                    glActiveTexture(GL_TEXTURE0 + nr_diffuse + nr_spec);
                    glBindTexture(GL_TEXTURE_2D, materials[i].spec_map.id);
                    glUniform1i(glGetUniformLocation(program_id, (type+index).c_str()), nr_diffuse + nr_spec);
                    glUniform1i(glGetUniformLocation(program_id, "nr_valid_diffuse_maps"), ++nr_spec);
                }
            }
        }
        void send_vertex_data()
        {
            unsigned int VBO_id;
            glGenBuffers(1, &VBO_id);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
            glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vertex), &vertices[0], GL_STATIC_DRAW);
        }
        //call this before any calls to draw()
        void send_data()
        {
            send_vertex_data();
            for (size_t i = 0; i < meshes.size(); i++)
            {
                meshes[i].send_index_data();       
            }
        }
        void draw()
        {
            for (auto mesh : meshes)
            {
                mesh.draw();
            }
        }
    };
}
bool gen_texture(const char* file_path, unsigned int &tex_id);

tinyobj::ObjReader obj_parser;
bool read_obj(std::string path, object_3D::object &obj)
{
    if (!obj_parser.ParseFromFile(path, tinyobj::ObjReaderConfig()))
    {
        std::cerr << "loading object failed :\n";
        if (!obj_parser.Error().empty())
        {
            std::cerr << obj_parser.Error() << std::endl;
        }
        return false;
    }
    if (!obj_parser.Warning().empty())
        std::cout << obj_parser.Warning() << std::endl;
    
    tinyobj::attrib_t vertex_attribs = obj_parser.GetAttrib();
    std::vector<tinyobj::shape_t> shapes = obj_parser.GetShapes();
    std::vector<tinyobj::material_t> materials = obj_parser.GetMaterials();

    //build object vertex array
    std::vector<object_3D::vertex> &vertices = obj.vertices;
    vertices = std::vector<object_3D::vertex>(vertex_attribs.vertices.size()/3);
    for(size_t i = 0; i < shapes.size(); i++)
    {   
        std::vector<unsigned int> recorded_indices;
        const std::vector<tinyobj::index_t> &raw_indices = shapes[i].mesh.indices;
        for (size_t j = 0; j < raw_indices.size(); j++)
        {
            const unsigned int vertex_index = raw_indices[j].vertex_index;
            bool new_vertex = true;
            for (size_t k = 0; k < recorded_indices.size(); k++) //is this vertex index recorded?
            {
                if(recorded_indices[k] == vertex_index) //vertex index is recorded
                {
                    new_vertex = false;
                    break;
                }
            }
            if (new_vertex)  //construct and record the vertex
            {
                recorded_indices.push_back(vertex_index);
                object_3D::vertex temp_vert;

                temp_vert.pos_coords.x = vertex_attribs.vertices[3*vertex_index + 0];
                temp_vert.pos_coords.y = vertex_attribs.vertices[3*vertex_index + 1];
                temp_vert.pos_coords.z = vertex_attribs.vertices[3*vertex_index + 2];

                const unsigned int normal_index = raw_indices[j].normal_index;
                if(normal_index >= 0)   //-1 signifies non-available data
                {
                    temp_vert.normal_coords.x = vertex_attribs.normals[3*normal_index + 0];
                    temp_vert.normal_coords.y = vertex_attribs.normals[3*normal_index + 1];
                    temp_vert.normal_coords.z = vertex_attribs.normals[3*normal_index + 2];
                }

                const unsigned int tex_index = raw_indices[j].texcoord_index;
                if (tex_index >= 0)
                {
                    temp_vert.tex_coords.x = vertex_attribs.texcoords[2*tex_index + 0];
                    temp_vert.tex_coords.y = vertex_attribs.texcoords[2*tex_index + 1];
                }
                vertices[vertex_index] = temp_vert;
            }
        }
    }
    
    //get mesh indices
    std::vector<object_3D::mesh> &meshes = obj.meshes;
    meshes = std::vector<object_3D::mesh>(shapes.size());
    for (size_t i = 0; i < shapes.size(); i++)
    {
        const std::vector<tinyobj::index_t> &indices = shapes[i].mesh.indices;
        object_3D::mesh temp_mesh;
        temp_mesh.indices = std::vector<unsigned int>(indices.size());
        for (size_t j = 0; j < indices.size(); j++)
        {
            //note that tinyobject.h takes care of offsetting the obj indices by 1 so we don't have to do it.
            temp_mesh.indices[j] = indices[j].vertex_index;
        }
        meshes[i] = temp_mesh;
    }

    std::vector<object_3D::material> &obj_materials = obj.materials;
    obj_materials = std::vector<object_3D::material>(materials.size());
    for (size_t i = 0; i < materials.size(); i++)
    {
        //within this directory, we will search for the texture names
        const std::string directory = path.substr(0, path.find_last_of("/\\")+1);

        std::string file_name = materials[i].diffuse_texname;
        gen_texture((directory+file_name).c_str(), obj_materials[i].diffuse_map.id);
        
        file_name = materials[i].specular_texname;
        gen_texture((directory+file_name).c_str(), obj_materials[i].spec_map.id);
    }
    return true;
}

//reads texture from file and assigns it to the GL_TEXTURE_2D target with tex_id.
//be warned that this functions expects images with 3 or 4 color channels,
//otherwise, undefined behaviour will occur.
bool gen_texture(const char* file_path, unsigned int &tex_id)
{
    stbi_set_flip_vertically_on_load(false);
    int img_width, img_height, img_nrChannels;
    unsigned char* data = stbi_load(file_path, &img_width, &img_height, &img_nrChannels, 0);
    if (!data)
    {
        std::cout << "reading texture file failed : " << file_path << std::endl;
        return false;
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexImage2D(GL_TEXTURE_2D, 0, img_nrChannels == 3 ? GL_RGB : GL_RGBA, img_width, img_height, 0, img_nrChannels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    
    std::cout << "Loaded texture : " << file_path <<std::endl;
    return true;
}
#endif
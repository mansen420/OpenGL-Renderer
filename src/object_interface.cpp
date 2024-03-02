#include "object_interface.h"
//TODO this whole module needs a refactor
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
    
    bool should_generate_normals = false;

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
                
                //index of -1 signifies non-available data
                const int normal_index = raw_indices[j].normal_index;
                bool should_add_normals = normal_index >= 0 && vertex_attribs.normals.size() > 0;

                if(should_add_normals)
                {
                    temp_vert.normal_coords.x = vertex_attribs.normals[3*normal_index + 0];
                    temp_vert.normal_coords.y = vertex_attribs.normals[3*normal_index + 1];
                    temp_vert.normal_coords.z = vertex_attribs.normals[3*normal_index + 2];
                }
                else    //generate our own normals
                {
                    should_generate_normals = true;
                }

                const int tex_index = raw_indices[j].texcoord_index;
                bool should_add_texture_coords = tex_index >=0 && vertex_attribs.texcoords.size() > 0;
                if (should_add_texture_coords)
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
        meshes[i]              = temp_mesh;      //FIXME expensive copy?
        meshes[i].material_idx = shapes[i].mesh.material_ids;
    }

    //generate our own normals!
    if (should_generate_normals)
    {
        for(size_t i = 0; i < shapes.size(); i++)
        {
            int counter = 0;
            for (size_t j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++)
            {
                tinyobj::index_t first_face_index = shapes[i].mesh.indices[0 + counter];

                //TODO YES IT WORKS!! now just implement weighted per-vertex normals.

                //we multiply the first face index by 3  --or the number of vertices per face--
                //in order to get the index of a single float in the vertex_attribs vector.
                //i.e., vertex indices != vertex_attrib indices
                glm::vec3 A = glm::vec3 
                (vertex_attribs.vertices[3*first_face_index.vertex_index + 0], 
                vertex_attribs.vertices[3*first_face_index.vertex_index + 1],
                vertex_attribs.vertices[3*first_face_index.vertex_index + 2]);

                tinyobj::index_t second_face_index = shapes[i].mesh.indices[1 + counter];
                glm::vec3 B = glm::vec3 
                (vertex_attribs.vertices[3*second_face_index.vertex_index + 0], 
                vertex_attribs.vertices[3*second_face_index.vertex_index + 1], 
                vertex_attribs.vertices[3*second_face_index.vertex_index + 2]);

                tinyobj::index_t third_face_index = shapes[i].mesh.indices[2 + counter];
                glm::vec3 C = glm::vec3 
                (vertex_attribs.vertices[3*third_face_index.vertex_index + 0], 
                vertex_attribs.vertices[3*third_face_index.vertex_index + 1],
                vertex_attribs.vertices[3*third_face_index.vertex_index + 2]);
                
                glm::vec3 AB = B - A;
                glm::vec3 AC = C - A;
                glm::vec3 normal = glm::normalize(glm::cross(AB, AC));

                vertices[first_face_index.vertex_index].normal_coords  = normal;
                vertices[second_face_index.vertex_index].normal_coords = normal;
                vertices[third_face_index.vertex_index].normal_coords  = normal;

                counter += shapes[i].mesh.num_face_vertices[j];
            }
        }
    }
    
    //get textures 
    std::vector<object_3D::material> &obj_materials = obj.materials;
    obj_materials = std::vector<object_3D::material>(materials.size());
    for (size_t i = 0; i < materials.size(); i++)
    {
        //within this directory, we will search for the texture names
        const std::string directory = path.substr(0, path.find_last_of("/\\")+1);
        std::string file_name;
        //TODO the below command is a candidate for a function that takes any texture type as paramater
        file_name = materials[i].diffuse_texname;
        if (!file_name.empty())
            gen_texture((directory+file_name).c_str(), obj_materials[i].diffuse_map.id);
        
        //TODO what if there are no textures? what if there are more textures?

        file_name = materials[i].specular_texname;
        if (!file_name.empty())
            gen_texture((directory+file_name).c_str(), obj_materials[i].spec_map.id);
    }
    return true;
}

//reads texture from file and assigns it to the GL_TEXTURE_2D target with tex_id.
//be warned that this functions expects images with 3 or 4 color channels,
//otherwise, bad things will happen.
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

    glTexImage2D(GL_TEXTURE_2D, 0, img_nrChannels == 3 ? GL_SRGB : GL_SRGB_ALPHA, img_width, img_height, 0, img_nrChannels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    
    std::cout << "Loaded texture : " << file_path <<std::endl;
    return true;
}
bool gen_cubemap(const std::vector<std::string> &file_paths, unsigned int &cubemap_tex_id)
{
    stbi_set_flip_vertically_on_load(false);
    glGenTextures(0, &cubemap_tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_tex_id);
    for (size_t i = 0; i < file_paths.size(); i++)
    {
        int img_width, img_height, img_nrChannels;
        unsigned char* data = stbi_load(file_paths[i].c_str(), &img_width, &img_height,
        &img_nrChannels,0);
        if(!data)
        {
            std::cerr << "reading texture file failed : " << file_paths[i] << std::endl;
            stbi_image_free(data);
            return false;
        }
        std::cout << "Loaded texture : " << file_paths[i] <<std::endl;
        GLenum format = img_nrChannels == 3 ? GL_RGB : GL_RGBA;
        GLenum internal_format = img_nrChannels == 3 ? GL_SRGB : GL_SRGB_ALPHA;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, internal_format, img_width, img_height, 0, format,
        GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        stbi_image_free(data);
    }
    return true;
}
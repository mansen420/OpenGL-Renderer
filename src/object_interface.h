#pragma once

#define VS_TRNSFRM_MDL_NAME "model_transform"

#include "tiny_obj_loader.h"

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
    class drawable
    {
    protected:
        //assigns textures IDs, if any, to samplers. note that most recently assigned value will apply if you leave this function empty.
        virtual void set_samplers(const unsigned int &program_id) const = 0;
        //assigns model transform matrix to vertex shader. note that most recently assigned value will apply if you leave this function empty.
        virtual void send_model_transform(const unsigned int &program_id) const = 0;
        //Binds VAOs, if any, for the draw call.
        virtual void bind_VAO() const = 0;
        //overridable draw command. this function should handle the drawing of your object(s).
        virtual void gl_draw(const unsigned int &program_id) const = 0;
        //sends all uniforms to the shader programs 
        virtual void send_uniforms(const unsigned int &program_id) const final 
        {
            glUseProgram(program_id);
            send_model_transform(program_id);
            set_samplers(program_id);
        }
    public:
        vec3 dimensions;
        vec3     center;
        //generates VAO(s) and/or sends buffer data.
        virtual void send_data() = 0;

        //caller must ensure that send_data() has been called before this. 
        virtual void draw(const unsigned int &program_id) const final 
        {
            send_uniforms(program_id);
            bind_VAO();
            gl_draw(program_id);
            glBindVertexArray(0);
        }
    };

    enum texture_type_option
    {
        DIFFUSE,
        SPECULAR,
        CUBEMAP
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
        texture cube_map;
        material() {spec_map.type=SPECULAR, diffuse_map.type=DIFFUSE, cube_map.type=CUBEMAP;}
    };
    
    class mesh : public drawable
    {
        unsigned int VAO_id;
        virtual void bind_VAO() const override { glBindVertexArray(VAO_id);}
        virtual void set_samplers(const unsigned int &program_id) const override
        {
            //FIXME we assume each face uses the same material index!
            if (material_idx[0] != -1)
            {
                glUniform1i(glGetUniformLocation(program_id, string("material_index").c_str()), material_idx[0]);
            }
        }
        virtual void send_model_transform(const unsigned int &program_id) const override
        {
        }
        virtual void gl_draw(const unsigned int &program_id) const override
        {
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
    public :
        vector<unsigned int> indices;
        vector<int> material_idx;        
        mesh(){}
        virtual void send_data() override
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
    };
    //holds an array of drawable meshes. Initialize with read_obj()
    class object : public drawable
    {   //TODO ensure objects are properly terminated 
        virtual void bind_VAO() const override {}
        virtual void send_model_transform(const unsigned int &program_id) const override
        {
            glUniformMatrix4fv(glGetUniformLocation(program_id, VS_TRNSFRM_MDL_NAME), 1, GL_FALSE, value_ptr(model_transform));
        }
        virtual void set_samplers(const unsigned int &program_id) const override
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
                {   //TODO parameterize these uniform names
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
                    glUniform1i(glGetUniformLocation(program_id, "nr_valid_spec_maps"), ++nr_spec);
                }
            }
        }
        void send_vertex_data() const 
        {
            unsigned int VBO_id;
            glGenBuffers(1, &VBO_id);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
            glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vertex), &vertices[0], GL_STATIC_DRAW);
        }
        virtual void gl_draw(const unsigned int &program_id) const override
        {
            for (auto mesh : meshes)
            {
                mesh.draw(program_id);
            }
        }
    public:
        object(){model_transform = mat4(1.0);}
        vector<vertex> vertices;
        vector<mesh> meshes;
        vector<material> materials;
        mat4 model_transform;
        
        size_t nr_vertices()
        {
            return vertices.size();
        }
        size_t nr_triangles()
        {
            size_t result = 0;
            for (auto m : meshes)
            {
                result += m.indices.size();
            }
            return result/3;
        }
        void calculate_dimensions()
        {
            if (vertices.size() == 0)
            {
                dimensions = vec3(0.0);
                center     = vec3(0.0);
                return;
            }
            vec3 largest_coords = vertices[0].pos_coords;
            vec3 smallest_coords = vertices[0].pos_coords;
            for (size_t i = 1; i < vertices.size(); ++i)
            {
                if (vertices[i].pos_coords.x < smallest_coords.x)
                    smallest_coords.x = vertices[i].pos_coords.x;
                if (vertices[i].pos_coords.y < smallest_coords.y)
                    smallest_coords.y = vertices[i].pos_coords.y;
                if (vertices[i].pos_coords.z < smallest_coords.z)
                    smallest_coords.z = vertices[i].pos_coords.z;

                if (vertices[i].pos_coords.x > largest_coords.x)
                    largest_coords.x = abs(vertices[i].pos_coords.x);
                if (vertices[i].pos_coords.y > largest_coords.y)
                    largest_coords.y = abs(vertices[i].pos_coords.y);
                if (vertices[i].pos_coords.z > largest_coords.z)
                    largest_coords.z = abs(vertices[i].pos_coords.z);
            }  
            dimensions =  largest_coords - smallest_coords;
            center     = (largest_coords + smallest_coords)/dimensions;
        }
        virtual void send_data() override
        {
            send_vertex_data();
            for (size_t i = 0; i < meshes.size(); i++)
            {
                meshes[i].send_data();       
            }
        }
    };
    
    //a drawable object with a manually generated float array of vertices. assumes coordinate order of pos, normals, texture 
    class array_drawable : public drawable
    {
        const float* const vertices;
        unsigned int VAO_id;
        const size_t array_size;
        bool texture, normals;
        virtual void bind_VAO() const override {glBindVertexArray(VAO_id);}
        virtual void gl_draw(const unsigned int &program_id) const override
        {
            const size_t nr_floats = size_t(array_size/sizeof(float));
            const unsigned int nr_floats_per_vertex = pos_dimension + (tex_dimension*texture) + (normals_dimension*normals);
            glDrawArrays(GL_TRIANGLES, 0, nr_floats/nr_floats_per_vertex);
        }
        virtual void set_samplers(const unsigned int &program_id) const 
        {
            if (cubemap)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, textures.cube_map.id);
                glUniform1i(glGetUniformLocation(program_id, "cubemap"), 0);
                return;
            }
            if (textures.diffuse_map.id > 0)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures.diffuse_map.id);
                glUniform1i(glGetUniformLocation(program_id, "diffuse_maps[0]"), 0);
            }
            if (textures.spec_map.id > 0)
            {
                const unsigned int texture_unit = 0 + (textures.diffuse_map.id > 0);
                glActiveTexture(texture_unit);
                glBindTexture(GL_TEXTURE_2D, textures.spec_map.id);
                glUniform1i(glGetUniformLocation(program_id, "spec_maps[0]"), texture_unit);
            }
            else if (textures.diffuse_map.id > 0)
            {
                glUniform1i(glGetUniformLocation(program_id, "spec_maps[0]"), 0);   //specular map points to diffuse map as fallback 
            }
        }
        virtual void send_model_transform(const unsigned int &program_id) const
        {
            glUniformMatrix4fv(glGetUniformLocation(program_id, VS_TRNSFRM_MDL_NAME), 1, GL_FALSE, value_ptr(model_transform));
        }
        public :
        array_drawable(const float* const vertices, const size_t array_byte_size, bool has_normal_coords = true, 
        bool has_texture_coords = true): vertices(vertices), array_size(array_byte_size), texture(has_texture_coords), 
        normals(has_normal_coords), model_transform(mat4(1.0)) {}

        mat4 model_transform;
        material textures;
        unsigned int pos_dimension = 3;
        unsigned int normals_dimension = 3;
        unsigned int tex_dimension = 2;
        bool cubemap = false;
        virtual void send_data() override
        {
            unsigned int VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, array_size, vertices, GL_STATIC_DRAW);

            glGenVertexArrays(1, &VAO_id);
            glBindVertexArray(VAO_id);
            
            const int nr_floats_per_vertex = (pos_dimension + (tex_dimension*texture) + (normals_dimension*normals));
            glVertexAttribPointer(0, pos_dimension, GL_FLOAT, GL_FALSE, nr_floats_per_vertex * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, normals_dimension, GL_FLOAT, GL_FALSE, nr_floats_per_vertex * sizeof(float), (void*)(pos_dimension*sizeof(float)));
            glEnableVertexAttribArray(1*normals);
            glVertexAttribPointer(2, tex_dimension, GL_FLOAT, GL_FALSE, nr_floats_per_vertex * sizeof(float), (void*)((pos_dimension+normals_dimension*normals)*sizeof(float)));
            glEnableVertexAttribArray(2*texture);
        
            glBindVertexArray(0);
        }
    };
}

bool read_obj(std::string path, object_3D::object &obj);
#pragma once
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
uint				LONGITUDE_EDGE = 72;
uint				LATITUDE_EDGE = 36;
uint				NUM_TESS = LONGITUDE_EDGE * LATITUDE_EDGE;
enum class MeshType {
    cube=0, sphere,  pen, cube_obj
};
class Mesh {
public:
    std::string name;
	std::vector<vertex> vertices;
	std::vector<uint> indices;
	uint VAO;
};

std::string filePath = "..\\bin\\models\\";
std::string mat_file = "..\\bin\\models";
std::vector<std::string> filename = { "pen.obj" };
std::string warn;
std::string err;
tinyobj::attrib_t attrib;

inline Mesh* create_cube_mesh()
{
    Mesh* cube = new Mesh();
    cube->name = "cube";
    float positions[] = {
        // front
      -1.0, -1.0,  1.0,
       1.0, -1.0,  1.0,
       1.0,  1.0,  1.0,
      -1.0,  1.0,  1.0,
      // back
      -1.0, -1.0, -1.0,
       1.0, -1.0, -1.0,
       1.0,  1.0, -1.0,
      -1.0,  1.0, -1.0
    };
    for (uint i = 0; i < 8; i++) {
        vec3 _pos = vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
        vec3 _norm = _pos.normalize();
        vec2 _tex = vec2(0.5f, 0.5f);
        cube->vertices.push_back({ _pos, _norm, _tex });
    }
    GLuint cube_elements[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };
    for (size_t i = 0; i < sizeof(cube_elements) / sizeof(GLuint); i++) {
        cube->indices.push_back(cube_elements[i]);
    }
    return cube;
}

inline Mesh* create_sphere_mesh() {
    Mesh* sphere = new Mesh();
    sphere->name = "sphere";
    for (uint i = 0; i <= LATITUDE_EDGE; i++) {
        float t = (PI / (float)LATITUDE_EDGE) * float(i);
        for (uint j = 0; j <= LONGITUDE_EDGE; j++) {
            float p = (PI * 2.0f / (float)LONGITUDE_EDGE) * float(j);
            float x = sin(t) * cos(p); float y = sin(t) * sin(p); float z = cos(t);
            sphere->vertices.push_back({ vec3(x, y, z), vec3(x, y, z), vec2(p / (PI * 2.0f), 1.0f - t / PI) });
        }
    }
	uint pi_lv[6] = { 0,0,1,0,1,1 };
	uint th_lv[6] = { 0,1,0,1,1,0 };
	for (uint i = 0; i < LATITUDE_EDGE; i++) {
		for (uint j = 0; j < LONGITUDE_EDGE; j++) {
			for (uint k = 0; k < 6; k++) sphere->indices.push_back(j + pi_lv[k] + (LONGITUDE_EDGE + 1) * (i + th_lv[k]));
		}
	}
    return sphere;
}

inline Mesh* create_square_mesh() {
    Mesh* square = new Mesh();
    square->name = "square";

    float positions[] = {
    -1.0f,  1.0f, 0.0f, //vertex 1 : Top-left
    1.0f, 1.0f, 0.0f, //vertex 2 : Top-right
    1.0f, -1.0f, 0.0f, //vertex 3 : Bottom-right
    1.0f, -1.0f, 0.0f, //vertex 4 : Bottom-right
    -1.0f, -1.0f, 0.0f, //vertex 5 : Bottom-left
    -1.0f,  1.0f, 0.0f //vertex 6 : Top-left
    };
    for (uint i = 0; i < 6; i++) {
        vec3 _pos = vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
        vec3 _norm = _pos.normalize();
        vec2 _tex = vec2(0.5f, 0.5f);
        square->vertices.push_back({ _pos, _norm, _tex });
    }

    GLuint square_elements[] = {
        0, 1, 2,
        2, 3, 0
    };

    for (size_t i = 0; i < sizeof(square_elements) / sizeof(GLuint); i++) {
        square->indices.push_back(square_elements[i]);
    }
    return square;
}

// primitive가 있으면 여기에 추가해주세요

inline std::vector<Mesh*> generate_mesh(GLuint& vertex_array) {
    std::cout << "Loading meshes ... " << '\n';
    std::vector<Mesh*> v;
    // primitive가 있으면 여기에 추가해주세요
   
    v.push_back(create_cube_mesh());
    v.push_back(create_sphere_mesh());
    for (size_t i = 0; i < v.size(); i++) {
        static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
        static GLuint index_buffer = 0;		// ID holder for index buffer

        // clear and create new buffers
        if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
        if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * v[i]->vertices.size(), &v[i]->vertices[0], GL_STATIC_DRAW);

        // geneation of index buffer
        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * v[i]->indices.size(), &v[i]->indices[0], GL_STATIC_DRAW);
        vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
        std::cout << vertex_array << ": " << v[i]->name << '\n';
        //if(i==2) v[i]->VAO = 2;
        v[i]->VAO = vertex_array;

    }

    for (size_t i = 0; i < filename.size(); i++) {
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string inputfile = filePath + filename[i];
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str(), mat_file.c_str());
        if (!warn.empty()) {
            std::cout << warn << std::endl;
        }

        if (!err.empty()) {
            std::cerr << err << std::endl;
        }

        if (!ret) {
            exit(1);
        }

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            Mesh* mesh = new Mesh();
            static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
            static GLuint index_buffer = 0;		// ID holder for index buffer

            // clear and create new buffers
            if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
            if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = shapes[s].mesh.num_face_vertices[f];
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                    //printf("%d\n", 3 * idx.normal_index);
                    tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
                    tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    vec3 vv = vec3(vx, vy, vz);
                    vec3 nn = vec3();
                    if (3 * idx.normal_index >= 0) nn = vec3(nx, ny, nz);
                    vec2 tt = vec2();
                    if (3 * idx.texcoord_index >= 0) tt = vec2(tx, ty);
                    
                    mesh->vertices.push_back({ vv,nn ,tt });
                    mesh->indices.push_back(index_offset + v);
                    // Optional: vertex colors
                    /*tinyobj::real_t red = attrib.colors[3 * idx.vertex_index + 0];
                    tinyobj::real_t green = attrib.colors[3 * idx.vertex_index + 1];
                    tinyobj::real_t blue = attrib.colors[3 * idx.vertex_index + 2];*/
                }
                index_offset += fv;

                // per-face material
                //shapes[s].mesh.material_ids[f] << '\n';
            }
            glGenBuffers(1, &vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * mesh->vertices.size(), &mesh->vertices[0], GL_STATIC_DRAW);

            // geneation of index buffer
            glGenBuffers(1, &index_buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mesh->indices.size(), &mesh->indices[0], GL_STATIC_DRAW);
            vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
            mesh->name = filename[i];
            std::cout << vertex_array << ": " << mesh->name << '\n';
            mesh->VAO = vertex_array;

            v.push_back(mesh);
        }
    }
    return v;
}
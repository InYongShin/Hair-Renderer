#ifndef HEAD_H
#define HEAD_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;


struct Head {
    GLuint va = 0;
    GLuint vBuf = 0;
    GLuint nBuf = 0;
    GLuint tBuf = 0;
    GLuint eBuf = 0;

    vector<unsigned int> indices = vector<unsigned int>(0);
    vector<unsigned int> vertIndices = vector<unsigned int>(0);
    vector<unsigned int> normIndices = vector<unsigned int>(0);
    vector<glm::vec3> vertices = vector<glm::vec3>(0);
    vector<glm::vec3> tempVertices = vector<glm::vec3>(0);
    vector<glm::vec3> normals = vector<glm::vec3>(0);
    vector<glm::vec3> tempNormals = vector<glm::vec3>(0);

    Head() {};
    Head(const string& path) {
        readHead(path);
    };

    void readHead(const string& path) {
        FILE* file = fopen(path.c_str(), "r");
        if (file == NULL) {
            printf("fail to open the file\n");
            return;
        }

        if(!indices.empty())        indices.clear();
        if(!vertices.empty())       vertices.clear();
        if(!tempVertices.empty())   tempVertices.clear();
        if(!normals.empty())        normals.clear();
        if(!tempNormals.empty())    tempNormals.clear();

        while(1) {
            char line[128];
            int res = fscanf(file, "%s", line);
            if (res == EOF) break;
            if (strcmp(line, "v") == 0) {
                glm::vec3 vertex;
                int count = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                if (count != 3) {
                    printf("fail to read vertex");
                    return;
                }

                // vertex = glm::vec3(vertex.x - 0.5, vertex.y, vertex.z - 0.5);

                tempVertices.push_back(vertex);
            }
            else if (strcmp(line, "vn") == 0) {
                glm::vec3 normal;
                int count = fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                if (count != 3) {
                    printf("fail to read normal");
                    return;
                }
                // tempNormals.push_back(normal);
                normals.push_back(normal);
            }
            else if (strcmp(line, "f") == 0) {
                unsigned int vi[3], ni[3];
                int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2]);

                if (matches != 6) {
                    printf("fail to parse : f\n");
                    return;
                }
                vertIndices.push_back(vi[0]-1);
                vertIndices.push_back(vi[1]-1);
                vertIndices.push_back(vi[2]-1);
                normIndices.push_back(ni[0]-1);
                normIndices.push_back(ni[1]-1);
                normIndices.push_back(ni[2]-1);
            }
            else {
                char stupidBuffer[1000];
                fgets(stupidBuffer, 1000, file);
            }
        }

        // Relocate vertices
        for (unsigned int i = 0; i < vertIndices.size(); i+= 3) {
            vertices.push_back(tempVertices[vertIndices[i]]);
            vertices.push_back(tempVertices[vertIndices[i+1]]);
            vertices.push_back(tempVertices[vertIndices[i+2]]);
        }

        // Set indices
        for (unsigned int i = 0; i < vertIndices.size(); i++) {
            indices.push_back(i);
        }

        /*cout << "Number of Vertex : " << vertices.size() << endl;
        cout << "Number of Normal : " << normals.size() << endl;
        cout << "Number of Indices: " << indices.size() << endl;*/

        printf("success read head file\n");
        fclose(file);
    }
    void create() {
        glGenVertexArrays(1, &va);
        glBindVertexArray(va);

        glGenBuffers(1, &vBuf);
        glBindBuffer(GL_ARRAY_BUFFER, vBuf);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glGenBuffers(1, &nBuf);
        glBindBuffer(GL_ARRAY_BUFFER, nBuf);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, nullptr);

        glGenBuffers(1, &eBuf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBuf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    void render(void) {
        glBindVertexArray(va);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBuf);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};


#endif HEAD_H
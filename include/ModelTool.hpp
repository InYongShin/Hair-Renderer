#ifndef ModelTool_hpp
#define ModelTool_hpp

#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>


struct HairModel {
private:
	std::vector<glm::vec3> lineVertData;
	std::vector<unsigned int> sizes;
	std::vector<glm::vec3> tanData;
	GLuint vertexArrayID = 0;
	GLuint lineVBO = 0;
	GLuint tanVBO = 0;
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	float strandWidth = 1.0f;

public:
	inline float getWidth( void ) const {
		return strandWidth;
	}
	inline void setWidth( float w ) {
		strandWidth = w;
	}
	inline glm::vec4 getColor( void ) const {
		return color;
	}
	inline void setColor( glm::vec4 c ) {
		color = c;
	}
	void readHairFile( const std::string& fileName ) {
		lineVertData.resize(0);
		sizes.resize(0);
		tanData.resize(0);

		unsigned int numOfStrand = 0;
		unsigned int numOfVertices = 0;
		glm::vec3 position(0);

		std::ifstream input(fileName, std::ios::in | std::ios::binary);

		if (input.peek() == std::ifstream::traits_type::eof()) {
			std::cout << "read error" << std::endl;
			exit(1);
		}
		int n = -1;
		input.read((char*)&numOfStrand, sizeof(int));
		for (unsigned int i = 0; i < numOfStrand; i++) {
			n++;
			input.read((char*)&numOfVertices, sizeof(int));
			if (numOfVertices != 1) {
				sizes.push_back(numOfVertices);
				for (unsigned int j = 0; j < numOfVertices; j++) {
					input.read((char*)&position, sizeof(glm::vec3));
					lineVertData.push_back(position);
				}
			}
			else {
				for (unsigned int j = 0; j < numOfVertices; j++) {
					input.read((char*)&position, sizeof(glm::vec3));
				}
			}
		}

		evaluateTangent();

		input.close();
	}
	void readMyHairFile( const std::string& fileName ) {
		lineVertData.resize(0);
		sizes.resize(0);
		tanData.resize(0);

		unsigned int numOfStrand = 0;
		unsigned int numOfVertices = 0;
		glm::vec3 position(0);

		std::ifstream input(fileName, std::ios::in | std::ios::binary);

		if (input.peek() == std::ifstream::traits_type::eof()) {
			std::cout << "read error" << std::endl;
			exit(1);
		}

		input >> numOfStrand;
		for (unsigned int i = 0; i < numOfStrand; i++) {
			input >> numOfVertices;
			sizes.push_back(numOfVertices);
			for (unsigned int j = 0; j < numOfVertices; j++) {
				input >> position.x >> position.y >> position.z;
				position += glm::vec3(0, 1.4f, 0);
				lineVertData.push_back(position);
			}
		}

		evaluateTangent();

		input.close();
	}
	void create( void ) {
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		glGenBuffers(1, &lineVBO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, lineVertData.size() * sizeof(glm::vec3), lineVertData.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &tanVBO);
		glBindBuffer(GL_ARRAY_BUFFER, tanVBO);
		glBufferData(GL_ARRAY_BUFFER, tanData.size() * sizeof(glm::vec3), tanData.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	void evaluateTangent( void ) {
		glm::vec3 tan(0);
		int start = 0;

		for (unsigned int s : sizes) {
			if (s <= 1) continue;
			/*
			glm::mat3 A;
			glm::vec3 bx, by, bz, xx, xy, xz;
			glm::vec3 v;
			int i;

			A = { 0,1,4, 0,1,2, 1,1,1 };
			bx = glm::vec3(lineVertData[start].x, lineVertData[start + 1].x, lineVertData[start + 2].x);
			by = glm::vec3(lineVertData[start].y, lineVertData[start + 1].y, lineVertData[start + 2].y);
			bz = glm::vec3(lineVertData[start].z, lineVertData[start + 1].z, lineVertData[start + 2].z);
			xx = glm::inverse(A) * bx;
			xy = glm::inverse(A) * by;
			xz = glm::inverse(A) * bz;
			v = glm::vec3(xx.y, xy.y, xz.y);
			tanData.push_back(v);

			for (i = 1; i < s - 1; i++) {
				A = { 1,0,1, -1,0,1, 1,1,1 };
				bx = glm::vec3(lineVertData[start + i - 1].x, lineVertData[start + i].x, lineVertData[start + i + 1].x);
				by = glm::vec3(lineVertData[start + i - 1].y, lineVertData[start + i].y, lineVertData[start + i + 1].y);
				bz = glm::vec3(lineVertData[start + i - 1].z, lineVertData[start + i].z, lineVertData[start + i + 1].z);
				xx = glm::inverse(A) * bx;
				xy = glm::inverse(A) * by;
				xz = glm::inverse(A) * bz;
				v = glm::vec3(xx.y, xy.y, xz.y);
				tanData.push_back(v);
			}

			A = { 4,1,0, -2,-1,0, 1,1,1 };
			bx = glm::vec3(lineVertData[start + i - 2].x, lineVertData[start + i - 1].x, lineVertData[start + i].x);
			by = glm::vec3(lineVertData[start + i - 2].y, lineVertData[start + i - 1].y, lineVertData[start + i].y);
			bz = glm::vec3(lineVertData[start + i - 2].z, lineVertData[start + i - 1].z, lineVertData[start + i].z);
			xx = glm::inverse(A) * bx;
			xy = glm::inverse(A) * by;
			xz = glm::inverse(A) * bz;
			v = glm::vec3(xx.y, xy.y, xz.y);
			tanData.push_back(v);

			start += s;
			*/
			
			tan = (lineVertData[start+1] - lineVertData[start]) / 2.0f;  // 1st - 2nd tangent
			tanData.push_back(tan);
			for (int i = 1; i < s-1; i++) {
				tan = (lineVertData[start+i+1] - lineVertData[start+i-1]) / 2.0f;  // middle tangents
				tanData.push_back(tan);
			}
			tanData.push_back(tan);  // last tangent
			start += s;
			
		}
	}
	inline void render( void ) {
		glLineWidth(strandWidth);
		glBindVertexArray(vertexArrayID);
		int start = 0;
		for (unsigned int s : sizes) {
			glDrawArrays(GL_LINE_STRIP, start, s);
			start += s;
		}
		glBindVertexArray(0);
	}

	HairModel(float width, glm::vec4 color) : strandWidth(width), color(color) {}
};


struct Model {
	GLuint va = 0;
	GLuint vBuf = 0;
	GLuint nBuf = 0;
	GLuint tBuf = 0;
	GLuint eBuf = 0;
	unsigned int nFaces = 0;
	
	void create( const std::vector<glm::vec3>& vertices,
		const std::vector<glm::vec3>& normals,
		const std::vector<glm::vec2>& texCoord,
		const std::vector<glm::uvec3>& faces ) {
		if (!va) {
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

			if (texCoord.size() > 0) {
				glGenBuffers(1, &tBuf);
				glBindBuffer(GL_ARRAY_BUFFER, tBuf);
				glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texCoord.size(), texCoord.data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			}

			glGenBuffers(1, &eBuf);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBuf);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::uvec3) * faces.size(), faces.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			nFaces = (unsigned int)(faces.size()) * 3;
		}
	}
	inline void render( void ) {
		glBindVertexArray(va);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBuf);
		glDrawElements(GL_TRIANGLES, nFaces, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	void drawQuad();
};

void Model::drawQuad() {
	Model model;
	if (!model.va) {
		const std::vector<glm::vec3> v = { {-1,1,0}, {-1,-1,0}, {1,-1,0}, {1,1,0} };
		const std::vector<glm::vec3> n = { {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0} };
		const std::vector<glm::vec2> t = { {0,0}, {0,1}, {1,1}, {1,0} };
		const std::vector<glm::uvec3> e = { {0,1,2}, {0,2,3} };
		model.create(v, n, t, e);
	}
	model.render();
}

void drawPlane( const glm::vec3& p ) {
	Model model;
	if (!model.va) {
		//const std::vector<glm::vec3> v = { {-5,1,5}, {-5,1,-5}, {5,1,5}, {5,1,-5} };
		const std::vector<glm::vec3> v = { {p.x+0.1,p.y,p.z+0.1}, {p.x-0.1,p.y,p.z+0.1}, {p.x+0.1,p.y,p.z-0.1}, {p.x-0.1,p.y,p.z-0.1} };
		const std::vector<glm::vec3> n = { {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0} };
		const std::vector<glm::vec2> t = { {0,0}, {0,1}, {1,1}, {1,0} };
		const std::vector<glm::uvec3> e = { {2,1,0}, {3,1,2} };
		model.create(v, n, t, e);
	}
	model.render();
}

void drawPlane( void ) {
	Model model;
	if (!model.va) {
		const std::vector<glm::vec3> v = { {-5,1,5}, {-5,1,-5}, {5,1,5}, {5,1,-5} };
		const std::vector<glm::vec3> n = { {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0} };
		const std::vector<glm::vec2> t = { {0,0}, {0,1}, {1,1}, {1,0} };
		const std::vector<glm::uvec3> e = { {2,1,0}, {3,1,2} };
		model.create(v, n, t, e);
	}
	model.render();
}


#endif /* ModelTool_hpp */
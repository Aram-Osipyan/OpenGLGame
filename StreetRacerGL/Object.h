#pragma once
#include <iostream>
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#define TO_STRING(x) #x

class Object
{
private:
    struct Vector3;
    struct Vertex;
#pragma region SHADER_SOURCES
    const char* _vertexShaderSource = R"(
    #version 330 core


    uniform vec3 position = vec3(0,0,0);
    uniform vec3 rotation;
    uniform vec3 scale = vec3(1,1,1);
    uniform mat4 gWorld;
    uniform mat4 gView;

    layout (location = 0)in vec3 coord;
    layout (location = 1)in vec3 normal;
    layout (location = 2)in vec2 uv;

    out vec2 textcoord;
    out float constant;
    out vec3 Normal;
    out vec3 FragPos; 

    void main() {               
        mat4 rotate_matr = mat4(
                1,  0,                0,            0,
                0,cos(rotation.r),-sin(rotation.r),0,
                0,sin(rotation.r),cos(rotation.r),0,
                0,0,                0,              1)
        *  mat4(
                cos(rotation.y),    0,  sin(rotation.y),    0,
                0,                  1,      0,   0,
                -sin(rotation.y),   0,  cos(rotation.y),    0,
                0,                  0,                 0,  1)
        *  mat4(
                cos(rotation.z),    -sin(rotation.z),  0,    0,
                sin(rotation.z),  cos(rotation.z),  0,   0,
                0,                          0,      1,      0,
                0,              0,                  0,      1);
        
        vec4 pos = vec4(coord,1);
        pos = pos *
            mat4(
                scale.x,0,0,0,
                0,scale.y,0,0,
                0,0,scale.z,0,
                0,0,0,1); // scale
        pos = rotate_matr*pos; // rotate
        pos = pos + vec4(position,0); // position
        
        pos = gWorld*gView * pos;
        
        gl_Position = vec4(pos.rgb, 1.0);
        textcoord = uv;
        constant = 1;
        FragPos = pos.rgb;
        vec4 n = vec4(normal,1)*rotate_matr;
        Normal = n.rgb;
    }
    )";
    const char* _fragShaderSource = R"(
        #version 330 core

        in vec2 textcoord;
        in float constant;
        in vec3 Normal;
        in vec3 FragPos;  

        out vec4 FragColor;

        uniform sampler2D textureData;
        uniform vec3 lightPos;
        uniform vec3 view;

        void main() {
            vec3 lightColor = vec3(1,1,1);
            vec3 objectColor = texture(textureData, textcoord).rgb;
            // ambient
            float ambient = 0.5;
  	
            // diffuse 
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diffuse = max(dot(norm, lightDir), 0.0);
            //vec3 diffuse = diff * lightColor;
    
            vec3 result = (ambient + diffuse) * lightColor * objectColor;
             
            FragColor = vec4(result, 1.0);
        }
        )";
#pragma endregion
    // ID шейдерной программы
    GLuint Program;

    // ID юниформ переменной перемешения
    GLint unifTexture;
    GLint unifLightPos;
    GLint unifViewVec;
    GLint unifgWorld;
    GLint unifgView;

    GLint Unif_position;
    GLint Unif_rotation;
    GLint Unif_scale;
    // ID Vertex Buffer Object
    GLuint VBO;
    GLuint VAO;
    GLuint IBO;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    // ID текстуры
    GLint textureHandle;
    // SFML текстура
    sf::Texture textureData;
    
    void InitShader();
    void ShaderLog(unsigned int shader);
    void CheckOpenGLerror();
    void InitTexture(std::string filename);
    void SetupMesh(std::string filename);
    void ReadVectorsFromFile(std::string filename);
    void ReleaseShader();
    void ReleaseVBO();
    std::vector<float> calculate_lookAt_matrix(Vector3 position, Vector3 target, Vector3 worldUp);
public:
    struct Vector3 {
        float x;
        float y;
        float z;
    public:
        Vector3(float x, float y, float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }
        Vector3() :x(0), y(0), z(0) {};
        static Vector3 normalize(const Vector3& vector) {            
            float znam = vector.x * vector.x + vector.y * vector.y + vector.z* vector.z;
            znam = sqrt(znam);
            Vector3 result(vector.x/znam, vector.y / znam, vector.z / znam);
            return result;
        }
        static Vector3 cross(Vector3 lhs, Vector3 rhs)
        {
            return Vector3(
                lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.x * rhs.y - lhs.y * rhs.x);
        }
        static float dot(Vector3 vec1, Vector3 vec2) {
            return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
        }
    };
    struct Transform
    {
        Vector3 position;
        Vector3 scale;
        Vector3 rotation;
    };
    struct Vertex {
        // position
        Vector3 Position;
        // normal
        Vector3 Normal;
        // texCoords
        Vector3 TexCoords;

    };
    Transform transform;

    Object(std::string texture_filename,std::string mesh_filename){
        InitShader();
        InitTexture(texture_filename);
        SetupMesh(mesh_filename);
        glEnable(GL_DEPTH_TEST);
    }
    virtual void Start() = 0;
    void Draw(const Vector3& lightpos,
        const Vector3& view,
        const std::vector<float>& perspective_projection,
        Vector3 cameraPos,
        Vector3 cameraFront,
        Vector3 cameraUp) {

        std::vector<float> camera_view = 
            calculate_lookAt_matrix(cameraPos,
                Vector3(cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z),
            cameraUp);
        glUseProgram(Program);
        glUniform3f(unifViewVec, view.x, view.y, view.z);

        glProgramUniform3f(Program, Unif_scale, transform.scale.x, transform.scale.y, transform.scale.z);
        glProgramUniform3f(Program, Unif_rotation, transform.rotation.x, transform.rotation.y, transform.rotation.z);
        glProgramUniform3f(Program, Unif_position, transform.position.x, transform.position.y, transform.position.z);
        glProgramUniformMatrix4fv(Program,unifgWorld, 1, GL_FALSE, &perspective_projection[0]);
        glProgramUniformMatrix4fv(Program, unifgView, 1, GL_FALSE, &camera_view[0]);
        glActiveTexture(GL_TEXTURE0);
        // Обёртка SFML на opengl функцией glBindTexture
        sf::Texture::bind(&textureData);
        glUniform1i(unifTexture, 0);
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

    }
    virtual void Update(sf::Event event) = 0;
    void Release() {
        ReleaseShader();
        ReleaseVBO();
    }
};


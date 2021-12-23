#include <iostream>
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "Object.h"
#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define PI 3.1415926f
#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000
#define ToRadian(x) (float)(((x) * PI / 180.0f))

template <typename valType>
std::vector<valType> perspective
(
    valType const& fovy,
    valType const& aspect,
    valType const& zNear,
    valType const& zFar
)
{
    assert(aspect != valType(0));
    assert(zFar != zNear);

    valType const rad = ToRadian(fovy);

    valType tanHalfFovy = tan(rad / valType(2));

    std::vector<valType> Result = std::vector<valType>(16, valType(0));
    Result[0] = valType(1) / (aspect * tanHalfFovy);
    Result[5] = valType(1) / (tanHalfFovy);
    Result[10] = -(zFar + zNear) / (zFar - zNear);
    Result[14] = -valType(1);
    Result[11] = -(valType(2) * zFar * zNear) / (zFar - zNear);
    return Result;
}

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

class Road : public Object {
public:
    Road(float x, float y, float z):Object("assets/road.png", "assets/road.obj") {
        this->transform.position.x = x;
        this->transform.position.y = y;
        this->transform.position.z = z;
    }
    // Inherited via Object
    virtual void Start() override
    {
        this->transform.scale.x = 0.02;
        this->transform.scale.y = 0.02;
        this->transform.scale.z = 0.1;
    }
    virtual void Update(sf::Event event) override
    {
        float init_pos = 20;
        if (this->transform.position.z <= -init_pos)
        {
            this->transform.position.z = init_pos;
        }

        this->transform.position.z -= 0.02;


    }
};

class Grass : public Object {
public:
    Grass(float x, float y, float z) :Object("assets/grass.png", "assets/grass.obj") {
        this->transform.position.x = x;
        this->transform.position.y = y;
        this->transform.position.z = z;
    }
    // Inherited via Object
    virtual void Start() override
    {
        this->transform.scale.x = 0.01;
        this->transform.scale.y = 0.01;
        this->transform.scale.z = 0.1;
        this->transform.rotation.z = 0.2;
        this->transform.position.y = -0.240;
    }
    virtual void Update(sf::Event event) override
    {
        float init_pos = 20;
        if (this->transform.position.z <= -init_pos)
        {
            this->transform.position.z = init_pos;
        }
        this->transform.position.z -= 0.02;
    }
};

class Car : public Object {
private :
    float initYrot = PI;
    void ClampYRotate(float step = 0.005) {
        if (abs(initYrot - this->transform.rotation.y)>step)
        {
            this->transform.rotation.y += sign(initYrot - this->transform.rotation.y) * step;
        }
    }
public:
    Car() : Object("assets/bus2.png", "assets/bus2.obj") {
    }
    // Inherited via Object
    virtual void Start() override
    {
        this->transform.scale.x = 0.03;
        this->transform.scale.y = 0.03;
        this->transform.scale.z = 0.03;
        this->transform.position.y = 0.03;
        this->transform.rotation.y = PI;
    }
    virtual void Update(sf::Event event) override
    {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case (sf::Keyboard::A): {
                    this->transform.position.x += 0.01f;
                    this->transform.rotation.y -= 0.04f;
                    
                };
                break;
                case (sf::Keyboard::D): {
                    this->transform.position.x -= 0.01f;  
                    this->transform.rotation.y += 0.04f;
                }
                break;

                default: break;
            }
        }
        ClampYRotate();
    }
};

int main()
{
#pragma region FONG_SHADERS
    const char* FongVertexShaderSource = R"(
    #version 330 core
    uniform float positX = 0;
    uniform float positY = 0;

    layout (location = 0)in vec3 coord;
    layout (location = 1)in vec3 normal;
    layout (location = 2)in vec2 uv;

    out vec2 textcoord;
    out vec3 Normal;  
    out vec3 FragPos;

    void main() {       
        float x_angle = 0;
        float y_angle = 50;
        
        
        vec3 position = coord * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        vec3 normal2 = normal * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        
        gl_Position = vec4(position, 1.0);
        textcoord = uv;
        Normal = normal2;
        FragPos = position;
    }
    )";

    // Исходный код фрагментного шейдера
    const char* FongFragShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 textcoord;
        in vec3 Normal;  
        in vec3 FragPos;  
  
        uniform vec3 lightPos = vec3(0,0,-1); 
        uniform vec3 view;
        uniform sampler2D textureData;

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
    
            // specular
            float specularStrength = 0.7;
            vec3 viewDir = normalize(view - FragPos);
            vec3 reflectDir = reflect(lightDir, norm);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            float specular = specularStrength * spec ;  
            //float specular = 0;  
            vec3 result = (ambient + diffuse) * lightColor * objectColor+specular*vec3(0,1,0);
            FragColor = vec4(result, 1.0);
        }
        )";
#pragma endregion

#pragma region TOON_SHADERS
    const char* ToonShadingVertexShaderSource = R"(
    #version 330 core
    uniform float positX;
    uniform float positY;

    layout (location = 0)in vec3 coord;
    layout (location = 1)in vec3 normal;
    layout (location = 2)in vec2 uv;

    out vec2 textcoord;
    out vec3 Normal;  
    out vec3 FragPos;

    void main() {       
        float x_angle = 1 + positX;
        float y_angle = 1 + positY;
        
        
        vec3 position = coord * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        vec3 normal2 = normal * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        
        gl_Position = vec4(position, 1.0);
        textcoord = uv;
        Normal = normal2;
        FragPos = position;
    }
    )";

    // Исходный код фрагментного шейдера
    const char* ToonShadingFragShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 textcoord;
        in vec3 Normal;  
        in vec3 FragPos;  
  
        uniform vec3 lightPos = vec3(0,0,-1); 
        uniform vec3 view;
        uniform sampler2D textureData;

        void main() {
            vec3 lightColor = vec3(1,1,1);
           vec3 objectColor = texture(textureData, textcoord).rgb;
            // ambient
            float ambient = 0.5;
  	
            // diffuse 
            vec3 diff ;
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diffuse = dot(norm, lightDir);
            if(diffuse<-0.1 ){
                diff = vec3(1,1,0);
            }
            else       
            if(diffuse>0.1){
            diff = vec3(0.1,0.1,0.1);
            } 
            else{
                diff = vec3(0.5,0.5,0);
            }          
            // specular
            float specularStrength = 0.7;
            vec3 viewDir = normalize(view - FragPos);
            vec3 reflectDir = reflect(lightDir, norm);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            float specular = specularStrength * spec ;  
            //float specular = 0;  
            vec3 result = (ambient ) * lightColor * objectColor+diff;
            FragColor = vec4(result, 1.0);


            /*
            vec4 position = vec4(FragPos,0);
            float _UnlitOutlineThickness = 0.5;
            float _LitOutlineThickness = 0.5;
            vec4 _OutlineColor = vec4(1,1,1,1);
            vec3 normalDirection = normalize(Normal);
            float _Shininess = 0.5;
            vec4 _SpecColor = vec4(0.5,0.5,0.5,1);
            vec4 _LightColor0 = vec4(1,1,0,1);
            vec4 _UnlitColor = vec4(1,0,0,1);
            float _DiffuseThreshold = 0.5;
            vec4 _Color = texture(textureData, textcoord);
            vec3 viewDirection = normalize(view);
            vec3 lightDirection;
            float attenuation;
            vec4 _WorldSpaceLightPos0 = vec4(lightPos,0.0);
            if (0.0 == _WorldSpaceLightPos0.w) // directional light?
            {
               attenuation = 1.0; // no attenuation
               lightDirection = normalize(vec3(_WorldSpaceLightPos0));
            } 
            else // point or spot light
            {
               vec3 vertexToLightSource = vec3(_WorldSpaceLightPos0 - position);
               float distance = length(vertexToLightSource);
               attenuation = 1.0 / distance; // linear attenuation 
               lightDirection = normalize(vertexToLightSource);
            }
            
            // default: unlit 
            vec3 fragmentColor = vec3(_UnlitColor); 
            
            // low priority: diffuse illumination
            if (attenuation * max(0.0, dot(normalDirection, lightDirection))  >= _DiffuseThreshold)
            {
               fragmentColor = vec3(_LightColor0) * vec3(_Color); 
            }
            
            // higher priority: outline
            if (dot(viewDirection, normalDirection) 
               < mix(_UnlitOutlineThickness, _LitOutlineThickness, 
               max(0.0, dot(normalDirection, lightDirection))))
            {
               fragmentColor = 
                  vec3(_LightColor0) * vec3(_OutlineColor); 
            }
            
            // highest priority: highlights
            if (dot(normalDirection, lightDirection) > 0.0 
               // light source on the right side?
               && attenuation *  pow(max(0.0, dot(
               reflect(-lightDirection, normalDirection), 
               viewDirection)), _Shininess) > 0.5) 
               // more than half highlight intensity? 
            {
               fragmentColor = _SpecColor.a 
                  * vec3(_LightColor0) * vec3(_SpecColor)
                  + (1.0 - _SpecColor.a) * fragmentColor;
            }
 
            FragColor = vec4(fragmentColor, 1.0);*/
                
        }
        )";
#pragma endregion

#pragma region BIDIRECTIONAL_LIGHT_SHADERS
    const char* BiDirectionalVertexShaderSource = R"(
    #version 330 core
    uniform float positX;
    uniform float positY;

    layout (location = 0)in vec3 coord;
    layout (location = 1)in vec3 normal;
    layout (location = 2)in vec2 uv;

    out vec2 textcoord;
    out vec3 Normal;  
    out vec3 FragPos;

    void main() {       
        float x_angle = 1 + positX;
        float y_angle = 1 + positY;
        
        
        vec3 position = coord * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        vec3 normal2 = normal * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        
        gl_Position = vec4(position, 1.0);
        textcoord = uv;
        Normal = normal2;
        FragPos = position;
    }
    )";

    // Исходный код фрагментного шейдера
    const char* BiDirectionalFragShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 textcoord;
        in vec3 Normal;  
        in vec3 FragPos;  
  
        uniform vec3 lightPos = vec3(0,0,-1); 
        uniform vec3 view;
        uniform sampler2D textureData;

        void main() {
            vec4 color1 = vec4(0.5, 0.0, 0.0,1);
            vec4 color2 = vec4(0.5, 0.5, 0.0,1);

            vec3 n2 = normalize(Normal);
            vec3 l2 = normalize(view);
            vec4 diff = color1 * max( dot(n2,l2),0) + color2 * max(dot(n2,-l2),0);
            FragColor = diff * texture(textureData, textcoord);

        }
        )";
#pragma endregion //BIDIRECTIONAL_LIGHT_SHADERS

    ////////////////////////////////////////////////
    Object::Vector3 cameraPos = Object::Vector3(-0.35f, 0.45f, -1.39f);
    Object::Vector3 cameraFront = Object::Vector3(2.74f, -2.94f, 38.5997f);
    Object::Vector3 cameraUp = Object::Vector3(0.0f, 1.0f, 0.0f);
    float fov = 45.0f;
    //////////////////////////////////////////////////


    sf::Window window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    // Инициализация glew
    glewInit();
    std::vector<Object*> objects_on_scene;

    float init_pos = 20;
    Road road = Road(0,0,0);
    Road road2 = Road(0,0, init_pos);
    Car car = Car();
    Grass grass = Grass(0,0, 0);
    Grass grass2 = Grass(0,0, init_pos);

    objects_on_scene.push_back(&road);
    objects_on_scene.push_back(&road2);
    objects_on_scene.push_back(&car);
    objects_on_scene.push_back(&grass);
    objects_on_scene.push_back(&grass2);

    for (Object* obj : objects_on_scene) {
        obj->Start();
    }

    Object::Vector3 lightPos(1.2f, 1.0f, 0.0f);
    Object::Vector3 view(0.5f, 0.0f, 0.0f);

    float a = -36.1f;
    float b = 40.0f;
    float aspect = WINDOW_WIDTH/ WINDOW_HEIGHT;
    float r = 0.1;
    std::vector<float> projection = perspective<float>(fov, aspect, a, b);
    while (window.isOpen()) {
        sf::Event event;
        window.pollEvent(event);

        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::Resized) {
            glViewport(0, 0, event.size.width, event.size.height);
        }
        // обработка нажатий клавиш
        else if (event.type == sf::Event::KeyPressed) {
                    
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (Object* obj : objects_on_scene) {
            obj->Draw(lightPos, view, projection, cameraPos, cameraFront, cameraUp);
            obj->Update(event);
        }

        window.display();
    }

    return 0;
}


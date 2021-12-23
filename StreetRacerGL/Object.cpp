#include "Object.h"

void Object::InitShader() {
    // Создаем вершинный шейдер
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    // Передаем исходный код
    glShaderSource(vShader, 1, &_vertexShaderSource, NULL);
    // Компилируем шейдер
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    ShaderLog(vShader);

    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Передаем исходный код
    glShaderSource(fShader, 1, &_fragShaderSource, NULL);
    // Компилируем шейдер
    glCompileShader(fShader);
    std::cout << "fragment shader \n";
    ShaderLog(fShader);

    // Создаем программу и прикрепляем шейдеры к ней
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);

    // Линкуем шейдерную программу
    glLinkProgram(Program);
    // Проверяем статус сборки
    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok)
    {
        std::cout << "error attach shaders \n";
        return;
    }
    Unif_position = glGetUniformLocation(Program, "position");
    if (Unif_position == -1)
    {
        std::cout << "could not bind uniform position" << std::endl;
        return;
    }
    Unif_rotation = glGetUniformLocation(Program, "rotation");
    if (Unif_rotation == -1)
    {
        std::cout << "could not bind uniform rotation" << std::endl;
        return;
    }
    Unif_scale = glGetUniformLocation(Program, "scale");
    if (Unif_scale == -1)
    {
        std::cout << "could not bind uniform scale" << std::endl;
        return;
    }

    unifTexture = glGetUniformLocation(Program, "textureData");
    if (unifTexture == -1)
    {
        std::cout << "could not bind uniform textureData" << std::endl;
        return;
    }
    unifLightPos = glGetUniformLocation(Program, "lightPos");
    if (unifTexture == -1)
    {
        std::cout << "could not bind uniform lightPos" << std::endl;
        return;
    }
    unifViewVec = glGetUniformLocation(Program, "view");
    if (unifTexture == -1)
    {
        std::cout << "could not bind uniform view" << std::endl;
        return;
    }
    unifgWorld = glGetUniformLocation(Program, "gWorld");
    if (unifgWorld == -1)
    {
        std::cout << "could not bind uniform gWorld" << std::endl;
        return;
    }
    unifgView = glGetUniformLocation(Program, "gView");
    if (unifgView == -1) {
        std::cout << "could not bind uniform gView" << std::endl;
        return;
    }
    CheckOpenGLerror();
}

void Object::ShaderLog(unsigned int shader)
{
    int infologLen = 0;
    int charsWritten = 0;
    char* infoLog;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1)
    {
        infoLog = new char[infologLen];
        if (infoLog == NULL)
        {
            std::cout << "ERROR: Could not allocate InfoLog buffer" << std::endl;
            exit(1);
        }
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
        std::cout << "InfoLog: " << infoLog << "\n\n\n";
        delete[] infoLog;
    }
}

void Object::CheckOpenGLerror() {
    GLenum errCode;
    // Коды ошибок можно смотреть тут
    // https://www.khronos.org/opengl/wiki/OpenGL_Error
    if ((errCode = glGetError()) != GL_NO_ERROR)
        std::cout << "OpenGl error!: " << errCode << std::endl;
}

void Object::InitTexture(std::string filename)
{
    // Загружаем текстуру из файла
    if (!textureData.loadFromFile(filename))
    {
        // Не вышло загрузить картинку
        return;
    }
    // Теперь получаем openGL дескриптор текстуры
    textureHandle = textureData.getNativeHandle();
}

void Object::SetupMesh(std::string filename)
{
    ReadVectorsFromFile(filename);
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), &positions[0], GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
    CheckOpenGLerror();
}

void Object::ReadVectorsFromFile(std::string filename) {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector3> uvs;
    std::ifstream infile(filename);
    std::string descr;
    while (infile >> descr) {
        if (descr == "v")
        {
            float x, y, z;
            infile >> x >> y >> z;
            positions.push_back({ x,y,z });
        }
        else if (descr == "vt")
        {
            float x, y;
            infile >> x >> y;
            uvs.push_back({ x,y,0 });
        }
        else if (descr == "vn")
        {
            float x, y, z;
            infile >> x >> y >> z;
            normals.push_back({ x,y,z });
        }
        else if (descr == "f")
        {
            // get triangle
            for (size_t i = 0; i < 3; i++)
            {
                int p_ind;
                char c;
                // read vertex index
                infile >> p_ind;
                infile >> c;
                // read uv index
                int uv_ind;

                infile >> uv_ind;
                infile >> c;
                // read normal index
                int n_ind;

                infile >> n_ind;

                vertices.push_back({ positions[p_ind - 1],normals[n_ind - 1],uvs[uv_ind - 1] });
            }
        }
    }

    indices = std::vector<unsigned int>(vertices.size());

    for (size_t i = 0; i < indices.size(); i++)
    {
        indices[i] = i;
    }
}

void Object::ReleaseShader() {
    // Передавая ноль, мы отключаем шейдрную программу
    glUseProgram(0);
    // Удаляем шейдерную программу
    glDeleteProgram(Program);
}

void Object::ReleaseVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO);
}

std::vector<float> Object::calculate_lookAt_matrix(Vector3 position, Vector3 target, Vector3 worldUp)
{
    // 1. Position = known
    // 2. Calculate cameraDirection
    Vector3 zaxis = Vector3::normalize(Vector3(position.x - target.x, position.y - target.y, position.z - target.z));
    // 3. Get positive right axis vector
    Vector3 xaxis = Vector3::normalize(Vector3::cross(Vector3::normalize(worldUp), zaxis));
    // 4. Calculate camera up vector
    Vector3 yaxis = Vector3::cross(zaxis, xaxis);

    // Create translation and rotation matrix
    // In glm we access elements as mat[col][row] due to column-major layout
    std::vector<float> translation(16); // Identity matrix by default
    translation[3] = -position.x; // Third column, first row
    translation[7] = -position.y;
    translation[11] = -position.z;
    std::vector<float> rotation(16);
    rotation[0] = xaxis.x; // First column, first row
    rotation[1] = xaxis.y;
    rotation[2] = xaxis.z;

    rotation[4] = yaxis.x; // First column, second row
    rotation[5] = yaxis.y;
    rotation[6] = yaxis.z;

    rotation[8] = zaxis.x; // First column, third row
    rotation[9] = zaxis.y;
    rotation[10] = zaxis.z;

    std::vector<float> res(16);
    res[3] = rotation[0] * translation[3] + rotation[1] * translation[7] + rotation[2] * translation[11];
    res[7] = rotation[4] * translation[3] + rotation[5] * translation[7] + rotation[6] * translation[11];
    res[11] = rotation[8] * translation[3] + rotation[9] * translation[7] + rotation[10] * translation[11];
    std::vector<float> viewMatrix = {
        xaxis.x,            yaxis.x,            zaxis.x,       0,
        xaxis.y,            yaxis.y,            zaxis.y,       0,
        xaxis.z,            yaxis.z,            zaxis.z,       0,
        -Vector3::dot(xaxis, position), -Vector3::dot(yaxis, position), -Vector3::dot(zaxis, position),  1
    };
    // Return lookAt matrix as combination of translation and rotation matrix
    return viewMatrix; // Remember to read from right to left (first translation then rotation)
}


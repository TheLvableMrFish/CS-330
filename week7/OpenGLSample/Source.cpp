#include <iostream>         // cout, cerr

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions
#include <camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Light"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vaoTri;         // Handle for the vertex array object
        GLuint vboTri;         // Handle for the vertex buffer object

        GLuint vaoCube;
        GLuint vboCube;

        GLuint nVertices;    // Number of indices of the mesh
        GLuint nVertices2;
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    GLMesh gMesh2;
    GLMesh gMesh3;

    // Texture
    GLuint gTextureId;
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader programs
    GLuint gShapeProgramId;
    GLuint gLampProgramId;
    GLuint gLampProgramId2;

    // camera
    //Camera gCamera(glm::vec3(-9.0f, 9.0f, 12.0f));
    Camera gCamera(glm::vec3(-8.0f, 12.0f, 22.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gShapePosition(0.0f, 0.0f, 0.0f);

    glm::vec3 gShapeScale(2.0f);

    // pyrimid and light color
    //m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    
    glm::vec3 gLightColor(1.0f, 1.0f, 0.6f);
    

    // Light position and scale
    glm::vec3 gLightPosition(-24.5f, 8.5f, -1.0f);

    //glm::vec3 gLightPosition(-1.5f, -0.5f, 3.0f);
    glm::vec3 gLightScale(0.3f);
    glm::vec3 gLightScale2(0.4f);

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model2;
uniform mat4 view2;
uniform mat4 projection2;

uniform vec3 lightPos2;



void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU
out vec4 fragmentColor2;

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightColor2;
uniform vec3 lightPos2;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    vec3 lightDirection2 = normalize(lightPos - vertexFragmentPos);
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    float impact2 = max(dot(norm, lightDirection2), 0.0);
    vec3 diffuse = impact * lightColor; // Generate diffuse light color
    vec3 diffuse2 = impact2 * lightColor2;

    //Calculate Specular lighting*/
    float specularIntensity = 1.0f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    vec3 viewDir2 = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir2 = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir2, reflectDir2), 0.0), highlightSize);
    float specularComponent2 = pow(max(dot(viewDir2, reflectDir2), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;
    vec3 specular2 = specularIntensity * specularComponent2 * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
    vec4 textureColor2 = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
    vec3 phong2 = (ambient + diffuse2 + specular2) * textureColor2.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU

    fragmentColor2 = vec4(phong2, 1.0f);
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 model2;
uniform mat4 view2;
uniform mat4 projection2;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU
    out vec4 fragmentColor2;

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    fragmentColor2 = vec4(1.0f);
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
    UCreateMesh(gMesh2);

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gShapeProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId2))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "../template.png";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gShapeProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gShapeProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);
    UDestroyMesh(gMesh2);

    // Release texture
    UDestroyTexture(gTextureId);

    // Release shader programs
    UDestroyShaderProgram(gShapeProgramId);
    UDestroyShaderProgram(gLampProgramId);
    UDestroyShaderProgram(gLampProgramId2);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static float cameraSpeed = 6.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(D_DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(D_UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(CHANGE_DIMENSION, gDeltaTime);
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{
    const float angularVelocity = glm::radians(45.0f);
   
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the cube VAO (used by tri and lamp)
    glBindVertexArray(gMesh.vaoTri);
    glBindVertexArray(gMesh2.vaoCube);
    // CUBE: draw cube
    //----------------
    // Set the shader to be used
    glUseProgram(gShapeProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gShapePosition) * glm::scale(gShapeScale);


    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();
    glm::mat4 view2 = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 projection2 = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gShapeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gShapeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gShapeProgramId, "projection");

    GLint modelLoc2 = glGetUniformLocation(gShapeProgramId, "model2");
    GLint viewLoc2 = glGetUniformLocation(gShapeProgramId, "view2");
    GLint projLoc2 = glGetUniformLocation(gShapeProgramId, "projection2");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(projection));


    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gShapeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gShapeProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gShapeProgramId, "lightPos");
    GLint lightPositionLoc2 = glGetUniformLocation(gShapeProgramId, "lightPos2");
    GLint objectColorLoc2 = glGetUniformLocation(gShapeProgramId, "objectColor2");
    GLint lightColorLoc2 = glGetUniformLocation(gShapeProgramId, "lightColor2");
    GLint viewPositionLoc = glGetUniformLocation(gShapeProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
   
    

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    

    GLint UVScaleLoc = glGetUniformLocation(gShapeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glDrawArrays(GL_TRIANGLES, 0, gMesh2.nVertices2);
    

    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    
    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);
    

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glDrawArrays(GL_TRIANGLES, 0, gMesh2.nVertices2);

    

    glUseProgram(gLampProgramId2);

 

 

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Objects



    GLfloat vertsAll[] = {
        // Top of gem

         -2.0f, 3.0f, -1.0f,    0.0f, 1.0f, 0.0f,        0.0f, 0.1f,
        0.0f, 3.0f, -2.0f,      0.0f, 1.0f, 0.0f,        0.0f, 0.15f,
        2.0f, 3.0f, -1.0f,      0.0f, 1.0f, 0.0f,        0.05f, 0.1f,
        //right
        -2.0f, 3.0f, -1.0f, 0.0f, 1.0f, 0.0f,       0.0f, 0.1f,
        2.0f, 3.0f, -1.0f, 0.0f, 1.0f, 0.0f,        0.0f, 0.15f,
        2.0f, 3.0f, 1.0f, 0.0f, 1.0f, 0.0f,         0.05f, 0.1f,
        //left
        -2.0f, 3.0f, -1.0f, 0.0f, 1.0f, 0.0f,       0.0f, 0.1f,
        2.0f, 3.0f, 1.0f, 0.0f, 1.0f, 0.0f,         0.0f, 0.15f,
        -2.0f, 3.0f, 1.0f, 0.0f, 1.0f, 0.0f,        0.05f, 0.1f,
        //front
        2.0f, 3.0f, 1.0f, 0.0f, 1.0f, 0.0f,     0.0f, 0.1f,
        -2.0f, 3.0f, 1.0f, 0.0f, 1.0f, 0.0f,    0.0f, 0.15f,
        0.0f, 3.0f, 2.0f, 0.0f, 1.0f, 0.0f,     0.05f, 0.1f,

        // Middle of Gem

            // front right pannel left
        0.0f, 3.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        0.0f, 2.0f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,
        // front right pannel right
        2.0f, 3.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        0.0f, 3.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        // front left pannel right
        0.0f, 2.0f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        0.0f, 3.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        -2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,
        //  front left pannel left
        -2.0f, 3.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        0.0f, 3.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        -2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        //  Side right pannel left
        2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        2.0f, 3.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        2.0f, 3.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,
        // Side right pannel right
        2.0f, 3.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        2.5f, 2.0f, -1.5f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        // Side left pannel left
        -2.5f, 2.0f, 1.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        -2.0f, 3.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        -2.5f, 2.0f, -1.5f, -1.0f, 0.0f, 0.0f, 0.05f, 0.1f,
        // Side left pannel right
        -2.5f, 2.0f, 1.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        -2.0f, 3.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        -2.0f, 3.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        // back right pannel left
        0.0f, 2.0f, -2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        0.0f, 3.0f, -2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        2.5f, 2.0f, -1.5f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,
        // back right pannel right
        2.0f, 3.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        0.0f, 3.0f, -2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        2.5f, 2.0f, -1.5f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        // back left pannel right
        0.0f, 2.0f, -2.5f, -1.0f, 1.0f, -0.5f, 0.0f, 0.1f,
        0.0f, 3.0f, -2.0f, -1.0f, 1.0f, -0.5f, 0.0f, 0.15f,
        -2.5f, 2.0f, -1.5f, -1.0f, 1.0f, -0.5f, 0.05f, 0.1f,
        //  back left pannel left
        -2.0f, 3.0f, -1.0f, -1.0f, 1.0f, -0.5f, 0.0f, 0.1f,
        0.0f, 3.0f, -2.0f, -1.0f, 1.0f, -0.5f, 0.0f, 0.15f,
        -2.5f, 2.0f, -1.5f, -1.0f, 1.0f, -0.5f, 0.05f, 0.1f,

        // Bottom of Gem

            // front right
        0.0f, 2.0f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        // front left
        0.0f, 2.0f, 2.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.1f,
        -2.5f, 2.0f, 1.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.15f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.05f, 0.1f,

        // side right
        2.5f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        2.5f, 2.0f, -1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        // side left
        -2.5f, 2.0f, 1.5f, -1.0f, 0.5f, 0.5f, 0.0f, 0.1f,
        -2.5f, 2.0f, -1.5f, -1.0f, 0.5f, 0.5f, 0.0f, 0.15f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f, 0.05f, 0.1f,

        // back side right
        0.0f, 2.0f, -2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f,
        2.5f, 2.0f, -1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.15f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.05f, 0.1f,

        // back side left
        0.0f, 2.0f, -2.5f, -1.0f, -1.0f, -0.5f, 0.0f, 0.1f,
        -2.5f, 2.0f, -1.5f, -1.0f, -1.0f, -0.5f, 0.0f, 0.15f,
        0.0f, 0.0f, 0.0f, -1.0f, -1.0f, -0.5f, 0.05f, 0.1f,

        // Triangle tri 
        // 
        //Bottom Face        //Negative Y Normal
        -8.0f, 0.0f, -1.0f,   0.0f, -1.0f, 0.0f,         0.1f, 0.15f,
        -8.0f, 0.0f,  0.0f,   0.0f, -1.0f, 0.0f,         0.1f, 0.1f,
        -7.0f, 0.0f,  0.0f,   0.0f, -1.0f, 0.0f,         0.15f, 0.1f,

        -8.0f, 0.0f, -1.0f,  0.0f, -1.0f,  0.0f,        0.1f, 0.15f,
        -7.0f, 0.0f, -1.0f,  0.0f, -1.0f,  0.0f,        0.1f, 0.1f,
        -7.0f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f,        0.15f, 0.1f,

        //Left Face           //Negative X Normal
        -8.0f, 0.0f, -1.0f,   -1.0f, 0.0f, 0.0f,         0.1f, 0.15f,
        -7.5f, 1.0f, -0.5f,   -1.0f, 0.0f, 0.0f,            0.1f, 0.1f,
        -8.0f, 0.0f,  0.0f,   -1.0f, 0.0f, 0.0f,        0.15f, 0.1f,

        //Front Face         //Positive Z Normal
        -8.0f, 0.0f,  0.0f,   0.0f, 0.0f, 0.5f,     0.1f, 0.15f,
        -7.5f, 1.0f, -0.5f,   0.0f, 0.0f, 0.5f,     0.1f, 0.1f,
        -7.0f, 0.0f,  0.0f,   0.0f, 0.0f, 0.5f,     0.15f, 0.1f,

        //Right Face         //Positive X Normal
         -7.0f, 0.0f,  0.0f,   1.0f, 0.0f, 0.0f,    0.1f, 0.15f,
        -7.5f, 1.0f, -0.5f,   1.0f, 0.0f, 0.0f,     0.1f, 0.1f,
         -7.0f, 0.0f, -1.0f,   1.0f, 0.0f, 0.0f,    0.15f, 0.1f,

            //Back Face          //Negative Z Normal  Texture Coords.
             -7.0f, 0.0f, -1.0f,   -1.5f, 0.0f, 0.0f,    0.1f, 0.15f,
            -7.5f, 1.0f, -0.5f,   -1.0f, 0.0f, 0.0f,     0.1f, 0.1f,
            -8.0f, 0.0f, -1.0f,   -1.0f, 0.0f, 0.0f,    0.15f, 0.1f,

        // Semi Sphere

        // front top half
        // 
        // front top face
        3.0f, 4.0f, -5.0f,       0.0f, 0.0f, 1.0f,      0.15f, 0.15f, // top point
        2.25, 3.5f, -3.5f,      0.0f, 0.0f, 1.0f,       0.15f, 0.1f,  // left
        3.75, 3.5f, -3.5f,       0.0f, 0.0f, 1.0f,      0.2f, 0.1f, // right

        // front top left
        3.0f, 4.0f, -5.0f,       -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        1.5f, 3.5f, -5.0f,      -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        2.25, 3.5f, -3.5f,      -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // front top right
        3.0f, 4.0f, -5.0f,       1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        3.75, 3.5f, -3.5f,       1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        4.5f, 3.5f, -5.0f,       1.0f, 0.0f, 0.0f, 0.2f, 0.1f,// right

        // front middle top left
        2.25, 3.5f, -3.5f,      0.0f, 0.1f, 1.0f, 0.15f, 0.15f, // left top
        3.75, 3.5f, -3.5f,       0.0f, 0.1f, 1.0f, 0.15f, 0.1f, // right
        2.0f, 2.0f, -3.0f,     0.0f, 0.1f, 1.0f, 0.2f, 0.1f, // left bottom

        // front middle top right
        3.75, 3.5f, -3.5f,       0.0f, 0.0f, 1.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -3.0f,      0.0f, 0.0f, 1.0f, 0.15f, 0.1f,  // left bottom
        4.0f, 2.0f, -3.0f,       0.0f, 0.0f, 1.0f, 0.2f, 0.1f, // right bottom

        // front left middle top left
        1.5f, 3.5f, -5.0f,      -1.0f, 0.0f, 1.0f, 0.15f, 0.15f, // left top
        2.25, 3.5f, -3.5f,      -1.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right top
        1.0f, 2.0f, -5.0f,      -1.0f, 0.0f, 1.0f, 0.2f, 0.1f, // bottom left

        // front left middle top right
        2.25, 3.5f, -3.5f,      -1.0f, 0.0f, 1.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -3.0f,      -1.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right bottom
        1.0f, 2.0f, -5.0f,      -1.0f, 0.0f, 1.0f, 0.2f, 0.1f, // bottom left

        // front right middle top right
        4.5f, 3.5f, -5.0f,       1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        3.75, 3.5f, -3.5f,       1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left top
        5.0f, 2.0f, -5.0f,       1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // front right middle top left
        3.75, 3.5f, -3.5f,       1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        4.0f, 2.0f, -3.0f,       1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left bottom
        5.0f, 2.0f, -5.0f,       1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // front bottom half
        // 
        // front top face
        3.0f, 0.0f, -5.0f, 0.0f, -1.0f, 0.0f, 0.15f, 0.15f,// top point
        2.25, 0.5f, -3.5f, 0.0f, -1.0f, 0.0f, 0.15f, 0.1f, // left
        3.75, 0.5f, -3.5f, 0.0f, -1.0f, 0.0f, 0.2f, 0.1f, // right

        // front top left
        3.0f, 0.0f, -5.0f, -1.0f, -1.0f, -1.0f, 0.15f, 0.15f, // top point
        1.5f, 0.5f, -5.0f, -1.0f, -1.0f, -1.0f, 0.15f, 0.1f, // left
        2.25, 0.5f, -3.5f, -1.0f, -1.0f, -1.0f, 0.2f, 0.1f, // right

        // front top right
        3.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        3.75, 0.5f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        4.5f, 0.5f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // front middle top left
        2.25, 0.5f, -3.5f, 0.0f, 0.0f, 1.0f, 0.15f, 0.15f, // left top
        3.75, 0.5f, -3.5f, 0.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right
        2.0f, 2.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.1f, // left bottom

        // front middle top right
        3.75, 0.5f, -3.5f, 0.0f, 0.0f, 1.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.15f, 0.1f,  // left bottom
        4.0f, 2.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.1f, // right bottom

        // front left middle top left
        1.5f, 0.5f, -5.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        2.25, 0.5f, -3.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right top
        1.0f, 2.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // front left middle top right
        2.25, 0.5f, -3.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -3.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right bottom
        1.0f, 2.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // front right middle top right
        4.5f, 0.5f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        3.75, 0.5f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left top
        5.0f, 2.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // front right middle top left
        3.75, 0.5f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        4.0f, 2.0f, -3.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left bottom
        5.0f, 2.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f,// right bottom

        // back top half
        // 
        // back top face
        3.0f, 4.0f, -5.0f, 0.0f, 1.0f, -1.0f, 0.15f, 0.15f,// top point
        2.25, 3.5f, -6.5f, 0.0f, 1.0f, -1.0f, 0.15f, 0.1f, // left
        3.75, 3.5f, -6.5f, 0.0f, 1.0f, -1.0f, 0.2f, 0.1f, // right

        // back top left
        3.0f, 4.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        1.5f, 3.5f, -5.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        2.25, 3.5f, -6.5f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back top right
        3.0f, 4.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        3.75, 3.5f, -6.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        4.5f, 3.5f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back middle top left
        2.25, 3.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // left top
        3.75, 3.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // right
        2.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // left bottom

        // back middle top right
        3.75, 3.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f,  // left bottom
        4.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back left middle top left
        1.5f, 3.5f, -5.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        2.25, 3.5f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right top
        1.0f, 2.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back left middle top right
        2.25, 3.5f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -7.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right bottom
        1.0f, 2.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back right middle top right
        4.5f, 3.5f, -5.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // right top
        3.75, 3.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // left top
        5.0f, 2.0f, -5.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back right middle top left
        3.75, 3.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // left top
        4.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // left bottom
        5.0f, 2.0f, -5.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back bottom half
        // 
        // back bottom face
        3.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.15f,// top point
        2.25, 0.5f, -6.5f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        3.75, 0.5f, -6.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back top left
        3.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        1.5f, 0.5f, -5.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        2.25, 0.5f, -6.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back top right
        3.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        3.75, 0.5f, -6.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        4.5f, 0.5f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back middle top left
        2.25, 0.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // left top
        3.75, 0.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // right
        2.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // left bottom

        // back middle top right
        3.75, 0.5f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f,  // left bottom
        4.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back left middle top left
        1.5f, 0.5f, -5.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        2.25, 0.5f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right top
        1.0f, 2.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back left middle top right
        2.25, 0.5f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        2.0f, 2.0f, -7.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right bottom
        1.0f, 2.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back right middle top right
        4.5f, 0.5f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        3.75, 0.5f, -6.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left top
        5.0f, 2.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // back right middle top left
        3.75, 0.5f, -6.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        4.0f, 2.0f, -7.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left bottom
        5.0f, 2.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // Cube
        //
        // bottom left
        -5.0f, 0.0f, -7.0f, 0.0f, -1.0f, 0.0f,       0.1f, 0.05f, // back bottom left
        -5.0f, 0.0f, -5.0f,  0.0f, -1.0f, 0.0f,       0.1f, 0.0f,  // front bottom left
        -3.0f, 0.0f, -5.0f,  0.0f, -1.0f, 0.0f,       0.15f, 0.0f, // front bottom right

        // bottom right
        -5.0f, 0.0f, -7.0f, 0.0f, -1.0f, 0.0f,       0.1f, 0.05f, // back bottom left
        -3.0f, 0.0f, -7.0f, 0.0f, -1.0f, 0.0f,       0.1f, 0.0f,  // back bottom right
        -3.0f, 0.0f, -5.0f, 0.0f, -1.0f, 0.0f,        0.15f, 0.0f, // front bottom right

        // front face left
        -5.0f, 2.0, -5.0f,  0.0f, 0.0f, 1.0f,  0.1f, 0.05f, // top front left
        -5.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.0f,  // bottom front left
        -3.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.0f, 0.15f, 0.0f, // bottom front right

        // front face right
        -5.0f, 2.0, -5.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.05f, // top front left
        -3.0f, 2.0f, -5.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.0f,  // top front right
        -3.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.0f, 0.15f, 0.0f, // bottom front right

        // side left face left
        -5.0f, 0.0, -7.0f, -1.0f, 0.0f, 0.0f, 0.1f, 0.05f, // bottom left side left
        -5.0f, 0.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.1f, 0.0f,  // bottom front left
        -5.0f, 2.0f, -7.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.0f, // top left side left

        // side left face right 
        -5.0f, 2.0, -5.0f, -1.0f, 0.0f, 0.0f, 0.1f, 0.05f, // top front left
        -5.0f, 0.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.1f, 0.0f,  // bottom front left
        -5.0f, 2.0f, -7.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.0f, // top left side left

        // second half of cube
        // top left
        -5.0f, 2.0f, -7.0f, 0.0f, 1.0f, 0.0f, 0.1f, 0.05f, // back bottom left
        -5.0f, 2.0f, -5.0f, 0.0f, 1.0f, 0.0f, 0.1f, 0.0f,  // front bottom left
        -3.0f, 2.0f, -5.0f, 0.0f, 1.0f, 0.0f, 0.15f, 0.0f, // front bottom right

        // top right
        -5.0f, 2.0f, -7.0f, 0.0f, 1.0f, 0.0f, 0.1f, 0.05f, // back bottom left
        -3.0f, 2.0f, -7.0f, 0.0f, 1.0f, 0.0f, 0.1f, 0.0f,  // back bottom right
        -3.0f, 2.0f, -5.0f, 0.0f, 1.0f, 0.0f, 0.15f, 0.0f, // front bottom right

        // back face left
        -5.0f, 2.0, -7.0f, 0.0f, 0.0f, -1.0f, 0.1f, 0.05f, // top front left
        -5.0f, 0.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.1f, 0.0f,  // bottom front left
        -3.0f, 0.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.0f, // bottom front right

        // back face right
        -5.0f, 2.0, -7.0f, 0.0f, 0.0f, -1.0f, 0.1f, 0.05f, // top front left
        -3.0f, 2.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.1f, 0.0f,  // top front right
        -3.0f, 0.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.0f, // bottom front right

        // side righ face left
        -3.0f, 0.0, -7.0f, 1.0f, 0.0f, 0.0f, 0.1f, 0.05f, // bottom left side left
        -3.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.1f, 0.0f,  // bottom front left
        -3.0f, 2.0f, -7.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.0f, // top left side left

        // side right face right 
        -3.0f, 2.0, -5.0f, 1.0f, 0.0f, 0.0f, 0.1f, 0.05f, // top front left
        -3.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.1f, 0.0f,  // bottom front left
        -3.0f, 2.0f, -7.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.0f, // top left side left
        //
        // 
        // 
        // dip sphere thing
        //
        // front top half
        // 
        // front top face
        -10.0f, 1.75f, -5.0f,   0.0f, 1.0f, 1.0f, 0.15f, 0.15f,// top point
        -10.75f, 2.0f, -3.5f,   0.0f, 1.0f, 1.0f, 0.15f, 0.1f, // left
        -9.25f, 2.0f, -3.5f,    0.0f, 1.0f, 1.0f, 0.2f, 0.1f, // right

        // front top left
        -10.0f, 1.75f, -5.0f,   1.0f, 1.0f, 1.0f, 0.15f, 0.15f, // top point
        -11.5f, 2.0f, -5.0f,    1.0f, 1.0f, 1.0f, 0.15f, 0.1f, // left
        -10.75f, 2.0f, -3.5f,   1.0f, 1.0f, 1.0f, 0.2f, 0.1f, // right

        // front top right
        -10.0f, 1.75f, -5.0f,   -1.0f, 1.0f, 1.0f, 0.15f, 0.15f, // top point
        -9.25f, 2.0f, -3.5f,    -1.0f, 1.0f, 1.0f, 0.15f, 0.1f, // left
        -8.5f, 2.0f, -5.0f,     -1.0f, 1.0f, 1.0f, 0.2f, 0.1f, // right

        // front middle top left
        -10.75f, 2.0f, -3.5f, 0.0f, 0.1f, 1.0f, 0.15f, 0.15f, // left top
        -9.25f, 2.0f, -3.5f, 0.0f, 0.1f, 1.0f, 0.15f, 0.1f, // right
        -11.0f, 1.0f, -3.0f, 0.0f, 0.1f, 1.0f, 0.2f, 0.1f, // left bottom

        // front middle top right
        -9.25f, 2.0f, -3.5f, 0.0f, 0.0f, 1.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.15f, 0.1f,  // left bottom
        -9.0f, 1.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.1f, // right bottom

        // front left middle top left
        -11.5f, 2.0f, -5.0f, -1.0f, 0.0f, 1.0f, 0.15f, 0.15f, // left top
        -10.75f, 2.0f, -3.5f, -1.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right top
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 1.0f, 0.2f, 0.1f, // bottom left

        // front left middle top right
        -10.75f, 2.0f, -3.5f, -1.0f, 0.0f, 1.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -3.0f, -1.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right bottom
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 1.0f, 0.2f, 0.1f,// bottom left

        // front right middle top right
        -8.5f, 2.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        -9.25f, 2.0f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left top
        -8.0f, 1.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // front right middle top left
        -9.25, 2.0f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        -9.0f, 1.0f, -3.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left bottom
        -8.0f, 1.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // front bottom half
        // 
        // front top face
        -10.0f, 0.25f, -5.0f, 0.0f, -1.0f, 0.0f, 0.15f, 0.15f,// top point
        -10.75f, 0.0f, -3.5f, 0.0f, -1.0f, 0.0f, 0.15f, 0.1f, // left
        -9.25f, 0.0f, -3.5f, 0.0f, -1.0f, 0.0f, 0.2f, 0.1f, // right

        // front top left
        -10.0f, 0.25f, -5.0f, -1.0f, -1.0f, -1.0f, 0.15f, 0.15f, // top point
        -11.5f, 0.0f, -5.0f, -1.0f, -1.0f, -1.0f, 0.15f, 0.1f, // left
        -10.75f, 0.0f, -3.5f, -1.0f, -1.0f, -1.0f, 0.2f, 0.1f, // right

        // front top right
        -10.0f, 0.25f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        -9.25f, 0.0f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        -8.5f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // front middle top left
        -10.75f, 0.0f, -3.5f, 0.0f, 0.0f, 1.0f, 0.15f, 0.15f, // left top
        -9.25f, 0.0f, -3.5f, 0.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right
        -11.0f, 1.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.1f, // left bottom

        // front middle top right
        -9.25f, 0.0f, -3.5f, 0.0f, 0.0f, 1.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.15f, 0.1f,  // left bottom
        -9.0f, 1.0f, -3.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.1f, // right bottom

        // front left middle top left
        -11.5f, 0.0f, -5.0f, -1.0f, 0.0f, 1.0f, 0.15f, 0.15f, // left top
        -10.75f, 0.0f, -3.5f, -1.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right top
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 1.0f, 0.2f, 0.1f, // bottom left

        // front left middle top right
        -10.75f, 0.0f, -3.5f, -1.0f, 0.0f, 1.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -3.0f, -1.0f, 0.0f, 1.0f, 0.15f, 0.1f, // right bottom
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 1.0f, 0.2f, 0.1f, // bottom left

        // front right middle top right
        -8.5f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        -9.25f, 0.0f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left top
        -8.0f, 1.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // front right middle top left
        -9.25, 0.0f, -3.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        -9.0f, 1.0f, -3.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left bottom
        -8.0f, 1.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // back top half
        // 
        // back top face
        -10.0f, 1.75f, -5.0f,   0.0f, 1.0f, 1.0f, 0.15f, 0.15f,// top point
        -10.75f, 2.0f, -6.5f,   0.0f, 1.0f, 1.0f, 0.15f, 0.1f, // left
        -9.25f, 2.0f, -6.5f,    0.0f, 1.0f, 1.0f, 0.2f, 0.1f, // right

        // back top left
        -10.0f, 1.75f, -5.0f,   1.0f, 1.0f, 1.0f, 0.15f, 0.15f, // top point
        -11.5f, 2.0f, -5.0f,    1.0f, 1.0f, 1.0f, 0.15f, 0.1f, // left
        -10.75f, 2.0f, -6.5f,   1.0f, 1.0f, 1.0f, 0.2f, 0.1f, // right

        // back top right
        -10.0f, 1.75f, -5.0f,   -1.0f, 1.0f, 1.0f, 0.15f, 0.15f, // top point
        -9.25f, 2.0f, -6.5f,    -1.0f, 1.0f, 1.0f, 0.15f, 0.1f, // left
        -8.5f, 2.0f, -5.0f,     -1.0f, 1.0f, 1.0f, 0.2f, 0.1f, // right

        // back middle top left
        -10.75f, 2.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // left top
        -9.25f, 2.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // right
        -11.0f, 1.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // left bottom

        // back middle top right
        -9.25f, 2.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f,  // left bottom
        -9.0f, 1.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back left middle top left
        -11.5f, 2.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f,// left top
        -10.75f, 2.0f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right top
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back left middle top right
        -10.75f, 2.0f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -7.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right bottom
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back right middle top right
        -8.5f, 2.0f, -5.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // right top
        -9.25f, 2.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // left top
        -8.0f, 1.0f, -5.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back right middle top left
        -9.25f, 2.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // left top
        -9.0f, 1.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // left bottom
        -8.0f, 1.0f, -5.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back bottom half
        // 
        // back bottom face
        -10.0f, 0.25f, -5.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.15f,// top point
        -10.75f, 0.0f, -6.5f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        -9.25f, 0.0f, -6.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back top left
        -10.0f, 0.25f, -5.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        -11.5f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        -10.75, 0.0f, -6.5f, 0.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back top right
        -10.0f, 0.25f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // top point
        -9.25f, 0.0f, -6.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left
        -8.5f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right

        // back middle top left
        -10.75f, 0.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // left top
        -9.25f, 0.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f, // right
        -11.0f, 1.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // left bottom

        // back middle top right
        -9.25f, 0.0f, -6.5f, 0.0f, 0.0f, -1.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.15f, 0.1f,  // left bottom
        -9.0f, 1.0f, -7.0f, 0.0f, 0.0f, -1.0f, 0.2f, 0.1f, // right bottom

        // back left middle top left
        -11.5f, 0.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        -10.75f, 0.0f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right top
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back left middle top right
        -10.75f, 0.0f, -6.5f, -1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // right top
        -11.0f, 1.0f, -7.0f, -1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // right bottom
        -12.0f, 1.0f, -5.0f, -1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // bottom left

        // back right middle top right
        -8.5f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f,// right top
        -9.25f, 0.0f, -6.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left top
        -8.0f, 1.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // back right middle top left
        -9.25f, 0.0f, -6.5f, 1.0f, 0.0f, 0.0f, 0.15f, 0.15f, // left top
        -9.0f, 1.0f, -7.0f, 1.0f, 0.0f, 0.0f, 0.15f, 0.1f, // left bottom
        -8.0f, 1.0f, -5.0f, 1.0f, 0.0f, 0.0f, 0.2f, 0.1f, // right bottom

        // Floor
        //
        // 
        -13.0f, 0.0f, -15.0f,   0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
        -13.0f, 0.0f,  5.0f,    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //  bottom left
        7.0f, 0.0f, 5.0f,       0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right

        -13.0f, 0.0f, -15.0f,   0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
        7.0f, 0.0f, -15.0f,     0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  //  top right
        7.0f, 0.0f, 5.0f,       0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
    };
    

  

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(vertsAll) / (sizeof(vertsAll[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));


    glGenVertexArrays(1, &mesh.vaoTri); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vaoTri);

    glGenVertexArrays(1, &mesh.vaoCube); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vaoCube);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vboTri);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboTri); // Activates the buffer
  

    glGenBuffers(1, &mesh.vboCube);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboTri); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsAll), vertsAll, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vaoTri);
    glDeleteBuffers(1, &mesh.vboTri);

    glDeleteVertexArrays(1, &mesh.vaoCube);
    glDeleteBuffers(1, &mesh.vboCube);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}


//
// Created by fight on 2/14/2024.
//
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader/shader.h>
#include <camera.h>
#include <model.h>

#include <iostream>

// I/O
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// utility functions
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);
unsigned int pickAndLoadCubemap(std::string path);
void loadSkybox();
void renderSkybox(Shader skyboxShader, unsigned int cubemap);
void renderScene(Shader &shader, Model objects[]);
std::vector<glm::mat4> getLightSpaceMatrices();
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);
void placeObject(Model object, Shader shader, float x, float y, float z, float r);
void placeRockwallX(Model objects[], Shader shader, float x, float y, float z);
void placeRockwallZ(Model objects[], Shader shader, float x, float y, float z);


// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// framebuffer size
int fb_width;
int fb_height;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float cameraNearPlane = 0.1f;
float cameraFarPlane = 500.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f};

// meshes
unsigned int planeVAO;

// lighting info
// -------------
const glm::vec3 lightDir = glm::normalize(glm::vec3(20.0f, 50, 20.0f));
unsigned int lightFBO;
unsigned int lightDepthMaps;
constexpr unsigned int depthMapResolution = 4096;

// textures
// --------
unsigned int grassTexture;

// cascades
int showCascades = 0;

// skybox
unsigned int currentCubemap;
unsigned int cubemapLand;
unsigned int cubemapOcean;
unsigned int cubemapDawn;
int cubemapIndex = 0;
unsigned int cubemapArray[3];

// pointShadow lightPos
glm::vec3 lightPos(13.7f, 6.0f, -12.2f);


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader shader("res/shader/presentation/casc_shadowMapping_shader.vs", "res/shader/presentation/casc_shadowMapping_shader.fs");
    Shader simpleDepthShader("res/shader/presentation/casc_shadowMapping_depthShader.vs",
                             "res/shader/presentation/casc_shadowMapping_depthShader.fs",
                             "res/shader/presentation/casc_shadowMapping_depthShader.gs");
    Shader skyboxShader("res/shader/presentation/skybox.vs", "res/shader/presentation/skybox.fs");
    Shader pointDepthShader("res/shader/presentation/pointShadows_depthShader.vs",
                            "res/shader/presentation/pointShadows_depthShader.fs",
                            "res/shader/presentation/pointShadows_depthShader.gs");

    // -----------
    Model tree1("res/texture/presentation/natureModels/tree1.obj");
    Model tree2("res/texture/presentation/natureModels/tree2.obj");
    Model grass("res/texture/presentation/natureModels/grass.obj");
    Model rockLow("res/texture/presentation/natureModels/rock1.obj");
    Model rockMid("res/texture/presentation/natureModels/rock2.obj");
    Model rockHigh("res/texture/presentation/natureModels/rock3.obj");
    Model bench("res/texture/presentation/bench/bench.obj");
    Model house("res/texture/presentation/house/house.obj");
    Model light("res/texture/presentation/light/light.obj");
    Model table("res/texture/presentation/table/table.obj");
    Model objects[] = {tree1, tree2, grass, rockLow, rockMid, rockHigh, bench, house, light, table};

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float planeVertices[] = {
            // positions            // normals         // texcoords
            25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
            -25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
            -25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,

            25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
            25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f,
            -25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f
    };
    // winding order reversed, because of culling, here are old floor vertices:
//    float planeVertices[] = {
//            // positions            // normals         // texcoords
//            25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
//            -25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
//            -25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
//
//            25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
//            -25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
//            25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
//    };
    // plane VAO
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // load skybox
    loadSkybox();

    // load textures
    // -------------
    grassTexture = loadTexture("res/texture/presentation/MCgrass.jpg");

    // load cubemap
    // -------------
    cubemapLand = pickAndLoadCubemap("res/texture/presentation/skybox/classicLand");
    cubemapOcean = pickAndLoadCubemap("res/texture/presentation/skybox/clearOcean");
    cubemapDawn = pickAndLoadCubemap("res/texture/presentation/skybox/dawn");
    cubemapArray[0] = cubemapLand;
    cubemapArray[1] = cubemapOcean;
    cubemapArray[2] = cubemapDawn;
    currentCubemap = cubemapOcean;

    // configure light FBO
    // -----------------------
    glGenFramebuffers(1, &lightFBO);

    glGenTextures(1, &lightDepthMaps);
    glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);

    glTexImage3D(
            GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapResolution, depthMapResolution, int(shadowCascadeLevels.size()) + 1,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightDepthMaps, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
        throw 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure UBO
    // --------------------
    unsigned int matricesUBO;
    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // configure point shadows depth map FBO
    // ------------------------------------

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture_diffuse1", 0);
    // map for cascaded shadows
    shader.setInt("shadowMap", 1);
    // map for point shadow
    shader.setInt("depthMap", 2);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        glm::vec3 pointLightColor = glm::vec3(0.3f, 0.3f, 0.3f);
        shader.setVec3("pointLightColor", pointLightColor);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. UBO setup
        const auto lightMatrices = getLightSpaceMatrices();
        glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
        for (size_t i = 0; i < lightMatrices.size(); ++i)
        {
            glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------

        // render scene from light's point of view
        simpleDepthShader.use();

        glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
        glViewport(0, 0, depthMapResolution, depthMapResolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);  // peter panning
        renderScene(simpleDepthShader, objects);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 3. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        pointDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            pointDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        pointDepthShader.setFloat("far_plane", far_plane);
        pointDepthShader.setVec3("lightPos", lightPos);
        renderScene(pointDepthShader, objects);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 4. render scene as normal using the generated depth/shadow maps
        // --------------------------------------------------------------
        shader.use();
        const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)fb_width / (float)fb_height, cameraNearPlane, cameraFarPlane);
        const glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set light uniforms
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightDir", lightDir);
        shader.setFloat("farPlane", cameraFarPlane);
        shader.setFloat("far_plane", far_plane);
        shader.setInt("cascadeCount", shadowCascadeLevels.size());

        shader.setInt("showCascades", showCascades);

        for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
        {
            shader.setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
        }
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shader, objects);

        // draw skybox as last
        renderSkybox(skyboxShader, currentCubemap);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &matricesUBO);
    glDeleteFramebuffers(1, &lightFBO);
    glDeleteTextures(1, &lightDepthMaps);
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthCubemap);

    glfwTerminate();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(Shader &shader, Model objects[])
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    // floor
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glEnable(GL_CULL_FACE);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // rock walls
    placeRockwallX(objects, shader, -23.0f, 0.0f, 0.0f);
    placeRockwallX(objects, shader, -23.0f, 0.0f, -46.0f);
    placeRockwallZ(objects, shader, 0.0f, 0.0f, -23.0f);
    placeRockwallZ(objects, shader, -46.0f, 0.0f, -23.0f);

    // middle rocks + tree
    placeObject(objects[3], shader, 0.0f, 0.5f, 0.0f, 0.0f);
    placeObject(objects[0], shader, 0.0f, 1.5f, 0.0f, 0.0f);
    placeObject(objects[3], shader, 2.0f, -1.2f, 0.0f, 0.0f);

    // bench
    placeObject(objects[6], shader, -12.50f, -1.0f, -15.0f, 45.0f);

    // house
    placeObject(objects[7], shader, 12.0f, 0.0f, -9.0f, 0.0f);

    // light
    placeObject(objects[8], shader, 13.95f, 6.5f, -12.0f, 0.0f);

    // table
    placeObject(objects[9], shader, 13.95f, 0.0f, -12.0f, 0.0f);
}

void placeObject(Model object, Shader shader, float x, float y, float z, float r)
{
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y-2.1f, z));
    model = glm::rotate(model, glm::radians(r), glm::normalize(glm::vec3(0.0, 1.0f, 0.0)));
    model = glm::scale(model, glm::vec3(30.0f));
    shader.setMat4("model", model);
    object.Draw(shader);
}

void placeRockwallX(Model objects[], Shader shader, float x, float y, float z)
{
    placeObject(objects[5], shader, (-1.0f)+x, 0.0f+y, 23.0f+z, 0.0f);
    for (int i = 0; i < 8; ++i) {
        placeObject(objects[3], shader, (0.0f+(float)i*5.5f)+x, 0.0f+y, 23.0f+z, 0.0f);
        placeObject(objects[4], shader, (2.0f+(float)i*5.5f)+x, 0.0f+y, 23.0f+z, 0.0f);
        placeObject(objects[5], shader, (2.5f+(float)i*5.5f)+x, -0.5f+y, 23.0f+z, 0.0f);
        placeObject(objects[5], shader, (4.0f+(float)i*5.5f)+x, 0.0f+y, 23.0f+z, 0.0f);
        placeObject(objects[1], shader, (4.0f+(float)i*5.5f)+x, 5.0f+y, 23.0f+z, 0.0f);
    }
}
void placeRockwallZ(Model objects[], Shader shader, float x, float y, float z)
{
    for (int i = 0; i < 8; ++i) {
        placeObject(objects[3], shader, 23.0f+x, 0.0f+y, (0.0f+(float)i*5.5f)+z, 0.0f);
        placeObject(objects[4], shader, 23.0f+x, 0.0f+y, (2.0f+(float)i*5.5f)+z, 0.0f);
        placeObject(objects[5], shader, 23.0f+x, -0.5f+y, (2.5f+(float)i*5.5f)+z, 0.0f);
        placeObject(objects[5], shader, 23.0f+x, 0.0f+y, (4.0f+(float)i*5.5f)+z, 0.0f);
        placeObject(objects[0], shader, 23.0f+x, 5.0f+y, (4.0f+(float)i*5.5f)+z, 0.0f);
    }
     placeObject(objects[5], shader, 23.0f+x, 0.0f+y, (2.0f+8*5.5f)+z,0.0f);
}

unsigned int skyboxVAO = 0;
unsigned int skyboxVBO = 0;
void loadSkybox()
{
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
    // skybox VAO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void renderSkybox(Shader skyboxShader, unsigned int cubemap)
{
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    skyboxShader.use();
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)fb_width / (float)fb_height, cameraNearPlane, cameraFarPlane);
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    camera.MovementSpeed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 2.5 * 10 : 2.5;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // cascades will be shown visually with colors
    static int qPress = GLFW_RELEASE;
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE && qPress == GLFW_PRESS)
    {
        if(showCascades==1)
        {
            showCascades = 0;
        }
        else
        {
            showCascades = 1;
        }
    }
    qPress = glfwGetKey(window, GLFW_KEY_Q);

    // one cascade
    static int onePress = glfwGetKey(window, GLFW_KEY_1);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE && onePress == GLFW_PRESS)
    {
        shadowCascadeLevels = { cameraFarPlane};
    }
    onePress = glfwGetKey(window, GLFW_KEY_1);
    // two cascades
    static int twoPress = glfwGetKey(window, GLFW_KEY_2);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE && twoPress == GLFW_PRESS)
    {
        shadowCascadeLevels = { cameraFarPlane / 50.0f };
    }
    twoPress = glfwGetKey(window, GLFW_KEY_2);
    // multiple cascades
    static int threePress = glfwGetKey(window, GLFW_KEY_3);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE && threePress == GLFW_PRESS)
    {
        shadowCascadeLevels = { cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f };
    }
    threePress = glfwGetKey(window, GLFW_KEY_3);

    // change skybox
    static int rPress = GLFW_RELEASE;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE && rPress == GLFW_PRESS)
    {
        currentCubemap = cubemapArray[cubemapIndex % 3];
        cubemapIndex++;
    }
    rPress = glfwGetKey(window, GLFW_KEY_R);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    fb_width = width;
    fb_height = height;
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 2)
            format = GL_RG;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int pickAndLoadCubemap(std::string path)
{
    std::vector<std::string> faces
            {
                    path + "/right.png",
                    path + "/left.png",
                    path + "/top.png",
                    path + "/bottom.png",
                    path + "/front.png",
                    path + "/back.png"
            };
    return loadCubemap(faces);
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
{
    const auto inv = glm::inverse(projview);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}


std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    return getFrustumCornersWorldSpace(proj * view);
}

glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
    const auto proj = glm::perspective(
            glm::radians(camera.Zoom), (float)fb_width / (float)fb_height, nearPlane,
            farPlane);
    const auto corners = getFrustumCornersWorldSpace(proj, camera.GetViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}

std::vector<glm::mat4> getLightSpaceMatrices()
{
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(cameraNearPlane, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane));
        }
    }
    return ret;
}
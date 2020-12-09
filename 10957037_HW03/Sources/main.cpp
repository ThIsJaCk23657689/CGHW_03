#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../Headers/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../Headers/logging.h"
#include "../Headers/mstack.h"
#include "../Headers/shader.h"
#include "../Headers/camera.h"
#include "../Headers/followcamera.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <ctime>
#include <random>

enum ROV_Movement {
	ROV_FORWARD,
	ROV_BACKWARD,
	ROV_TURNLEFT,
	ROV_TURNRIGHT,
	ROV_LEFT,
	ROV_RIGHT,
	ROV_UP,
	ROV_DOWN,
};

enum Monitor {
	Monitor_X,
	Monitor_Y,
	Monitor_Z,
	Monitor_Result,
};

void showUI();
void setViewMatrix(int type);
void setProjectionMatrix(int type);
void setViewport(int type);
void geneObejectData();
void geneSphereData();
void updateViewVolumeData();
void drawFloor();
void drawCube();
void drawPlane();
void drawFish();
void drawGrass();
void drawBox();
void drawROV(Shader shader);
void drawCamera(Shader shader);
void drawAxis(Shader shader);
void processROV(ROV_Movement direction, float deltaTime);
void checkNoGetOut();
void updateROVFront();
void drawSphere();
void setFullScreen();
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xpos, double ypos);
void errorCallback(int error, const char* description);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);
glm::mat4 GetPerspectiveProjMatrix(float fovy, float ascept, float znear, float zfar);
glm::mat4 GetOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far);

// ========== Global Variable ==========

// Window parameters
GLFWwindow* window;
bool isfullscreen = false;
const std::string WINDOW_TITLE = "Viewing and Projection";
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
std::vector <int> window_position{ 0, 0 };
std::vector <int> window_size{ 0, 0 };

// Matrix stack paramters
StackArray modelMatrix;

// Transform Matrices
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// Time parameters
float deltaTime = 0.0f;
float lastTime = 0.0f;

// ROV Parameter
static float ROVMovementSpeed = 5.0f;
float ROVYaw = 0.0f;
float ROVEngineAngle = 0.0f;
glm::vec3 ROVPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 FollowCamearaPosition = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 ROVFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 ROVRight = glm::vec3(1.0f, 0.0f, 0.0f);

// Camera parameter
bool isGhost = false;
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool moveCameraDirection = false;
bool showAxis = false;

// Follow Camera parameters
fcamera::FollowCamera followCamera(FollowCamearaPosition, ROVPosition);

// Projection parameters
static bool isPerspective = true;
float aspect_wh = (float)SCR_WIDTH / (float)SCR_HEIGHT;
float aspect_hw = (float)SCR_HEIGHT / (float)SCR_WIDTH;
static float global_left = 0.0f;
static float global_right = 0.0f;
static float global_bottom = 0.0f;
static float global_top = 0.0f;
static float global_near = 0.1f;
static float global_far = 250.0f;
std::vector <glm::vec4> nearPlaneVertex;
std::vector <glm::vec4> farPlaneVertex;

// 0 => x-ortho, 1 => y-ortho, 2 => z-ortho, 3 => main-camera(perspective), 4 => all
static int currentScreen = 4;
static float distanceOrthoCamera = 5.0;

// Light Parameters
glm::vec3 lightPosition = glm::vec3(90.0f, 0.0f, 0.0f);

// Object Data
std::vector<float> cubeVertices;
std::vector<int> cubeIndices;
unsigned int cubeVAO, cubeVBO, cubeEBO;

std::vector<float> floorVertices;
std::vector<unsigned int> floorIndices;
unsigned int floorVAO, floorVBO, floorEBO;

std::vector<float> planeVertices;
unsigned int planeVAO, planeVBO;

std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;
unsigned int sphereVAO, sphereVBO, sphereEBO;

std::vector<float> viewVolumeVertices;
std::vector<int> viewVolumeIndices;
unsigned int viewVolumeVAO, viewVolumeVBO, viewVolumeEBO;

// Texture parameter
unsigned int rovTexture, seaTexture, sandTexture, grassTexture, boxTexture, fishTexture, skyTexture;

int main() {

	// Initialize GLFW
	if (!glfwInit()) {
		logging::loggingMessage(logging::LogType::ERROR, "Failed to initialize GLFW.");
		glfwTerminate();
		return -1;
	}
	else {
		logging::loggingMessage(logging::LogType::DEBUG, "Initialize GLFW successful.");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 32);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE.c_str(), NULL, NULL);
	if (!window) {
		logging::loggingMessage(logging::LogType::ERROR, "Failed to create GLFW window.");
		glfwTerminate();
		return -1;
	}
	else {
		logging::loggingMessage(logging::LogType::DEBUG, "Create GLFW window successful.");
	}

	// Register callbacks
	glfwMakeContextCurrent(window);
	glfwSetErrorCallback(errorCallback);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);

	// Initialize GLAD (Must behind the create window)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		logging::loggingMessage(logging::LogType::ERROR, "Failed to initialize GLAD.");
		glfwTerminate();
		return -1;
	}
	else {
		logging::loggingMessage(logging::LogType::DEBUG, "Initialize GLAD successful.");
	}

	// Initialize ImGui and bind to GLFW and OpenGL3(glad)
	std::string glsl_version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());
	ImGui::StyleColorsDark();

	// Show version info
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	logging::showInitInfo(renderer, version);

	// Setting OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create shader program
	Shader myShader("Shaders/lighting.vs", "Shaders/lighting.fs");
	// Shader textureShader("Shaders\\texture.vs", "Shaders\\texture.fs");
	Shader cubemapShader("Shaders/cubemap.vs", "Shaders/cubemap.fs");
	
	// Create object data
	geneObejectData();

	// Setting amount of fishes, boxed and grass. 
	std::default_random_engine generator(time(NULL));
	std::uniform_real_distribution<float> unif_g(-80.0, 80.0);
	std::uniform_real_distribution<float> unif_f(-60.0, 60.0);
	std::uniform_real_distribution<float> unif_b(-30.0, 30.0);

	std::vector<glm::vec3> boxposition;
	for (int i = 0; i < 20; i++) {
		boxposition.push_back(glm::vec3(unif_b(generator), 0.0f, unif_b(generator)));
	}

	std::vector<glm::vec3> grassposition;
	for (int i = 0; i < 600; i++) {
		grassposition.push_back(glm::vec3(unif_g(generator), 0.0f, unif_g(generator)));
	}

	std::vector<glm::vec3> fishposition;
	for (int i = 0; i < 300; i++) {
		fishposition.push_back(glm::vec3(unif_f(generator), 0.0f, unif_f(generator)));
	}

	// Loading textures
	rovTexture = loadTexture("Resources\\Textures\\metal.png");
	seaTexture = loadTexture("Resources\\Textures\\sea.jpg");
	sandTexture = loadTexture("Resources\\Textures\\sand.jpg");
	grassTexture = loadTexture("Resources\\Textures\\grass.png");
	boxTexture = loadTexture("Resources\\Textures\\container2.png");
	fishTexture = loadTexture("Resources\\Textures\\fish.png");
	skyTexture = loadTexture("Resources\\Textures\\sky.jpg");

	// Loading Cubemap
	std::vector<std::string> faces{
		"Resources/Textures/skybox/right.jpg",
		"Resources/Textures/skybox/left.jpg",
		"Resources/Textures/skybox/top.jpg",
		"Resources/Textures/skybox/bottom.jpg",
		"Resources/Textures/skybox/front.jpg",
		"Resources/Textures/skybox/back.jpg",
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// binding texture to shader
	myShader.use();
	myShader.setInt("material.diffuse", 0);
	myShader.setInt("material.specular", 0);
	myShader.setFloat("material.shininess", 64.0f);
	myShader.setInt("skybox", 2);

	// The main loop
	while (!glfwWindowShouldClose(window)) {
		
		// Calculate the deltaFrame
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		float daytime = sin(currentTime / 10) / 2 + 0.5;

		// Process Input (Moving camera)
		processInput(window);

		// Clear the buffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update the view volume
		updateViewVolumeData();

		// feed inputs to dear imgui start new frame;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		showUI();
		// ImGui::ShowDemoWindow();

		// 控制螢幕顯示
		int scr_start = 0, scr_end = 3;
		if(currentScreen == 0) {
			scr_start = 0;
			scr_end = 0;
		} else if(currentScreen == 1) {
			scr_start = 1;
			scr_end = 1;
		} else if (currentScreen == 2) {
			scr_start = 2;
			scr_end = 2;
		} else if (currentScreen == 3) {
			scr_start = 3;
			scr_end = 3;
		}

		for (int i = scr_start; i <= scr_end; i++) {
			setViewMatrix(i);
			setProjectionMatrix(i);
			setViewport(i);

			// Enable Shader and setting view & projection matrix
			myShader.use();
			myShader.setMat4("view", view);
			myShader.setMat4("projection", projection);

			myShader.setBool("isCubeMap", false);
			myShader.setBool("isGlowObj", false);
			myShader.setFloat("alpha", 1.0f);
			myShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
			myShader.setVec3("viewPos", (isGhost) ? camera.Position: followCamera.Position);
			
			myShader.setVec3("light.position", lightPosition);
			myShader.setVec3("light.ambient", glm::vec3(0.2f, 0.2, 0.2f));
			myShader.setVec3("light.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
			myShader.setVec3("light.specular", glm::vec3(0.4f, 0.4f, 0.4f));
			myShader.setFloat("light.constant", 1.0f);
			myShader.setFloat("light.linear", 0.007f);
			myShader.setFloat("light.quadratic", 0.0002f);

			// Render on the screen;

			// Draw origin and 3 axes 
			if (showAxis) {
				myShader.setBool("enableTexture", false);
				modelMatrix.push();
					modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.2f, 0.2f)));
					myShader.setVec3("color", glm::vec3(0.1, 0.1, 0.1));
					myShader.setMat4("model", modelMatrix.top());
					drawSphere();
				modelMatrix.pop();
				drawAxis(myShader);
			}

			// Draw Skybox (Using Cubemap)
			glDepthFunc(GL_LEQUAL);
			myShader.setBool("isCubeMap", true);
			modelMatrix.push();
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
				modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(distanceOrthoCamera * 5.34)));
				// myShader.setVec3("color", glm::vec3(0.294117647 * daytime, 0.623529412 * daytime, 0.949019608 * daytime));
				myShader.setMat4("model", modelMatrix.top());
				drawCube();
			modelMatrix.pop();
			myShader.setBool("isCubeMap", false);
			glDepthFunc(GL_LESS);

			// Draw Sea
			myShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, seaTexture);
			myShader.setBool("enableTexture", true);
			myShader.setMat4("model", modelMatrix.top());
			drawFloor();

			// Draw Seabed (Sand)
			modelMatrix.push();
				// draw sand
				modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, -5.0f, 0.0f)));
				myShader.setMat4("model", modelMatrix.top());
				glBindTexture(GL_TEXTURE_2D, sandTexture);
				drawFloor();

				// draw grass
				for (unsigned int i = 0; i < grassposition.size(); i++) {
					modelMatrix.push();
						modelMatrix.save(glm::translate(modelMatrix.top(), grassposition[i]));
						myShader.setMat4("model", modelMatrix.top());
						drawGrass();
					modelMatrix.pop();
				}
			modelMatrix.pop();

			// Draw fishes
			modelMatrix.push();
				modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, -2.5f, 0.0f)));
				for (unsigned int i = 0; i < fishposition.size(); i++) {
					modelMatrix.push();
					modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(-sin(currentTime), 0.0f, 0.0f)));
					modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(currentTime * 5), glm::vec3(0.0, 1.0, 0.0)));
					modelMatrix.save(glm::translate(modelMatrix.top(), fishposition[i]));
					modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(1.0f, 0.5f, 0.5f)));
					myShader.setMat4("model", modelMatrix.top());
					drawFish();
					modelMatrix.pop();
				}
			modelMatrix.pop();

			// Draw obstacles
			modelMatrix.push();
				for (unsigned int i = 0; i < boxposition.size(); i++) {
					modelMatrix.push();
					modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(boxposition[i].x, sin(currentTime * 3 + boxposition[i].z) / 4, boxposition[i].z)));
					myShader.setMat4("model", modelMatrix.top());
					drawBox();
					modelMatrix.pop();
				}
			modelMatrix.pop();

			// Draw ROV
			myShader.setBool("enableTexture", false);
			modelMatrix.push();
				modelMatrix.save(glm::translate(modelMatrix.top(), ROVPosition));
				modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(ROVYaw), glm::vec3(0.0, 1.0, 0.0)));
				myShader.setMat4("model", modelMatrix.top());
				if (showAxis) {
					drawAxis(myShader);
				}
				drawROV(myShader);
			modelMatrix.pop();

			// Draw Camera
			modelMatrix.push();
				if(isGhost) {
					glm::vec3 location = camera.Front * -1.4f + camera.Position;
					modelMatrix.save(glm::translate(modelMatrix.top(), location));
					modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(-camera.Yaw), glm::vec3(0.0f, 1.0f, 0.0f)));
					modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f)));
				}else {
					glm::vec3 location = followCamera.Front * 1.4f + followCamera.Position;
					modelMatrix.save(glm::translate(modelMatrix.top(), location));
					modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(-followCamera.Yaw), glm::vec3(0.0f, 1.0f, 0.0f)));
					modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(followCamera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f)));
				}
				drawCamera(myShader);
				if (showAxis) {
					drawAxis(myShader);
				}
			modelMatrix.pop();

			// Update View Volume
			glBindVertexArray(viewVolumeVAO);
			glBindBuffer(GL_ARRAY_BUFFER, viewVolumeVBO);
			glBufferData(GL_ARRAY_BUFFER, viewVolumeVertices.size() * sizeof(float), viewVolumeVertices.data(), GL_STATIC_DRAW);
			glBindVertexArray(0);

			// Draw View Volume
			modelMatrix.push();
				myShader.setVec3("color", glm::vec3(0.6, 0.6, 0.6));
				myShader.setMat4("model", modelMatrix.top());
				myShader.setFloat("alpha", 0.6f);
				glBindVertexArray(viewVolumeVAO);
					glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				myShader.setFloat("alpha", 1.0f);
			modelMatrix.pop();

			// draw sun
			myShader.setBool("isGlowObj", true);
			modelMatrix.push();
				lightPosition = glm::vec3(cos(currentTime / 10) * 90.0f, sin(currentTime / 10) * 90.0f, 0.0f);
				modelMatrix.save(glm::translate(modelMatrix.top(), lightPosition));
				myShader.setVec3("color", glm::vec3(1.0, 1.0, 1.0));
				myShader.setMat4("model", modelMatrix.top());
				drawSphere();
			modelMatrix.pop();
			myShader.setBool("isGlowObj", false);
		}

		// render on the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap Buffers and Trigger event
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &floorVBO);

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);

	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);

	glDeleteVertexArrays(1, &viewVolumeVAO);
	glDeleteBuffers(1, &viewVolumeVBO);
	glDeleteBuffers(1, &viewVolumeEBO);

	// Release the resources.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void showUI() {
	ImGui::Begin("Control Panel");
	ImGuiTabBarFlags tab_bar_flags = ImGuiBackendFlags_None;
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
		if (ImGui::BeginTabItem("ROV")) {
			ImGui::Text("Position = (%.2f, %.2f, %.2f)", ROVPosition.x, ROVPosition.y, ROVPosition.z);
			ImGui::Text("Front = (%.2f, %.2f, %.2f)", ROVFront.x, ROVFront.y, ROVFront.z);
			ImGui::Text("Right = (%.2f, %.2f, %.2f)", ROVRight.x, ROVRight.y, ROVRight.z);
			ImGui::Text("Pitch = %.2f deg", ROVYaw);
			ImGui::SliderFloat("Speed", &ROVMovementSpeed, 1, 20);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Camera")) {
			if (isGhost) {
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "Ghost Camera");
				ImGui::Text("Position = (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
				ImGui::Text("Front = (%.2f, %.2f, %.2f)", camera.Front.x, camera.Front.y, camera.Front.z);
				ImGui::Text("Right = (%.2f, %.2f, %.2f)", camera.Right.x, camera.Right.y, camera.Right.z);
				ImGui::Text("Up = (%.2f, %.2f, %.2f)", camera.Up.x, camera.Up.y, camera.Up.z);
				ImGui::Text("Pitch = %.2f deg, Yaw = %.2f deg", camera.Pitch, camera.Yaw);
			}
			else {
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "Follow Camera");
				ImGui::Text("Position = (%.2f, %.2f, %.2f)", followCamera.Position.x, followCamera.Position.y, followCamera.Position.z);
				ImGui::Text("Front = (%.2f, %.2f, %.2f)", followCamera.Front.x, followCamera.Front.y, followCamera.Front.z);
				ImGui::Text("Right = (%.2f, %.2f, %.2f)", followCamera.Right.x, followCamera.Right.y, followCamera.Right.z);
				ImGui::Text("Up = (%.2f, %.2f, %.2f)", followCamera.Up.x, followCamera.Up.y, followCamera.Up.z);
				ImGui::Text("Target = (%.2f, %.2f, %.2f)", followCamera.Target.x, followCamera.Target.y, followCamera.Target.z);
				ImGui::Text("Distance = %.2f", followCamera.Distance);
				ImGui::Text("Pitch = %.2f deg, Yaw = %.2f deg", followCamera.Pitch, followCamera.Yaw);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Projection")) {

			ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), (isPerspective) ? "Perspective Projection" : "Orthogonal Projection");
			ImGui::Text("Parameters");
			ImGui::BulletText("FoV = %.2f deg, Aspect = %.2f", (isGhost) ? camera.Zoom : followCamera.Zoom, aspect_wh);
			ImGui::BulletText("left: %.2f, right: %.2f ", global_left, global_right);
			ImGui::BulletText("bottom: %.2f, top: %.2f ", global_bottom, global_top);
			ImGui::SliderFloat("Near", &global_near, 0.1, 10);
			ImGui::SliderFloat("Far", &global_far, 10, 250);
			ImGui::Spacing();

			if (ImGui::TreeNode("Projection Matrix")) {
				setProjectionMatrix(3);
				glm::mat4 proj = projection;

				ImGui::Columns(4, "mycolumns");
				ImGui::Separator();
				for (int i = 0; i < 4; i++) {
					ImGui::Text("%.2f", proj[0][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", proj[1][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", proj[2][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", proj[3][i]); ImGui::NextColumn();
					ImGui::Separator();
				}
				ImGui::Columns(1);

				ImGui::TreePop();
			}
			ImGui::Spacing();

			if (ImGui::TreeNode("View Volume Vertices")) {
				ImGui::BulletText("rtnp: (%.2f, %.2f, %.2f)", nearPlaneVertex[0].x, nearPlaneVertex[0].y, nearPlaneVertex[0].z);
				ImGui::BulletText("ltnp: (%.2f, %.2f, %.2f)", nearPlaneVertex[1].x, nearPlaneVertex[1].y, nearPlaneVertex[1].z);
				ImGui::BulletText("rbnp: (%.2f, %.2f, %.2f)", nearPlaneVertex[2].x, nearPlaneVertex[2].y, nearPlaneVertex[2].z);
				ImGui::BulletText("lbnp: (%.2f, %.2f, %.2f)", nearPlaneVertex[3].x, nearPlaneVertex[3].y, nearPlaneVertex[3].z);
				ImGui::BulletText("rtfp: (%.2f, %.2f, %.2f)", farPlaneVertex[0].x, farPlaneVertex[0].y, farPlaneVertex[0].z);
				ImGui::BulletText("ltfp: (%.2f, %.2f, %.2f)", farPlaneVertex[1].x, farPlaneVertex[1].y, farPlaneVertex[1].z);
				ImGui::BulletText("rbfp: (%.2f, %.2f, %.2f)", farPlaneVertex[2].x, farPlaneVertex[2].y, farPlaneVertex[2].z);
				ImGui::BulletText("lbfp: (%.2f, %.2f, %.2f)", farPlaneVertex[3].x, farPlaneVertex[3].y, farPlaneVertex[3].z);
				ImGui::TreePop();
			}
			ImGui::Spacing();

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Illustration")) {

			ImGui::Text("Current Screen: %d", currentScreen + 1);
			ImGui::Text("Ghost Mode: %s", isGhost ? "True" : "false");
			ImGui::Text("Projection Mode: %s", isPerspective ? "Perspective" : "Orthogonal");
			ImGui::Text("Showing Axes: %s", showAxis ? "True" : "false");
			ImGui::Text("Full Screen:  %s", isfullscreen ? "True" : "false");
			ImGui::SliderFloat("zoom", &distanceOrthoCamera, 5, 25);
			ImGui::Spacing();

			if (ImGui::TreeNode("General")) {
				ImGui::BulletText("Press G to switch Ghost Mode");
				ImGui::BulletText("Press X to show / hide the axes");
				ImGui::BulletText("Press Y to switch the projection");
				ImGui::BulletText("Press 1~5 to switch the screen");
				ImGui::BulletText("Press F11 to Full Screen");
				ImGui::BulletText("Press ESC to close the program");
				ImGui::TreePop();
			}
			ImGui::Spacing();

			if (ImGui::TreeNode("ROV Illustration")) {
				ImGui::BulletText("Press W to Forward");
				ImGui::BulletText("Press S to Backward");
				ImGui::BulletText("Press A to Move Left");
				ImGui::BulletText("Press D to Move Right");
				ImGui::BulletText("Press Q to Turn left");
				ImGui::BulletText("Press E to Turn right");
				ImGui::BulletText("Press Left Shift to Dive");
				ImGui::BulletText("Press Space to Rise");
				ImGui::TreePop();
			}
			ImGui::Spacing();

			if (ImGui::TreeNode("Follow Camera Illustration")) {
				ImGui::BulletText("Press O to Increase Distance");
				ImGui::BulletText("Press P to Decrease Distance");
				ImGui::BulletText("Hold mouse right button to change view angle");
				ImGui::BulletText("Mouse scroll to Zoom in / out");
				ImGui::TreePop();
			}
			ImGui::Spacing();

			if (ImGui::TreeNode("Ghost Camera Illustration")) {
				ImGui::BulletText("WSAD to move camera");
				ImGui::BulletText("Hold mouse right button to rotate");
				ImGui::BulletText("Press Left Shift to speed up");
				ImGui::TreePop();
			}
			ImGui::Spacing();

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::End();
}

void setViewMatrix(int type) {

	if (isGhost) {
		switch (type) {
			case Monitor::Monitor_X:
				view = glm::lookAt(camera.Position + glm::vec3(distanceOrthoCamera, 0.0, 0.0), camera.Position, glm::vec3(0.0, 1.0, 0.0));
				break;
			case Monitor::Monitor_Y:
				view = glm::lookAt(camera.Position + glm::vec3(0.0, distanceOrthoCamera, 0.0), camera.Position, glm::vec3(0.0, 0.0, -1.0));
				break;
			case Monitor::Monitor_Z:
				view = glm::lookAt(camera.Position + glm::vec3(0.0, 0.0, distanceOrthoCamera), camera.Position, glm::vec3(0.0, 1.0, 0.0));
				break;
			case Monitor::Monitor_Result:
				view = camera.GetViewMatrix();
				break;
		}
	} else {
		switch (type) {
			case Monitor::Monitor_X:
				view = glm::lookAt(followCamera.Position + glm::vec3(distanceOrthoCamera, 0.0, 0.0), followCamera.Position, glm::vec3(0.0, 1.0, 0.0));
				break;
			case Monitor::Monitor_Y:
				view = glm::lookAt(followCamera.Position + glm::vec3(0.0, distanceOrthoCamera, 0.0), followCamera.Position, glm::vec3(0.0, 0.0, -1.0));
				break;
			case Monitor::Monitor_Z:
				view = glm::lookAt(followCamera.Position + glm::vec3(0.0, 0.0, distanceOrthoCamera), followCamera.Position, glm::vec3(0.0, 1.0, 0.0));
				break;
			case Monitor::Monitor_Result:
				view = followCamera.GetViewMatrix();
				break;
		}
	}
}

void setProjectionMatrix(int type) {
	aspect_wh = (float)SCR_WIDTH / (float)SCR_HEIGHT;
	aspect_hw = (float)SCR_HEIGHT / (float)SCR_WIDTH;

	if (type == Monitor::Monitor_Result) {
		if (isGhost) {
			if (isPerspective) {
				projection = GetPerspectiveProjMatrix(glm::radians(camera.Zoom), aspect_wh, global_near, global_far);
			} else {
				float length = tan(glm::radians(camera.Zoom / 2)) * global_near * 50;
				if (SCR_WIDTH > SCR_HEIGHT) {
					projection = GetOrthoProjMatrix(-length, length, -length * aspect_hw, length * aspect_hw, global_near, global_far);
				} else {
					projection = GetOrthoProjMatrix(-length * aspect_wh, length * aspect_wh, -length, length, global_near, global_far);
				}
			}
		} else {
			if(isPerspective) {
				projection = GetPerspectiveProjMatrix(glm::radians(followCamera.Zoom), aspect_wh, global_near, global_far);
			} else {
				float length = tan(glm::radians(followCamera.Zoom / 2)) * global_near * 50;
				if (SCR_WIDTH > SCR_HEIGHT) {
					projection = GetOrthoProjMatrix(-length, length, -length * aspect_hw, length * aspect_hw, global_near, global_far);
				} else {
					projection = GetOrthoProjMatrix(-length * aspect_wh, length * aspect_wh, -length, length, global_near, global_far);
				}
			}
		}
	} else {
		if (SCR_WIDTH > SCR_HEIGHT) {
			projection = GetOrthoProjMatrix(-distanceOrthoCamera, distanceOrthoCamera, -distanceOrthoCamera * aspect_hw, distanceOrthoCamera * aspect_hw, 0.1f, 250.0f);
		} else {
			projection = GetOrthoProjMatrix(-distanceOrthoCamera * aspect_wh, distanceOrthoCamera * aspect_wh, -distanceOrthoCamera, distanceOrthoCamera, 0.1f, 250.0f);
		}
	}
}

void setViewport(int type) {
	if(currentScreen == 4) {
		switch (type) {
			case Monitor::Monitor_X:
				glViewport(0, SCR_HEIGHT / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2);
				break;
			case Monitor::Monitor_Y:
				glViewport(SCR_WIDTH / 2, SCR_HEIGHT / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2);
				break;
			case Monitor::Monitor_Z:
				glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT / 2);
				break;
			case Monitor::Monitor_Result:
				glViewport(SCR_WIDTH / 2, 0, SCR_WIDTH / 2, SCR_HEIGHT / 2);
				break;
		}
	} else {
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	}
}

void geneObejectData() {
	// ========== Generate Cube vertex data ==========
	cubeVertices = {
		// Positions			// Normals 			// Texture coords
		 0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,

		0.5f,  0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	1.0f, 0.0f, -1.0f,	0.0f, 1.0f,
	};
	cubeIndices = {
		0, 1, 3,
		1, 2, 3,

		4, 5, 7,
		5, 6, 7,

		8, 9, 11,
		9, 10, 11,

		12, 13, 15,
		13, 14, 15,

		16, 17, 19,
		17, 18, 19,

		20, 21, 23,
		21, 22, 23,
	};
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), cubeIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================


	// ========== Generate floor vertex data ==========
	floorVertices = {
		// Positions			// Normals			// Texture Coords
		-100.0,  0.0,  100.0,	0.0f, 1.0f, 0.0f,	25.0f,  0.0f,
		 100.0,  0.0,  100.0,	0.0f, 1.0f, 0.0f,	25.0f, 25.0f,
		 100.0,  0.0, -100.0,	0.0f, 1.0f, 0.0f,	 0.0f, 25.0f,
		-100.0,  0.0, -100.0,	0.0f, 1.0f, 0.0f,	 0.0f,  0.0f,
	};
	floorIndices = {
		0, 1, 2,
		0, 2, 3,
	};
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glGenBuffers(1, &floorEBO);
	glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(float), floorVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, floorIndices.size() * sizeof(unsigned int), floorIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================


	// ========== Generate grass vertex data ==========
	planeVertices = {
		// Positions		// Normals			// Texture coords
		 0.0,  1.0, 0.0,	0.0, 0.0, 1.0,		0.0, 0.0,
		 0.0,  0.0, 0.0,	0.0, 0.0, 1.0,		0.0, 1.0,
		 1.0,  0.0, 0.0,	0.0, 0.0, 1.0,		1.0, 1.0,

		 0.0,  1.0, 0.0,	0.0, 0.0, 1.0,		0.0, 0.0,
		 1.0,  0.0, 0.0,	0.0, 0.0, 1.0,		1.0, 1.0,
		 1.0,  1.0, 0.0,	0.0, 0.0, 1.0,		1.0, 0.0,
	};
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(float), planeVertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================
	
	// ========== Generate View Volume vertex data ==========
	viewVolumeVertices = {
		// Positions			// Normals			// Texture Coords
		 0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,

		0.5f,  0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
	};
	viewVolumeIndices = {
		0, 1, 3,
		1, 2, 3,

		4, 5, 7,
		5, 6, 7,

		8, 9, 11,
		9, 10, 11,

		12, 13, 15,
		13, 14, 15,

		16, 17, 19,
		17, 18, 19,

		20, 21, 23,
		21, 22, 23,
	};
	glGenVertexArrays(1, &viewVolumeVAO);
	glGenBuffers(1, &viewVolumeVBO);
	glGenBuffers(1, &viewVolumeEBO);
	glBindVertexArray(viewVolumeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, viewVolumeVBO);
		glBufferData(GL_ARRAY_BUFFER, viewVolumeVertices.size() * sizeof(float), viewVolumeVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, viewVolumeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, viewVolumeIndices.size() * sizeof(unsigned int), viewVolumeIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================

	// ========== Generate sphere vertex data ==========
	geneSphereData();
	// ==================================================
}

void geneSphereData() {
	float radius = 1.0f;
	unsigned int latitude = 30;
	unsigned int longitude = 30;

	for (int i = 0; i <= latitude; i++) {
		float theta = i * M_PI / latitude;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);
		for (int j = 0; j <= longitude; j++) {
			float phi = j * 2.0f * M_PI / longitude;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);

			float x = cosPhi * sinTheta;
			float y = cosTheta;
			float z = sinPhi * sinTheta;

			sphereVertices.push_back(radius * x);
			sphereVertices.push_back(radius * y);
			sphereVertices.push_back(radius * z);

			// Generate normal vectors
			glm::vec3 normal = glm::vec3(2 * radius * x, 2 * radius * y, 2 * radius * z);
			normal = glm::normalize(normal);
			sphereVertices.push_back(normal.x);
			sphereVertices.push_back(normal.y);
			sphereVertices.push_back(normal.z);

			// Generate texture coordinate
			float u = 1 - (j / longitude);
			float v = 1 - (i / latitude);
			sphereVertices.push_back(u);
			sphereVertices.push_back(-v);
		}
	}

	for (int i = 0; i < latitude; i++) {
		for (int j = 0; j < longitude; j++) {
			int first = (i * (longitude + 1)) + j;
			int second = first + longitude + 1;

			sphereIndices.push_back(first);
			sphereIndices.push_back(second);
			sphereIndices.push_back(first + 1);

			sphereIndices.push_back(second);
			sphereIndices.push_back(second + 1);
			sphereIndices.push_back(first + 1);
		}
	}

	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereEBO);
	glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
		glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}

void updateViewVolumeData() {

	glm::vec4 rtnp, ltnp, rbnp, lbnp, rtfp, ltfp, rbfp, lbfp = glm::vec4(1.0f);

	if(isPerspective) {
		// 計算透視投影矩陣的各參數
		float p_tn = tan(glm::radians(((isGhost) ? camera.Zoom : followCamera.Zoom) / 2)) * global_near;
		float p_bn = -p_tn;
		float p_rn = p_tn * aspect_wh;
		float p_ln = -p_rn;

		float p_tf = p_tn * global_far / global_near;
		float p_bf = -p_tf;
		float p_rf = p_rn * global_far / global_near;
		float p_lf = -p_rf;

		global_left = p_ln;
		global_right = p_rn;
		global_bottom = p_bn;
		global_top = p_tn;

		// 創建近平面的4個頂點 （記得要將近平面往前多挪0.01，攝影機才不會看不到）
		rtnp = glm::vec4(p_rn, p_tn, -global_near + 0.01, 1.0);
		ltnp = glm::vec4(p_ln, p_tn, -global_near + 0.01, 1.0);
		rbnp = glm::vec4(p_rn, p_bn, -global_near + 0.01, 1.0);
		lbnp = glm::vec4(p_ln, p_bn, -global_near + 0.01, 1.0);

		// 創建遠平面的4個頂點　（記得要將遠平面往後多挪0.01，背景才不會打架）
		rtfp = glm::vec4(p_rf, p_tf, -global_far - 0.01, 1.0);
		ltfp = glm::vec4(p_lf, p_tf, -global_far - 0.01, 1.0);
		rbfp = glm::vec4(p_rf, p_bf, -global_far - 0.01, 1.0);
		lbfp = glm::vec4(p_lf, p_bf, -global_far - 0.01, 1.0);
	} else {
		float length = tan(glm::radians(((isGhost) ? camera.Zoom : followCamera.Zoom) / 2)) * global_near * 50;
		float r, t = 0.0f;
		if (SCR_WIDTH > SCR_HEIGHT) {
			r = length;
			t = length * aspect_hw;
		} else {
			r = length * aspect_wh;
			t = length;
		}
		float l = -r;
		float b = -t;

		global_left = l;
		global_right = r;
		global_bottom = b;
		global_top = t;
		
		// 創建近平面的4個頂點 （記得要將近平面往前多挪0.01，攝影機才不會看不到）
		rtnp = glm::vec4(r, t, -global_near + 0.01, 1.0);
		ltnp = glm::vec4(l, t, -global_near + 0.01, 1.0);
		rbnp = glm::vec4(r, b, -global_near + 0.01, 1.0);
		lbnp = glm::vec4(l, b, -global_near + 0.01, 1.0);

		// 創建遠平面的4個頂點　（記得要將遠平面往後多挪0.01，背景才不會打架）
		rtfp = glm::vec4(r, t, -global_far - 0.01, 1.0);
		ltfp = glm::vec4(l, t, -global_far - 0.01, 1.0);
		rbfp = glm::vec4(r, b, -global_far - 0.01, 1.0);
		lbfp = glm::vec4(l, b, -global_far - 0.01, 1.0);
	}

	// 取得右下角之觀察矩陣後，求反矩陣
	setViewMatrix(3);
	glm::mat4 view_inv = glm::inverse(view);

	// 將這些頂點乘上觀察反矩陣，即可求出世界坐標系頂點（先算近平面）
	rtnp = view_inv * rtnp;
	ltnp = view_inv * ltnp;
	rbnp = view_inv * rbnp;
	lbnp = view_inv * lbnp;

	// 再來換遠平面
	rtfp = view_inv * rtfp;
	ltfp = view_inv * ltfp;
	rbfp = view_inv * rbfp;
	lbfp = view_inv * lbfp;

	nearPlaneVertex = {
		rtnp, ltnp, rbnp, lbnp
	};
	farPlaneVertex = {
		rtfp, ltfp, rbfp, lbfp
	};

	// 更新 View Volume 的頂點資料
	viewVolumeVertices = {
		// Positions				// Normals			// Texture Coords

		// Front
		rtnp.x, rtnp.y, rtnp.z,		0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		rbnp.x, rbnp.y, rbnp.z,		0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		lbnp.x, lbnp.y, lbnp.z,		0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		ltnp.x, ltnp.y, ltnp.z,		0.0f, 0.0f, 1.0f,	0.0f, 1.0f,

		// Right
		rtnp.x, rtnp.y, rtnp.z,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		rtfp.x, rtfp.y, rtfp.z,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		rbfp.x, rbfp.y, rbfp.z,		1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		rbnp.x, rbnp.y, rbnp.z,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		// Left
		ltnp.x, ltnp.y, ltnp.z,		-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		ltfp.x, ltfp.y, ltfp.z,		-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		lbfp.x, lbfp.y, lbfp.z,		-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		lbnp.x, lbnp.y, lbnp.z,		-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		// Top
		rtnp.x, rtnp.y, rtnp.z,		0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		rtfp.x, rtfp.y, rtfp.z,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		ltfp.x, ltfp.y, ltfp.z,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		ltnp.x, ltnp.y, ltnp.z,		0.0f, 1.0f, 0.0f,	0.0f, 1.0f,

		// Down
		rbnp.x, rbnp.y, rbnp.z,		0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		rbfp.x, rbfp.y, rbfp.z,		0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
		lbfp.x, lbfp.y, lbfp.z,		0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		lbnp.x, lbnp.y, lbnp.z,		0.0f, -1.0f, 0.0f,	0.1f, 0.0f,

		// Back
		rtfp.x, rtfp.y, rtfp.z,		0.0f, 0.0f, -1.0f,	1.0f, 1.0f,
		rbfp.x,	rbfp.y, rbfp.z,		0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		lbfp.x,	lbfp.y, lbfp.z,		0.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		ltfp.x,	ltfp.y, ltfp.z,		0.0f, 0.0f, -1.0f,	0.1f, 0.0f,
	};
}

void drawFloor() {
	modelMatrix.push();
	glBindVertexArray(floorVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	modelMatrix.pop();
}

void drawCube() {
	modelMatrix.push();
	glBindVertexArray(cubeVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	modelMatrix.pop();
}

void drawPlane() {
	modelMatrix.push();
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	modelMatrix.pop();
}

void drawFish() {
	glBindTexture(GL_TEXTURE_2D, fishTexture);
	drawPlane();
}

void drawGrass() {
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	drawPlane();
}

void drawBox() {
	glBindTexture(GL_TEXTURE_2D, boxTexture);
	drawCube();
}

void drawROV(Shader shader) {
	modelMatrix.push();
	// Head
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(1.0f, 0.6f, 2.0f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(1.0f, 0.956862745f, 0.580392157f));
	drawCube();
	modelMatrix.pop();

	// Body
	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, -0.5f, 0.0f)));
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.8f, 0.4f, 1.6f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.611764706f, 0.611764706f, 0.611764706f));
	drawCube();
	modelMatrix.pop();

	// Camera
	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, -0.95f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.2f, 0.3f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.1f, 0.1f, 0.1f));
	drawCube();
	modelMatrix.pop();

	// Hand
	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, -0.2f, -0.4f)));
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.2f, 0.2f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.4f, 0.4f, 0.4f));
	drawSphere();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, -0.3f, 0.0f)));
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.05f, 0.6f, 0.05f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, -0.3f, 0.0f)));
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.15f, 0.15f, 0.15f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.4f, 0.4f, 0.4f));
	drawSphere();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, -0.5f)));
	modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.05f, 1.0f, 0.05f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, -1.0f)));
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.1f, 0.1f, 0.1f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.4f, 0.4f, 0.4f));
	drawSphere();
	modelMatrix.pop();

	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, -0.1f)));
	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(-0.05f, 0.0f, 0.0f)));
	modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.05f, 0.2f, 0.2f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.05f, 0.0f, 0.0f)));
	modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.05f, 0.2f, 0.2f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();
	modelMatrix.pop();
	modelMatrix.pop();

	// Engine
	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, 1.1f)));
	//modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.1f, 0.1f, 0.6f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, 0.3f)));
	modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(ROVEngineAngle), glm::vec3(0.0f, 0.0f, 1.0f)));
	modelMatrix.push();
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.2f, 0.1f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.4f, 0.4f, 0.4f));
	drawSphere();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.3f, 0.0f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.6f, 0.05f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(120.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.3f, 0.0f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.6f, 0.05f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(240.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.3f, 0.0f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.6f, 0.05f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.pop();


	modelMatrix.pop();

	modelMatrix.pop();

	modelMatrix.pop();
}

void drawCamera(Shader shader) {
	modelMatrix.push();
		modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(1.0f, 0.8f, 1.8f)));
		shader.setVec3("color", glm::vec3(0.2f, 0.2f, 0.2f));
		shader.setMat4("model", modelMatrix.top());
		drawCube();

		modelMatrix.push();
			modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, -0.2f)));
			modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.6f, 0.6f, 1.2f)));
			shader.setVec3("color", glm::vec3(0.25f, 0.25f, 0.25f));
			shader.setMat4("model", modelMatrix.top());
			drawCube();
		modelMatrix.pop();
	modelMatrix.pop();
}

void drawAxis(Shader shader) {
	shader.setBool("isGlowObj", true);
	modelMatrix.push();
	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(1.5f, 0.0f, 0.0f)));
	// modelMatrix.save(glm::rotate(modelMatrix.top(), glm::radians(currentTime * 5), glm::vec3(0.0, 1.0, 0.0)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(3.0f, 0.1f, 0.1f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
	drawCube();
	modelMatrix.pop();


	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 1.5f, 0.0f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.1f, 3.0f, 0.1f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
	drawCube();
	modelMatrix.pop();

	modelMatrix.push();
	modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, 1.5f)));
	modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.1f, 0.1f, 3.0f)));
	shader.setMat4("model", modelMatrix.top());
	shader.setVec3("color", glm::vec3(0.0f, 0.0f, 1.0f));
	drawCube();
	modelMatrix.pop();
	modelMatrix.pop();
	shader.setBool("isGlowObj", false);
}

void processROV(ROV_Movement direction, float deltaTime) {
	float velocity = ROVMovementSpeed * deltaTime;
	if (direction == ROV_Movement::ROV_FORWARD) {
		ROVPosition += ROVFront * velocity;
		checkNoGetOut();
		followCamera.updateTargetPosition(ROVPosition);
		ROVEngineAngle += ROVMovementSpeed * 40 * velocity;
		if (ROVEngineAngle > 360) {
			ROVEngineAngle -= 360;
		}
	}
	if (direction == ROV_Movement::ROV_BACKWARD) {
		ROVPosition -= ROVFront * velocity;
		checkNoGetOut();
		followCamera.updateTargetPosition(ROVPosition);
		ROVEngineAngle -= ROVMovementSpeed * 40 * velocity;
		if (ROVEngineAngle < -360) {
			ROVEngineAngle += 360;
		}
	}
	if (direction == ROV_Movement::ROV_LEFT) {
		ROVPosition -= ROVRight * velocity;
		checkNoGetOut();
		followCamera.updateTargetPosition(ROVPosition);
	}
	if (direction == ROV_Movement::ROV_RIGHT) {
		ROVPosition += ROVRight * velocity;
		checkNoGetOut();
		followCamera.updateTargetPosition(ROVPosition);
	}
	if (direction == ROV_Movement::ROV_TURNLEFT) {
		ROVYaw += 8 * velocity;
		if (ROVYaw > 360) {
			ROVYaw -= 360;
		}
		updateROVFront();
	}
	if (direction == ROV_Movement::ROV_TURNRIGHT) {
		ROVYaw -= 8 * velocity;
		if (ROVYaw < -360) {
			ROVYaw += 360;
		}
		updateROVFront();
	}
	if (direction == ROV_Movement::ROV_UP) {
		if (ROVPosition.y >= -3.0f && ROVPosition.y <= 0.7f) {
			ROVPosition += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
			followCamera.updateTargetPosition(ROVPosition);
		}
		if (ROVPosition.y > 0.7f) {
			ROVPosition.y = 0.7f;
		}
	}
	if (direction == ROV_Movement::ROV_DOWN) {
		if (ROVPosition.y >= -3.0f && ROVPosition.y <= 0.7f) {
			ROVPosition -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
			followCamera.updateTargetPosition(ROVPosition);
		}
		if (ROVPosition.y < -3.0f) {
			ROVPosition.y = -3.0f;
		}
	}
}

void checkNoGetOut() {
	if (ROVPosition.x > 98.0f) {
		ROVPosition.x = 98.0f;
	}

	if (ROVPosition.x < -98.0f) {
		ROVPosition.x = -98.0f;
	}

	if (ROVPosition.z > 98.0f) {
		ROVPosition.z = 98.0f;
	}

	if (ROVPosition.z < -98.0f) {
		ROVPosition.z = -98.0f;
	}
}

void updateROVFront() {
	glm::mat4 rotatematrix = glm::rotate(glm::mat4(1.0f), glm::radians(ROVYaw), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 front = rotatematrix * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	ROVFront = glm::vec3(front.x, front.y, front.z);

	ROVRight = glm::normalize(glm::cross(ROVFront, glm::vec3(0.0f, 1.0f, 0.0f)));
}

void drawSphere() {
	modelMatrix.push();
	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	modelMatrix.pop();
}

void setFullScreen() {
	// Create Window
	if (isfullscreen) {
		glfwGetWindowPos(window, &window_position[0], &window_position[1]);
		glfwGetWindowSize(window, &window_size[0], &window_size[1]);
		GLFWmonitor* myMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(myMonitor);
		glfwSetWindowMonitor(window, myMonitor, 0, 0, mode->width, mode->height, 0);
	}
	else {
		glfwSetWindowMonitor(window, nullptr, window_position[0], window_position[1], window_size[0], window_size[1], 0);
	}
}

// Handle window size change.
void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {

	// Set new width and height
	SCR_WIDTH = width;
	SCR_HEIGHT = height;

	// Reset projection matrix and viewport
	if (isGhost) {
		projection = GetPerspectiveProjMatrix(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 250.0f);
	} else {
		projection = GetPerspectiveProjMatrix(glm::radians(followCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 250.0f);
	}
	glViewport(0, 0, width, height);
}

// Handle the input which in the main loop
void processInput(GLFWwindow* window) {
	if (isGhost) {
		// like ghost, u can go any where.
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			camera.MovementSpeed = 25.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
			camera.MovementSpeed = 10.0f;
		}
	} else {
		// Oh~ poor guy, u only can move the ROV.
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_FORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_BACKWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_LEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_RIGHT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_TURNLEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_TURNRIGHT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_UP, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			processROV(ROV_Movement::ROV_DOWN, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
			followCamera.AdjustDistance(-0.5);
		}
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			followCamera.AdjustDistance(0.5);
		}
	}
}

// Handle the key callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	// Only handle press events
	if (action == GLFW_RELEASE) {
		return;
	}

	// Exit the program
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
	}

	// Full screen switch 
	if (key == GLFW_KEY_F11) {
		if (isfullscreen) {
			isfullscreen = false;
			setFullScreen();
			logging::loggingMessage(logging::LogType::INFO, "Fullscreen: off.");
		}
		else {
			isfullscreen = true;
			setFullScreen();
			logging::loggingMessage(logging::LogType::INFO, "Fullscreen: on.");
		}
	}

	if (key == GLFW_KEY_X) {
		if (showAxis) {
			showAxis = false;
			logging::loggingMessage(logging::LogType::INFO, "Hidding Axis.");
		}
		else {
			showAxis = true;
			logging::loggingMessage(logging::LogType::INFO, "Showing Axis.");
		}
	}

	if (key == GLFW_KEY_G) {
		if (isGhost) {
			isGhost = false;
			logging::loggingMessage(logging::LogType::INFO, "Ghost mode is turn off.");
		}
		else {
			isGhost = true;
			logging::loggingMessage(logging::LogType::INFO, "You're a ghost!");
		}
	}
	
	if (key == GLFW_KEY_Y) {
		if (isPerspective) {
			isPerspective = false;
			logging::loggingMessage(logging::LogType::INFO, "Using Orthogonal Projection");
		}
		else {
			isPerspective = true;
			logging::loggingMessage(logging::LogType::INFO, "Using Perspective Projection");
		}
	}

	if (key == GLFW_KEY_1) {
		currentScreen = 0;
		logging::loggingMessage(logging::LogType::INFO, "Switch to Screen 1.");
	}
	if (key == GLFW_KEY_2) {
		currentScreen = 1;
		logging::loggingMessage(logging::LogType::INFO, "Switch to Screen 2.");
	}
	if (key == GLFW_KEY_3) {
		currentScreen = 2;
		logging::loggingMessage(logging::LogType::INFO, "Switch to Screen 3.");
	}
	if (key == GLFW_KEY_4) {
		currentScreen = 3;
		logging::loggingMessage(logging::LogType::INFO, "Switch to Screen 4.");
	}
	if (key == GLFW_KEY_5) {
		currentScreen = 4;
		logging::loggingMessage(logging::LogType::INFO, "Switch to All Screen.");
	}
}

// Handle mouse movement (cursor's position)
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	// In the first time u create a window, ur cursor may not in the middle of the window.
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	// Allow u to move the direction of camera
	if (moveCameraDirection) {
		if (isGhost) {
			camera.ProcessMouseMovement(xoffset, yoffset);
		}
		else {
			followCamera.ProcessMouseMovement(xoffset, yoffset);
		}
	}
}

// Handle mouse button (like: left middle right)
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		moveCameraDirection = true;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		moveCameraDirection = false;
	}
}

// Handle mouse scroll
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (isGhost) {
		camera.ProcessMouseScroll(yoffset);
	} else {
		followCamera.ProcessMouseScroll(yoffset);
	}
}

// Handle GLFW Error Callback
void errorCallback(int error, const char* description) {
	logging::loggingMessage(logging::LogType::ERROR, description);
}

// Loading Texture
unsigned int loadTexture(char const* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1) {
			format = GL_RED;
		}
		else if (nrComponents == 3) {
			format = GL_RGB;
		}
		else if (nrComponents == 4) {
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Failed to load texture at path:" << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// Loading Cubemap
unsigned int loadCubemap(std::vector<std::string> faces) {

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Failed to load Cubemap texture at path:" << faces[i] << std::endl;
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

glm::mat4 GetPerspectiveProjMatrix(float fovy, float ascept, float znear, float zfar) {

	glm::mat4 proj = glm::mat4(1.0f);

	proj[0][0] = 1 / (tan(fovy / 2) * ascept);
	proj[1][0] = 0;
	proj[2][0] = 0;
	proj[3][0] = 0;

	proj[0][1] = 0;
	proj[1][1] = 1 / (tan(fovy / 2));
	proj[2][1] = 0;
	proj[3][1] = 0;

	proj[0][2] = 0;
	proj[1][2] = 0;
	proj[2][2] = -(zfar + znear) / (zfar - znear);
	proj[3][2] = (-2 * zfar * znear) / (zfar - znear);

	proj[0][3] = 0;
	proj[1][3] = 0;
	proj[2][3] = -1;
	proj[3][3] = 0;

	return proj;
}

glm::mat4 GetOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far) {
	glm::mat4 proj = glm::mat4(1.0f);

	proj[0][0] = 2 / (right - left);
	proj[1][0] = 0;
	proj[2][0] = 0;
	proj[3][0] = -(right + left) / (right - left);

	proj[0][1] = 0;
	proj[1][1] = 2 / (top - bottom);
	proj[2][1] = 0;
	proj[3][1] = -(top + bottom) / (top - bottom);

	proj[0][2] = 0;
	proj[1][2] = 0;
	proj[2][2] = -2 / (far - near);
	proj[3][2] = -(far + near) / (far - near);

	proj[0][3] = 0;
	proj[1][3] = 0;
	proj[2][3] = 0;
	proj[3][3] = 1;

	return proj;
}
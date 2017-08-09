#include "VulkanGlobal.h"
#include "../common/Macros.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <array>
#include "../maths/Matrix.h"
#include <math.h>
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"
#include "Buffer.h"
#include "StagingBuffer.h"
#include "Queue.h"
#include "StagingBufferManager.h"
#include "FrameManager.h"
#include "../thread/ThreadWorker.hpp"
#include <gli\gli.hpp>

void VulkanGlobal::InitVulkanInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanLearn";
	appInfo.apiVersion = (((1) << 22) | ((0) << 12) | (0));

	//Need surface extension to create surface from device
	std::vector<const char*> extensions = { EXTENSION_VULKAN_SURFACE };
	std::vector<const char*> layers;
#if defined(_WIN32)
	extensions.push_back(EXTENSION_VULKAN_SURFACE_WIN32);
#endif
#if defined(_DEBUG)
	layers.push_back(EXTENSION_VULKAN_VALIDATION_LAYER);
	extensions.push_back(EXTENSION_VULKAN_DEBUG_REPORT);
#endif
	VkInstanceCreateInfo instCreateInfo = {};
	instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instCreateInfo.pApplicationInfo = &appInfo;
	instCreateInfo.enabledExtensionCount = (int32_t)extensions.size();
	instCreateInfo.ppEnabledExtensionNames = extensions.data();
	instCreateInfo.enabledLayerCount = (int32_t)layers.size();
	instCreateInfo.ppEnabledLayerNames = layers.data();

	m_pVulkanInstance = Instance::Create(instCreateInfo);
	assert(m_pVulkanInstance != nullptr);
}

void VulkanGlobal::InitPhysicalDevice(HINSTANCE hInstance, HWND hWnd)
{
	m_pPhysicalDevice = PhysicalDevice::Create(m_pVulkanInstance, hInstance, hWnd);
	ASSERTION(m_pPhysicalDevice != nullptr);
}

void VulkanGlobal::InitVulkanDevice()
{
	m_pDevice = Device::Create(m_pVulkanInstance, m_pPhysicalDevice);
	ASSERTION(m_pDevice != nullptr);
}

#if defined (_WIN32)
void VulkanGlobal::SetupWindow(HINSTANCE hinstance, WNDPROC wndproc)
{
	m_hPlatformInst = hinstance;

	bool fullscreen = false;

	// Check command line arguments
	for (int32_t i = 0; i < __argc; i++)
	{
		if (__argv[i] == std::string("-fullscreen"))
		{
			fullscreen = true;
		}
	}

	WNDCLASSEX wndClass;

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndproc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hinstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = PROJECT_NAME;
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		std::cout << "Could not register window class!\n";
		fflush(stdout);
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((WINDOW_WIDTH != screenWidth) && (WINDOW_HEIGHT != screenHeight))
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					fullscreen = FALSE;
				}
				else
				{
					return;
				}
			}
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	if (fullscreen)
	{
		windowRect.left = (long)0;
		windowRect.right = (long)screenWidth;
		windowRect.top = (long)0;
		windowRect.bottom = (long)screenHeight;
	}
	else
	{
		windowRect.left = (long)screenWidth / 2 - WINDOW_WIDTH / 2;
		windowRect.right = (long)WINDOW_WIDTH;
		windowRect.top = (long)screenHeight / 2 - WINDOW_HEIGHT / 2;
		windowRect.bottom = (long)WINDOW_HEIGHT;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	std::string windowTitle = PROJECT_NAME;
	m_hWindow = CreateWindowEx(0,
		PROJECT_NAME,
		windowTitle.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		windowRect.left,
		windowRect.top,
		windowRect.right,
		windowRect.bottom,
		NULL,
		NULL,
		hinstance,
		NULL);

	if (!m_hWindow)
	{
		printf("Could not create window!\n");
		fflush(stdout);
		exit(1);
	}

	ShowWindow(m_hWindow, SW_SHOW);
	SetForegroundWindow(m_hWindow);
	SetFocus(m_hWindow);
}

#if defined(_WIN32)
#define KEY_ESCAPE VK_ESCAPE 
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_F3 VK_F3
#define KEY_F4 VK_F4
#define KEY_F5 VK_F5
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44
#define KEY_P 0x50
#define KEY_SPACE 0x20
#define KEY_KPADD 0x6B
#define KEY_KPSUB 0x6D
#define KEY_B 0x42
#define KEY_F 0x46
#define KEY_L 0x4C
#define KEY_N 0x4E
#define KEY_O 0x4F
#define KEY_T 0x54
#endif

void VulkanGlobal::HandleMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	float x, y, width, height;
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case KEY_ESCAPE:
			PostQuitMessage(0);
			break;
		}

		switch (wParam)
		{
		case KEY_W:
			m_moveFlag |= CharMoveDir::Forward;
			break;
		case KEY_S:
			m_moveFlag |= CharMoveDir::Backward;
			break;
		case KEY_A:
			m_moveFlag |= CharMoveDir::Leftward;
			break;
		case KEY_D:
			m_moveFlag |= CharMoveDir::Rightward;
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case KEY_W:
			m_moveFlag &= ~(CharMoveDir::Forward);
			break;
		case KEY_S:
			m_moveFlag &= ~(CharMoveDir::Backward);
			break;
		case KEY_A:
			m_moveFlag &= ~(CharMoveDir::Leftward);
			break;
		case KEY_D:
			m_moveFlag &= ~(CharMoveDir::Rightward);
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		x = (float)LOWORD(lParam);
		y = (float)HIWORD(lParam);
		width = GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
		height = GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
		m_pCharacter->OnRotate({ x / width, (height - y) / height }, true);
		break;
	case WM_RBUTTONUP:
		x = (float)LOWORD(lParam);
		y = (float)HIWORD(lParam);
		width = GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
		height = GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
		m_pCharacter->OnRotate({ x / width, (height - y) / height }, false);
		break;
	case WM_MOUSEMOVE:
		if (wParam & MK_RBUTTON)
		{
			x = (float)LOWORD(lParam);
			y = (float)HIWORD(lParam);
			width = GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
			height = GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
			m_pCharacter->OnRotate({ x / width, (height - y) / height }, true);
		}
		break;
	}
}

#endif

void VulkanGlobal::InitQueue()
{
}

void VulkanGlobal::InitSurface()
{
}

void VulkanGlobal::InitSwapchain()
{
}

void VulkanGlobal::Update()
{
#if defined(_WIN32)
	static uint32_t frameCount = 0;
	static float fpsTimer = 0;
	MSG msg;
	bool quitMessageReceived = false;
	while (!quitMessageReceived)
	{
		auto tStart = std::chrono::high_resolution_clock::now();
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				quitMessageReceived = true;
				GetSwapChain()->SetInvalid(true);
				break;
			}
		}
		Draw();
		frameCount++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

		fpsTimer += (float)tDiff;
		if (fpsTimer > 1000.0f)
		{
			std::stringstream ss;
			ss << "FPS:" << frameCount;
			SetWindowText(m_hWindow, ss.str().c_str());
			fpsTimer = 0.0f;
			frameCount = 0.0f;
		}
	}
#endif
}

void VulkanGlobal::InitCommandPool()
{
	m_pCommandPool = CommandPool::Create(m_pDevice);
}

void VulkanGlobal::InitSetupCommandBuffer()
{
}

void VulkanGlobal::InitMemoryMgr()
{
}

void VulkanGlobal::InitSwapchainImgs()
{
}

void VulkanGlobal::InitDepthStencil()
{
	m_pDSBuffer = DepthStencilBuffer::Create(m_pDevice);
}

void VulkanGlobal::InitRenderpass()
{
	std::vector<VkAttachmentDescription> attachmentDescs(2);
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescs[0].format = m_pPhysicalDevice->GetSurfaceFormat().format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].format = m_pPhysicalDevice->GetDepthStencilFormat();
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference colorAttach = {};
	colorAttach.attachment = 0;
	colorAttach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference dsAttach = {};
	dsAttach.attachment = 1;
	dsAttach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttach;
	subpass.pDepthStencilAttachment = &dsAttach;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstSubpass = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;
	renderpassCreateInfo.dependencyCount = 1;
	renderpassCreateInfo.pDependencies = &subpassDependency;

	m_pRenderPass = RenderPass::Create(m_pDevice, renderpassCreateInfo);

	attachmentDescs[0].format = FrameBuffer::OFFSCREEN_HDR_COLOR_FORMAT;
	attachmentDescs[1].format = FrameBuffer::OFFSCREEN_DEPTH_STENCIL_FORMAT;
	m_pOffscreenRenderPass = RenderPass::Create(m_pDevice, renderpassCreateInfo);
}

void VulkanGlobal::InitFrameBuffer()
{
	m_framebuffers.resize(GlobalDeviceObjects::GetInstance()->GetSwapChain()->GetSwapChainImageCount());
	for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		m_framebuffers[i] = FrameBuffer::Create(m_pDevice, GlobalDeviceObjects::GetInstance()->GetSwapChain()->GetSwapChainImage(i), m_pDSBuffer, m_pRenderPass);

	m_pOffscreenFrameBuffer = FrameBuffer::CreateOffScreenFrameBuffer(m_pDevice, 512, 512, m_pOffscreenRenderPass);
}

void VulkanGlobal::InitVertices()
{
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;
	pScene = imp.ReadFile("../data/textures/cerberus/cerberus.fbx", aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	ASSERTION(pScene != nullptr);
	
	aiMesh* pMesh = pScene->mMeshes[0];
	uint32_t vertexSize = 0;
	if (pMesh->HasPositions())
		vertexSize += 3 * sizeof(float);
	if (pMesh->HasNormals())
		vertexSize += 3 * sizeof(float);
	//FIXME: hard-coded index 0 here, we don't have more than 1 color for now
	if (pMesh->HasVertexColors(0))
		vertexSize += 4 * sizeof(float);
	//FIXME: hard-coded index 0 here, we don't have more than 1 texture coord for now
	if (pMesh->HasTextureCoords(0))
		vertexSize += 3 * sizeof(float);
	if (pMesh->HasTangentsAndBitangents())
		vertexSize += 6 * sizeof(float);

	float* pVertices = new float[pMesh->mNumVertices * vertexSize / sizeof(float)];
	uint32_t verticesNumBytes = pMesh->mNumVertices * vertexSize;
	uint32_t count = 0;
	
	for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
	{
		uint32_t offset = i * vertexSize / sizeof(float);
		count = 0;
		if (pMesh->HasPositions())
		{
			pVertices[offset] = pMesh->mVertices[i].x;
			pVertices[offset + 1] = pMesh->mVertices[i].y;
			pVertices[offset + 2] = pMesh->mVertices[i].z;
			count += 3;
		}
		if (pMesh->HasNormals())
		{
			pVertices[offset + count] = pMesh->mNormals[i].x;
			pVertices[offset + count + 1] = pMesh->mNormals[i].y;
			pVertices[offset + count + 2] = pMesh->mNormals[i].z;
			count += 3;
		}
		if (pMesh->HasVertexColors(0))
		{
			pVertices[offset + count] = pMesh->mColors[i][0].r;
			pVertices[offset + count + 1] = pMesh->mColors[i][0].g;
			pVertices[offset + count + 2] = pMesh->mColors[i][0].b;
			pVertices[offset + count + 3] = pMesh->mColors[i][0].a;
			count += 4;
		}
		if (pMesh->HasTextureCoords(0))
		{
			pVertices[offset + count] = pMesh->mTextureCoords[0][i].x;
			pVertices[offset + count + 1] = pMesh->mTextureCoords[0][i].y;
			pVertices[offset + count + 2] = pMesh->mTextureCoords[0][i].z;
			count += 3;
		}
		if (pMesh->HasTangentsAndBitangents())
		{
			pVertices[offset + count] = pMesh->mTangents[i].x;
			pVertices[offset + count + 1] = pMesh->mTangents[i].y;
			pVertices[offset + count + 2] = pMesh->mTangents[i].z;
			pVertices[offset + count + 3] = pMesh->mBitangents[i].x;
			pVertices[offset + count + 4] = pMesh->mBitangents[i].y;
			pVertices[offset + count + 5] = pMesh->mBitangents[i].z;
			count += 6;
		}
	}
	
	uint32_t* pIndices = new uint32_t[pMesh->mNumFaces * 3];
	uint32_t indicesNumBytes = pMesh->mNumFaces * 3 * sizeof(uint32_t);
	for (size_t i = 0; i < pMesh->mNumFaces; i++)
	{
		pIndices[i * 3] = pMesh->mFaces[i].mIndices[0];
		pIndices[i * 3 + 1] = pMesh->mFaces[i].mIndices[1];
		pIndices[i * 3 + 2] = pMesh->mFaces[i].mIndices[2];
	}

	//Binding and attributes information
	VkVertexInputBindingDescription bindingDesc = {};
	bindingDesc.binding = 0;		//hard coded 0
	bindingDesc.stride = vertexSize;	//xyzrgb, all hard coded, mockup code, don't care, just for learning
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attribDesc;
	attribDesc.resize(5);

	uint32_t offset = 0;
	attribDesc[0].binding = 0;
	attribDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[0].location = 0;	//layout location 0 in shader
	attribDesc[0].offset = offset;
	offset += sizeof(float) * 3;

	attribDesc[1].binding = 0;
	attribDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[1].location = 1;
	attribDesc[1].offset = offset;	//after xyz*/
	offset += sizeof(float) * 3;

	attribDesc[2].binding = 0;
	attribDesc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[2].location = 2;
	attribDesc[2].offset = offset;	//after xyz*/
	offset += sizeof(float) * 3;

	attribDesc[3].binding = 0;
	attribDesc[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[3].location = 3;
	attribDesc[3].offset = offset;	//after xyz*/
	offset += sizeof(float) * 3;

	attribDesc[4].binding = 0;
	attribDesc[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[4].location = 4;
	attribDesc[4].offset = offset;	//after xyz*/
	offset += sizeof(float) * 3;

	m_pVertexBuffer = VertexBuffer::Create(m_pDevice, verticesNumBytes, bindingDesc, attribDesc);
	m_pVertexBuffer->UpdateByteStream(pVertices, 0, verticesNumBytes, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	m_pIndexBuffer = IndexBuffer::Create(m_pDevice, indicesNumBytes, VK_INDEX_TYPE_UINT32);
	m_pIndexBuffer->UpdateByteStream(pIndices, 0, indicesNumBytes, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

	float cubeVertices[] = {
		// front
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// back
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
	};

	uint32_t cubeIndices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 0, 3,
		3, 7, 4,
		// left
		4, 5, 1,
		1, 0, 4,
		// right
		3, 2, 6,
		6, 7, 3,
	};

	bindingDesc = {};
	bindingDesc.binding = 0;
	bindingDesc.stride = 3 * sizeof(float);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	attribDesc.clear();
	attribDesc.resize(1);
	attribDesc[0].binding = 0;
	attribDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc[0].location = 0;	//layout location 0 in shader
	attribDesc[0].offset = 0;

	m_pCubeVertexBuffer = VertexBuffer::Create(m_pDevice, sizeof(cubeVertices), bindingDesc, attribDesc);
	m_pCubeVertexBuffer->UpdateByteStream(cubeVertices, 0, sizeof(cubeVertices), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	m_pCubeIndexBuffer = IndexBuffer::Create(m_pDevice, sizeof(cubeIndices), VK_INDEX_TYPE_UINT32);
	m_pCubeIndexBuffer->UpdateByteStream(cubeIndices, 0, sizeof(cubeIndices), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
}

void VulkanGlobal::InitUniforms()
{
	uint32_t minAlign = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	uint32_t alignedBytes = sizeof(GlobalUniforms) / minAlign * minAlign + (sizeof(GlobalUniforms) % minAlign > 0 ? minAlign : 0);
	uint32_t totalUniformBytes = alignedBytes * GetSwapChain()->GetSwapChainImageCount();
	m_pUniformBuffer = UniformBuffer::Create(m_pDevice, totalUniformBytes);

	m_pAlbedo = Texture2D::Create(m_pDevice, "../data/textures/cerberus/albedo.ktx", VK_FORMAT_R8G8B8A8_UNORM);
	m_pAmbientOcclusion = Texture2D::Create(m_pDevice, "../data/textures/cerberus/ao.ktx", VK_FORMAT_R8_UNORM);
	m_pMetalic = Texture2D::Create(m_pDevice, "../data/textures/cerberus/metallic.ktx", VK_FORMAT_R8_UNORM);
	m_pNormal = Texture2D::Create(m_pDevice, "../data/textures/cerberus/normal.ktx", VK_FORMAT_R8G8B8A8_UNORM);
	m_pRoughness = Texture2D::Create(m_pDevice, "../data/textures/cerberus/roughness.ktx", VK_FORMAT_R8_UNORM);
	m_pSkyBoxTex = TextureCube::Create(m_pDevice, "../data/textures/hdr/gcanyon_cube.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);
}

void VulkanGlobal::InitDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> dsLayoutBindings = 
	{
		{
			0,	//binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	//type
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		},
		{
			1,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			2,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			3,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			4,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
		{
			5,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		}
	};

	m_pDescriptorSetLayout = DescriptorSetLayout::Create(m_pDevice, dsLayoutBindings);

	DescriptorSetLayoutList list =
	{
		m_pDescriptorSetLayout,
	};
	m_pPipelineLayout = PipelineLayout::Create(m_pDevice, list);

	dsLayoutBindings =
	{
		{
			0,	//binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	//type
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		},
		{
			1,	//binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	//type
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		},
	};

	m_pSkyBoxDSLayout = DescriptorSetLayout::Create(m_pDevice, dsLayoutBindings);

	list =
	{
		m_pSkyBoxDSLayout,
	};
	m_pSkyBoxPLayout = PipelineLayout::Create(m_pDevice, list);
}

void VulkanGlobal::InitPipelineCache()
{
	/*
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	CHECK_VK_ERROR(vkCreatePipelineCache(m_pDevice->GetDeviceHandle(), &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
	*/
}

void VulkanGlobal::InitPipeline()
{
	GraphicPipeline::SimplePipelineStateCreateInfo info = {};
	info.pRenderPass = m_pRenderPass;
	info.pPipelineLayout = m_pPipelineLayout;
	info.pVertShader = m_pVertShader;
	info.pFragShader = m_pFragShader;
	info.vertexBindingsInfo = { m_pVertexBuffer->GetBindingDesc() };
	info.vertexAttributesInfo = m_pVertexBuffer->GetAttribDesc();

	m_pPipeline = GraphicPipeline::Create(m_pDevice, info);

	info = {};
	info.pRenderPass = m_pRenderPass;
	info.pPipelineLayout = m_pSkyBoxPLayout;
	info.pVertShader = m_pSkyBoxVS;
	info.pFragShader = m_pSkyBoxFS;
	info.vertexBindingsInfo = { m_pCubeVertexBuffer->GetBindingDesc() };
	info.vertexAttributesInfo = m_pCubeVertexBuffer->GetAttribDesc();

	m_pSkyBoxPipeline = GraphicPipeline::Create(m_pDevice, info);
}

void VulkanGlobal::InitShaderModule()
{
	m_pVertShader = ShaderModule::Create(m_pDevice, L"../data/shaders/simple.vert.spv");
	m_pFragShader = ShaderModule::Create(m_pDevice, L"../data/shaders/simple.frag.spv");

	m_pSkyBoxVS = ShaderModule::Create(m_pDevice, L"../data/shaders/sky_box.vert.spv");
	m_pSkyBoxFS = ShaderModule::Create(m_pDevice, L"../data/shaders/sky_box.frag.spv");
}

void VulkanGlobal::InitDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> descPoolSize =
	{
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10
		}
	};

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pPoolSizes = descPoolSize.data();
	descPoolInfo.poolSizeCount = descPoolSize.size();
	descPoolInfo.maxSets = 10;

	m_pDescriptorPool = DescriptorPool::Create(m_pDevice, descPoolInfo);
}

void VulkanGlobal::InitDescriptorSet()
{
	m_pDescriptorSet = m_pDescriptorPool->AllocateDescriptorSet(m_pDescriptorSetLayout);
	m_pDescriptorSet->UpdateBufferDynamic(0, m_pUniformBuffer->GetDescBufferInfo());

	m_pDescriptorSet->UpdateImage(1, m_pAlbedo->GetDescriptorInfo());
	m_pDescriptorSet->UpdateImage(2, m_pNormal->GetDescriptorInfo());
	m_pDescriptorSet->UpdateImage(3, m_pRoughness->GetDescriptorInfo());
	m_pDescriptorSet->UpdateImage(4, m_pMetalic->GetDescriptorInfo());
	m_pDescriptorSet->UpdateImage(5, m_pAmbientOcclusion->GetDescriptorInfo());

	m_pSkyBoxDS = m_pDescriptorPool->AllocateDescriptorSet(m_pSkyBoxDSLayout);
	m_pSkyBoxDS->UpdateBufferDynamic(0, m_pUniformBuffer->GetDescBufferInfo());
	m_pSkyBoxDS->UpdateImage(1, m_pSkyBoxTex->GetDescriptorInfo());
}

void VulkanGlobal::InitDrawCmdBuffers()
{
	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		m_perFrameRes.push_back(FrameMgr()->AllocatePerFrameResource(i));
		//m_drawCmdBuffers.push_back(m_perFrameRes[i]->AllocateCommandBuffer());
		//PrepareDrawCommandBuffer(m_drawCmdBuffers[i], i);
	}
}

void VulkanGlobal::InitSemaphore()
{
}

void VulkanGlobal::EndSetup()
{
	GlobalDeviceObjects::GetInstance()->GetStagingBufferMgr()->FlushDataMainThread();

	CameraInfo camInfo =
	{
		3.1415f / 3.0f,
		1024.0f / 768.0f,
		1.0f,
		2000.0f,
	};
	m_pCameraComp = Camera::Create(camInfo);
	m_pCameraObj = BaseObject::Create();
	m_pCameraObj->AddComponent(m_pCameraComp);

	m_pCameraObj->SetPos({ 0, 0, 50 });
	m_pCameraObj->Update(0);
	m_pCameraComp->Update(0);

	Vector3f up = { 0, 1, 0 };
	Vector3f look = { 0, 0, -1 };
	look.Normalize();
	Vector3f xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3f yaxis = look ^ xaxis;
	yaxis.Normalize();

	Matrix3f rotation;
	rotation.c[0] = xaxis;
	rotation.c[1] = yaxis;
	rotation.c[2] = look;
	m_pCameraObj->SetRotation(rotation);

	m_pCharacter = Character::Create({30.0f}, m_pCameraComp);
	m_pCameraObj->AddComponent(m_pCharacter);
}

void VulkanGlobal::UpdateUniforms(uint32_t frameIndex)
{
	m_pCameraObj->Update(0);
	m_pCameraObj->LateUpdate(0);

	memset(&m_globalUniforms, 0, sizeof(GlobalUniforms));

	Matrix4f model;

	Matrix4f view;
	Vector3f up = { 0, 1, 0 };
	Vector3f look = { 0, 0, -1 };
	look.Normalize();
	Vector3f position = { 0, 0, 50 };
	Vector3f xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3f yaxis = look ^ xaxis;
	yaxis.Normalize();

	view.c[0] = xaxis;
	view.c[1] = yaxis;
	view.c[2] = look;
	view.c[3] = position;
	view.Inverse();

	Matrix4f projection;
	float aspect = 1024.0 / 768.0;
	float fov = 3.1415 / 2.0;
	float nearPlane = 1;
	float farPlane = 200;

	projection.c[0] = { 1.0f / (nearPlane * std::tanf(fov / 2.0f) * aspect), 0, 0, 0 };
	projection.c[1] = { 0, 1.0f / (nearPlane * std::tanf(fov / 2.0f)), 0, 0 };
	projection.c[2] = { 0, 0, (nearPlane + farPlane) / (nearPlane - farPlane), -1 };
	projection.c[3] = { 0, 0, 2.0f * nearPlane * farPlane / (nearPlane - farPlane), 0 };

	Matrix4f vulkanNDC;
	vulkanNDC.c[1].y = -1.0f;
	vulkanNDC.c[2].z = vulkanNDC.c[3].z = 0.5f;

	Matrix4f mvp = vulkanNDC * m_pCameraComp->GetProjMatrix() * m_pCameraComp->GetViewMatrix() * model;

	Vector3f camPos = m_pCameraComp->GetObjectA()->GetWorldPosition();

	memcpy_s(m_globalUniforms.model, sizeof(m_globalUniforms.model), &model, sizeof(model));
	//memcpy_s(m_globalUniforms.view, sizeof(m_globalUniforms.view), &view, sizeof(view));
	memcpy_s(m_globalUniforms.view, sizeof(m_globalUniforms.view), &m_pCameraComp->GetViewMatrix(), sizeof(m_pCameraComp->GetViewMatrix()));
	//memcpy_s(m_globalUniforms.projection, sizeof(m_globalUniforms.projection), &projection, sizeof(projection));
	memcpy_s(m_globalUniforms.projection, sizeof(m_globalUniforms.projection), &m_pCameraComp->GetProjMatrix(), sizeof(m_pCameraComp->GetProjMatrix()));
	memcpy_s(m_globalUniforms.vulkanNDC, sizeof(m_globalUniforms.vulkanNDC), &vulkanNDC, sizeof(vulkanNDC));
	memcpy_s(m_globalUniforms.mvp, sizeof(m_globalUniforms.mvp), &mvp, sizeof(mvp));
	memcpy_s(m_globalUniforms.camPos, sizeof(m_globalUniforms.camPos), &camPos, sizeof(camPos));
	memcpy_s(&m_globalUniforms.roughness, sizeof(m_globalUniforms.roughness), &m_roughness, sizeof(m_roughness));

	uint32_t totalUniformBytes = m_pUniformBuffer->GetDescBufferInfo().range / GetSwapChain()->GetSwapChainImageCount();
	m_pUniformBuffer->UpdateByteStream(&m_globalUniforms, totalUniformBytes * frameIndex, sizeof(m_globalUniforms), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
}

void VulkanGlobal::PrepareDrawCommandBuffer(const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{
	std::unique_lock<std::mutex> lock(m_updateMutex);

	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = pPerFrameRes->AllocateCommandBuffer();

	std::vector<VkClearValue> clearValues = 
	{
		{ 0.2f, 0.2f, 0.2f, 0.2f },
		{ 1.0f, 0 }
	};

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CHECK_VK_ERROR(vkBeginCommandBuffer(pDrawCmdBuffer->GetDeviceHandle(), &cmdBeginInfo));

	UpdateUniforms(pPerFrameRes->GetFrameIndex());
	StagingBufferMgr()->RecordDataFlush(pDrawCmdBuffer);

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.clearValueCount = clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();
	renderPassBeginInfo.renderPass = m_pRenderPass->GetDeviceHandle();
	renderPassBeginInfo.framebuffer = m_framebuffers[pPerFrameRes->GetFrameIndex()]->GetDeviceHandle();
	renderPassBeginInfo.renderArea.extent.width = GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	renderPassBeginInfo.renderArea.extent.height = GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;

	vkCmdBeginRenderPass(pDrawCmdBuffer->GetDeviceHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	pDrawCmdBuffer->AddToReferenceTable(m_pRenderPass);
	pDrawCmdBuffer->AddToReferenceTable(m_framebuffers[pPerFrameRes->GetFrameIndex()]);

	VkViewport viewport =
	{
		0, 0,
		GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height
	};

	vkCmdSetViewport(pDrawCmdBuffer->GetDeviceHandle(), 0, 1, &viewport);
	vkCmdSetScissor(pDrawCmdBuffer->GetDeviceHandle(), 0, 1, &scissorRect);

	std::vector<VkDescriptorSet> dsSets = { m_pDescriptorSet->GetDeviceHandle() };

	uint32_t offset = FrameMgr()->FrameIndex() * m_pUniformBuffer->GetDescBufferInfo().range / GetSwapChain()->GetSwapChainImageCount();

	vkCmdBindDescriptorSets(pDrawCmdBuffer->GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipelineLayout->GetDeviceHandle(), 0, dsSets.size(), dsSets.data(), 1, &offset);
	pDrawCmdBuffer->AddToReferenceTable(m_pPipelineLayout);
	pDrawCmdBuffer->AddToReferenceTable(m_pDescriptorSet);

	vkCmdBindPipeline(pDrawCmdBuffer->GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->GetDeviceHandle());
	pDrawCmdBuffer->AddToReferenceTable(m_pPipeline);

	std::vector<VkBuffer> vertexBuffers = { m_pVertexBuffer->GetDeviceHandle() };
	std::vector<VkDeviceSize> offsets = { 0 };
	vkCmdBindVertexBuffers(pDrawCmdBuffer->GetDeviceHandle(), 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
	vkCmdBindIndexBuffer(pDrawCmdBuffer->GetDeviceHandle(), m_pIndexBuffer->GetDeviceHandle(), 0, m_pIndexBuffer->GetType());
	pDrawCmdBuffer->AddToReferenceTable(m_pVertexBuffer);
	pDrawCmdBuffer->AddToReferenceTable(m_pIndexBuffer);

	vkCmdDrawIndexed(pDrawCmdBuffer->GetDeviceHandle(), m_pIndexBuffer->GetCount(), 1, 0, 0, 0);


	dsSets = { m_pSkyBoxDS->GetDeviceHandle() };

	vkCmdBindDescriptorSets(pDrawCmdBuffer->GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pSkyBoxPLayout->GetDeviceHandle(), 0, dsSets.size(), dsSets.data(), 1, &offset);
	pDrawCmdBuffer->AddToReferenceTable(m_pSkyBoxPLayout);
	pDrawCmdBuffer->AddToReferenceTable(m_pSkyBoxDS);

	vkCmdBindPipeline(pDrawCmdBuffer->GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pSkyBoxPipeline->GetDeviceHandle());
	pDrawCmdBuffer->AddToReferenceTable(m_pSkyBoxPipeline);

	vertexBuffers = { m_pCubeVertexBuffer->GetDeviceHandle() };
	offsets = { 0 };
	vkCmdBindVertexBuffers(pDrawCmdBuffer->GetDeviceHandle(), 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
	vkCmdBindIndexBuffer(pDrawCmdBuffer->GetDeviceHandle(), m_pCubeIndexBuffer->GetDeviceHandle(), 0, m_pCubeIndexBuffer->GetType());
	pDrawCmdBuffer->AddToReferenceTable(m_pCubeVertexBuffer);
	pDrawCmdBuffer->AddToReferenceTable(m_pCubeIndexBuffer);

	vkCmdDrawIndexed(pDrawCmdBuffer->GetDeviceHandle(), m_pCubeIndexBuffer->GetCount(), 1, 0, 0, 0);




	vkCmdEndRenderPass(pDrawCmdBuffer->GetDeviceHandle());
	
	CHECK_VK_ERROR(vkEndCommandBuffer(pDrawCmdBuffer->GetDeviceHandle()));
	
	std::vector<VkPipelineStageFlags> waitFlags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	FrameMgr()->CacheSubmissioninfo(GlobalGraphicQueue(), { pDrawCmdBuffer }, waitFlags, false);
}

void VulkanGlobal::Draw()
{
	GetSwapChain()->AcquireNextImage();
	m_pCharacter->Move(m_moveFlag, 0.001f);
	FrameMgr()->AddJobToFrame(std::bind(&VulkanGlobal::PrepareDrawCommandBuffer, this, std::placeholders::_1));
	GetSwapChain()->QueuePresentImage(GlobalObjects()->GetPresentQueue());
}

void VulkanGlobal::Init(HINSTANCE hInstance, WNDPROC wndproc)
{
	SetupWindow(hInstance, wndproc);
	InitVulkanInstance();
	InitPhysicalDevice(m_hPlatformInst, m_hWindow);
	InitSurface();
	InitVulkanDevice();
	GlobalDeviceObjects::GetInstance()->Init(m_pDevice);
	InitSwapchain();
	InitQueue();

	InitCommandPool();
	InitSetupCommandBuffer();
	InitMemoryMgr();
	InitSwapchainImgs();
	InitDepthStencil();
	InitRenderpass();
	InitFrameBuffer();
	InitVertices();
	InitUniforms();
	InitDescriptorSetLayout();
	InitShaderModule();
	InitPipelineCache();
	InitPipeline();
	InitDescriptorPool();
	InitDescriptorSet();
	InitDrawCmdBuffers();
	InitSemaphore();
	EndSetup();
}
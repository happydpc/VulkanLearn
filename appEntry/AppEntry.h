#pragma once
#include "../common/Singleton.h"
#include "vulkan.h"
#include "../vulkan/PhysicalDevice.h"
#include "../vulkan/Device.h"
#include "../vulkan/SwapChain.h"
#include <vector>
#include <memory>
#include "../vulkan/DeviceMemoryManager.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/VertexBuffer.h"
#include "../vulkan/IndexBuffer.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChainImage.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/DescriptorSetLayout.h"
#include "../vulkan/PipelineLayout.h"
#include "../vulkan/DescriptorPool.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/ShaderModule.h"
#include "../vulkan/GraphicPipeline.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/Semaphore.h"
#include "../class/PerFrameResource.h"
#include "../thread/ThreadTaskQueue.hpp"
#include "../Base/BaseObject.h"
#include "../component/Character.h"
#include "../class/Mesh.h"
#include "../component/MeshRenderer.h"
#include "../class/Material.h"
#include "../class/MaterialInstance.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "../vulkan/Image.h"
#include "../class/ForwardMaterial.h"
#include "../class/DeferredMaterial.h"
#include "../component/DirectionLight.h"
#include "../class/ShadowMapMaterial.h"
#include "../class/PostProcessingMaterial.h"
#include "../component/PhysicalCamera.h"
#include "../component/PlanetGenerator.h"

class AppEntry : public Singleton<AppEntry>
{
	const static uint32_t OffScreenSize = 512;

public:
#if defined(_WIN32)
	void InitVulkan(HINSTANCE hInstance, WNDPROC wndproc);
#endif
	void InitVulkanInstance();
	void InitPhysicalDevice(HINSTANCE hInstance, HWND hWnd);
	void InitVulkanDevice();
#if defined(_WIN32)
	void SetupWindow(HINSTANCE hInstance, WNDPROC wndproc);
	void HandleMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	void InitVertices();
	void InitUniforms();
	void InitDrawCmdBuffers();
	void InitMaterials();
	void InitScene();
	void EndSetup();

	void Tick();
	void Update();

public:
	std::shared_ptr<Instance>			m_pVulkanInstance;
	std::shared_ptr<PhysicalDevice>		m_pPhysicalDevice;
	std::shared_ptr<Device>				m_pDevice;

	std::shared_ptr<CommandBuffer>		m_pPrePresentCmdBuffer;
	std::shared_ptr<CommandBuffer>		m_pPostPresentCmdBuffer;


	std::shared_ptr<Mesh>				m_pGunMesh;
	std::shared_ptr<Mesh>				m_pSophiaMesh;
	std::vector<std::shared_ptr<Mesh>>	m_innerBallMeshes;
	std::vector<std::shared_ptr<Mesh>>	m_bunnyMeshes;
	std::vector<std::shared_ptr<Mesh>>	m_dragonMeshes;
	uint32_t							m_LODPatchLevel;
	std::shared_ptr<Mesh>				m_pLODTriangleMesh;
	std::shared_ptr<Mesh>				m_pSphereMesh;
	std::shared_ptr<Mesh>				m_pCubeMesh;
	std::shared_ptr<Mesh>				m_pQuadMesh;
	std::shared_ptr<Mesh>				m_pPBRBoxMesh;

	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;

	//std::vector<VkCommandBuffer>		m_drawCmdBuffers;
	std::vector<std::shared_ptr<CommandBuffer>>		m_drawCmdBuffers;

	std::vector<std::shared_ptr<PerFrameResource>> m_perFrameRes;

	std::shared_ptr<ThreadTaskQueue>	m_pThreadTaskQueue;
	std::mutex							m_updateMutex;

	std::shared_ptr<BaseObject>			m_pCameraObj;
	std::shared_ptr<PhysicalCamera>		m_pCameraComp;
	std::shared_ptr<Character>			m_pCharacter;

	std::shared_ptr<BaseObject>			m_pDirLightObj;
	std::shared_ptr<DirectionLight>		m_pDirLight;

	std::shared_ptr<PlanetGenerator>	m_pPlanetGenerator;

	std::shared_ptr<Texture2D>			m_pAlbedoRoughness;
	std::shared_ptr<Texture2D>			m_pNormalAO;
	std::shared_ptr<Texture2D>			m_pMetalic;

	std::shared_ptr<BaseObject>			m_pGunObject;
	std::shared_ptr<BaseObject>			m_pSphere0;
	std::shared_ptr<BaseObject>			m_pSphere1;
	std::shared_ptr<BaseObject>			m_pSphere2;
	std::shared_ptr<BaseObject>			m_pSphere3;
	std::shared_ptr<BaseObject>			m_pSphere4;
	std::vector<std::shared_ptr<BaseObject>> m_innerBallObjects;
	std::shared_ptr<BaseObject>			m_pInnerBall;
	std::vector<std::shared_ptr<BaseObject>> m_bunnyObjects;
	std::shared_ptr<BaseObject>			m_pBunny;
	std::vector<std::shared_ptr<BaseObject>> m_dragonObjects;
	std::shared_ptr<BaseObject>			m_pDragon;
	std::shared_ptr<BaseObject>			m_pBoxObject0;
	std::shared_ptr<BaseObject>			m_pBoxObject1;
	std::shared_ptr<BaseObject>			m_pBoxObject2;
	std::shared_ptr<BaseObject>			m_pBoxObject3;
	std::shared_ptr<BaseObject>			m_pBoxObject4;
	std::shared_ptr<BaseObject>			m_pSophiaObject;
	std::shared_ptr<MeshRenderer>		m_pGunMeshRenderer;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer0;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer1;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer2;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer3;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer4;
	std::vector<std::shared_ptr<MeshRenderer>>	m_innerBallRenderers;
	std::vector<std::shared_ptr<MeshRenderer>>	m_bunnyRenderers;
	std::vector<std::shared_ptr<MeshRenderer>>	m_dragonRenderers;
	std::shared_ptr<MeshRenderer>		m_pQuadRenderer;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer0;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer1;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer2;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer3;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer4;
	std::vector<std::shared_ptr<MeshRenderer>> m_boneBoxRenderers;
	std::vector<std::shared_ptr<MaterialInstance>> m_boneBoxMaterialInstances;
	std::shared_ptr<MeshRenderer>		m_pSophiaRenderer;
	std::shared_ptr<MeshRenderer>       m_pPlanetRenderer;

	std::shared_ptr<MaterialInstance>	m_pGunMaterialInstance;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance0;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance1;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance2;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance3;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance4;
	std::vector<std::shared_ptr<MaterialInstance>> m_innerBallMaterialInstances;
	std::vector<std::shared_ptr<MaterialInstance>> m_bunnyMaterialInstances;
	std::vector<std::shared_ptr<MaterialInstance>> m_dragonMaterialInstances;
	std::shared_ptr<MaterialInstance>	m_pQuadMaterialInstance;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance0;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance1;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance2;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance3;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance4;
	std::shared_ptr<MaterialInstance>	m_pSophiaMaterialInstance;
	std::shared_ptr<MaterialInstance>   m_pPlanetMaterialInstance;

	std::shared_ptr<MaterialInstance>	m_pShadowMapMaterialInstance;
	std::shared_ptr<MaterialInstance>	m_pSkinnedShadowMapMaterialInstance;

	std::shared_ptr<BaseObject>			m_pSkyBoxObject;
	std::shared_ptr<MeshRenderer>		m_pSkyBoxMeshRenderer;
	std::shared_ptr<ForwardMaterial>	m_pSkyBoxMaterial;
	std::shared_ptr<MaterialInstance>	m_pSkyBoxMaterialInstance;

	std::shared_ptr<BaseObject>			m_pQuadObject;

	std::shared_ptr<BaseObject>			m_pTestObject;
	std::shared_ptr<MeshRenderer>		m_pTestRenderer;
	std::shared_ptr<Material>			m_pTestMaterial;
	std::shared_ptr<MaterialInstance>	m_pTestMaterialInstance;

	std::shared_ptr<BaseObject>			m_pPlanetObject;

	std::shared_ptr<BaseObject>			m_pRootObject;
	std::shared_ptr<BaseObject>			m_pSceneRootObject;

	std::vector<std::shared_ptr<CommandBuffer>> m_commandBufferList;

#if defined(_WIN32)
	HINSTANCE							m_hPlatformInst;
	HWND								m_hWindow;
#endif

	void AddBoneBox(const std::shared_ptr<BaseObject>& pObject);
};
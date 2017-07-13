#pragma once
#include "../common/RefCounted.h"
#include "vulkan.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include <vector>
#include <memory>
#include "DeviceObjectBase.h"
#include "SwapChainImage.h"

class Semaphore;
class Queue;
class Fence;

class SwapChain : public DeviceObjectBase<SwapChain>
{
public:
	~SwapChain();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SwapChain>& pSelf);

	const VkSwapchainKHR GetDeviceHandle() const { return m_swapchain; }
	const std::shared_ptr<SwapChainImage> GetSwapChainImage(uint32_t index) { assert(index < m_swapchainImages.size()); return m_swapchainImages[index]; }
	uint32_t GetSwapChainImageCount() const { return m_swapchainImages.size(); }
	uint32_t GetCurrentIndex() const { return m_currentIndex; }

	void AcquireNextImage(const std::shared_ptr<Semaphore>& acquireDone, uint32_t& index) const;
	void QueuePresentImage(const std::shared_ptr<Queue>& pPresentQueue, const std::shared_ptr<Semaphore>& renderDone, uint32_t index);
	
	void EnsureSwapChainImageLayout();

	PFN_vkAcquireNextImageKHR GetAcquireNextImageFuncPtr() const { return m_fpAcquireNextImageKHR; }
	PFN_vkQueuePresentKHR GetQueuePresentFuncPtr() const { return m_fpQueuePresentKHR; }
	PFN_vkGetSwapchainImagesKHR GetGetSwapchainImagesFuncPtr() const { return m_fpGetSwapchainImagesKHR; }

public:
	static std::shared_ptr<SwapChain> Create(const std::shared_ptr<Device>& pDevice);

protected:
	VkSwapchainKHR						m_swapchain;

	PFN_vkCreateSwapchainKHR			m_fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR			m_fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR			m_fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR			m_fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR				m_fpQueuePresentKHR;

	std::vector<std::shared_ptr<SwapChainImage>>	m_swapchainImages;
	std::vector<std::shared_ptr<Fence>>				m_frameFences;
	uint32_t										m_currentIndex;
};
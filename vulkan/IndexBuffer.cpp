#include "IndexBuffer.h"

bool IndexBuffer::Init(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	VkIndexType type)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	info.size = numBytes;

	if (!Buffer::Init(pDevice, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	m_type = type;

	switch (m_type)
	{
	case VK_INDEX_TYPE_UINT16 : 
		m_count = numBytes / sizeof(uint16_t);
		break;
	case VK_INDEX_TYPE_UINT32 :
		m_count = numBytes / sizeof(uint32_t);
		break;
	default:
		assert(false);
		break;
	}

	return true;
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	VkIndexType type)
{
	std::shared_ptr<IndexBuffer> pIndexBuffer = std::make_shared<IndexBuffer>();
	if (pIndexBuffer.get() && pIndexBuffer->Init(pDevice, numBytes, type))
		return pIndexBuffer;
	return nullptr;
}
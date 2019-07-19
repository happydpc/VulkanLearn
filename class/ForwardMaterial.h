#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"

class ForwardMaterial : public Material
{
public:
	static std::shared_ptr<ForwardMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) override;
	void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3f& groupNum, const Vector3f& groupSize, uint32_t pingpong = 0) override {}
	void DrawScreenQuad(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer);

protected:
	FrameBufferDiction::FrameBufferType	m_frameBufferType;
};
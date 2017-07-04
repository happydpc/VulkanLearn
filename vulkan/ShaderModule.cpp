#include "ShaderModule.h"
#include <fstream>

ShaderModule::~ShaderModule()
{
	vkDestroyShaderModule(GetDevice()->GetDeviceHandle(), m_shaderModule, nullptr);
}

bool ShaderModule::Init(const std::shared_ptr<Device>& pDevice, const std::wstring& path)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	std::ifstream ifs;
	ifs.open(path, std::ios::binary);
	if (ifs.fail())
		return false;

	std::vector<char> buffer;
	buffer.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = buffer.size();
	info.pCode = (uint32_t*)buffer.data();
	CHECK_VK_ERROR(vkCreateShaderModule(pDevice->GetDeviceHandle(), &info, nullptr, &m_shaderModule));

	return true;
}

std::shared_ptr<ShaderModule> ShaderModule::Create(const std::shared_ptr<Device>& pDevice, const std::wstring& path)
{
	std::shared_ptr<ShaderModule> pModule = std::make_shared<ShaderModule>();
	if (pModule.get() && pModule->Init(pDevice, path))
		return pModule;
	return nullptr;
}
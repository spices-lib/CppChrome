#pragma once
#include <string>

class CppRenderer 
{
public:

	CppRenderer() = default;
	virtual ~CppRenderer() = default;

	void Init();

	void Render(uint32_t texture);

private:

	uint32_t m_VAO;
	uint32_t m_VBO;
	uint32_t m_EBO;
	uint32_t m_Shader;
};
#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

class Shaders {
public:
	static unsigned loadVertexShader(const char* path);
	static unsigned loadGeometryShader(const char* path);
	static unsigned loadFragmentShader(const char* path);
	static unsigned createProgram(const char* vertex_file_path, const char* fragment_file_path);
	static unsigned createProgramGeom(const char* vertex_file_path, const char* geom_file_path,
			const char* fragment_file_path);
	static unsigned createAndCompileShader(std::string const &code, unsigned shaderType);
	static unsigned linkProgram(unsigned vertexShader, unsigned fragmentShader, unsigned geomShader=0);

private:
	Shaders() {
	}
	static std::string readShaderFile(const char* path);
};

#endif

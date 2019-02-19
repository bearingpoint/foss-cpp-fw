#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <functional>
#include <vector>

using shaderCallback = std::function<void(unsigned shaderId)>;
using programCallback = std::function<void(unsigned programId)>;

class Shaders {
public:
	static void loadVertexShader(const char* path, shaderCallback cb);
	static void loadGeometryShader(const char* path, shaderCallback cb);
	static void loadFragmentShader(const char* path, shaderCallback cb);
	static void createProgram(const char* vertex_file_path, const char* fragment_file_path, programCallback cb);
	static void createProgramGeom(const char* vertex_file_path, const char* geom_file_path,
			const char* fragment_file_path, programCallback cb);
	static unsigned createAndCompileShader(std::string const &code, unsigned shaderType);
	static unsigned linkProgram(unsigned vertexShader, unsigned fragmentShader, unsigned geomShader=0);

	// reloads all shaders that were loaded from files, recompiles them and calls the callbacks again with the new values
	static void reloadAllShaders();

private:
	Shaders() {}
	static std::string readShaderFile(const char* path);

	struct shaderDesc {
		unsigned shaderType;
		unsigned shaderId;
		std::string filename;
		shaderCallback callback;
	};

	struct programDesc {
		unsigned programId = 0;
		int vertexDescIdx = -1;
		int fragDescIdx = -1;
		int geomDescIdx = -1;
		programCallback callback;
	};

	static std::vector<shaderDesc> loadedShaders_;
	static std::vector<programDesc> loadedPrograms_;
};

#endif

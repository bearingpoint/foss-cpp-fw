#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/utils/log.h>

#include <GL/glew.h>

#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

vector<Shaders::shaderDesc> Shaders::loadedShaders_;
vector<Shaders::programDesc> Shaders::loadedPrograms_;
IShaderPreprocessor* Shaders::pPreprocessor_ = nullptr;

void Shaders::useShaderPreprocessor(IShaderPreprocessor* ppp) {
	pPreprocessor_ = ppp;
}

std::string Shaders::readShaderFile(const char* path) {
	std::ifstream shaderStream(path, std::ios::in);
	if (shaderStream.is_open()) {
		std::string str;
		shaderStream.seekg(0, std::ios::end);
		str.reserve(shaderStream.tellg());
		shaderStream.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(shaderStream)), std::istreambuf_iterator<char>());
		shaderStream.close();
		if (pPreprocessor_ != nullptr)
			str = pPreprocessor_->preprocess(str, path);
		if (str == "") {
			ERROR("Shader preprocessing failed for: " << path);
		}
		return str;
	} else {
		ERROR("Impossible to open file: " << path);
		return "";
	}
}

unsigned Shaders::createAndCompileShader(std::string const &code, unsigned shaderType) {
	unsigned shaderID = glCreateShader(shaderType);
	if (!shaderID || checkGLError("create shader"))
		return 0;
	GLint Result = GL_FALSE;
	// Compile Shader
	const char* sourcePointer = code.c_str();
	glShaderSource(shaderID, 1, &sourcePointer, NULL);
	glCompileShader(shaderID);
	checkGLError("compile shader");

	// Check Shader
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
	if (Result != GL_TRUE) {
		int infoLogLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) {
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
			ERROR("ERROR!!!\n" << &shaderErrorMessage[0]);
		} else {
			ERROR("ERROR!!!\n(no error description)");
		}
		return 0;
	}
	LOGNP("Shader OK\n");
	return shaderID;
}

void Shaders::loadVertexShader(const char* path, shaderCallback cb) {
	string shaderCode = readShaderFile(path);
	if (shaderCode == "") {
		cb(0);
		return;
	}
	LOG("Compiling shader : " << path << " . . . ");
	unsigned id = createAndCompileShader(shaderCode, GL_VERTEX_SHADER);
	loadedShaders_.push_back({
		GL_VERTEX_SHADER,
		id,
		path,
		cb
	});
	cb(id);
}

void Shaders::loadGeometryShader(const char* path, shaderCallback cb) {
	string shaderCode = readShaderFile(path);
	if (shaderCode == "") {
		cb(0);
		return;
	}
	LOG("Compiling shader : " << path << " . . . ");
	unsigned id = createAndCompileShader(shaderCode, GL_GEOMETRY_SHADER);
	loadedShaders_.push_back({
		GL_GEOMETRY_SHADER,
		id,
		path,
		cb
	});
	cb(id);
}
void Shaders::loadFragmentShader(const char* path, shaderCallback cb) {
	string shaderCode = readShaderFile(path);
	if (shaderCode == "") {
		cb(0);
		return;
	}
	LOG("Compiling shader : " << path << " . . . ");
	unsigned id = createAndCompileShader(shaderCode, GL_FRAGMENT_SHADER);
	loadedShaders_.push_back({
		GL_FRAGMENT_SHADER,
		id,
		path,
		cb
	});
	cb(id);
}

void printProgramInfoLog(int programID) {
	int InfoLogLength;
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		ERROR(&ProgramErrorMessage[0]);
	} else {
		ERROR("(no error description)");
	}
}

void Shaders::createProgram(const char* vertex_file_path, const char* fragment_file_path, programCallback cb) {
	createProgramGeom(vertex_file_path, nullptr, fragment_file_path, cb);
}
void Shaders::createProgramGeom(const char* vertex_file_path, const char* geom_file_path,
		const char* fragment_file_path, programCallback cb) {
	LOGPREFIX("SHADERS");
	// Create the shaders
	unsigned vertexShaderID = 0;
	int vertexDescIdx = -1;
	loadVertexShader(vertex_file_path, [&](unsigned id) {
		vertexShaderID = id;
		vertexDescIdx = loadedShaders_.size() - 1;
	});
	if (vertexDescIdx >= 0)
		loadedShaders_[vertexDescIdx].callback = nullptr;
	unsigned geomShaderID = 0;
	int geomDescIdx = -1;
	if (geom_file_path != nullptr) {
		loadGeometryShader(geom_file_path, [&](unsigned id) {
			geomShaderID = id;
			geomDescIdx = loadedShaders_.size() - 1;
		});
		if (geomDescIdx >= 0)
			loadedShaders_[geomDescIdx].callback = nullptr;
	}
	unsigned fragmentShaderID = 0;
	int fragmentDescIdx = -1;
	loadFragmentShader(fragment_file_path, [&](unsigned id) {
		fragmentShaderID = id;
		fragmentDescIdx = loadedShaders_.size() - 1;
	});
	if (fragmentDescIdx >= 0)
		loadedShaders_[fragmentDescIdx].callback = nullptr;

	if (vertexShaderID == 0 || fragmentShaderID == 0 || (geom_file_path != nullptr && geomShaderID == 0)) {
		LOGLN("Some shaders failed. Aborting...");
		cb(0);
		return;
	}
	unsigned prog = linkProgram(vertexShaderID, fragmentShaderID, geomShaderID);
	// delete shaders:
	glDeleteShader(vertexShaderID);
	loadedShaders_[vertexDescIdx].shaderId = 0;
	if (geomShaderID) {
		glDeleteShader(geomShaderID);
		loadedShaders_[geomDescIdx].shaderId = 0;
	}
	glDeleteShader(fragmentShaderID);
	loadedShaders_[fragmentDescIdx].shaderId = 0;
	checkGLError("delete shaders");

	loadedPrograms_.push_back({
		prog,
		vertexDescIdx,
		fragmentDescIdx,
		geomDescIdx,
		cb
	});

	cb(prog);
}

unsigned Shaders::linkProgram(unsigned vertexShaderID, unsigned fragmentShaderID, unsigned geomShaderID) {
	// Link the program
	LOG("Linking program . . .");
	unsigned programID = glCreateProgram();
	if (checkGLError("create program"))
		return 0;
	glAttachShader(programID, vertexShaderID);
	checkGLError("attach shader");
	if (geomShaderID != 0) {
		glAttachShader(programID, geomShaderID);
		if (checkGLError("attach shader"))
			return 0;
	}
	glAttachShader(programID, fragmentShaderID);
	if (checkGLError("attach shader"))
		return 0;
	glLinkProgram(programID);
	checkGLError("link program");

	// Check the program
	GLint Result = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &Result);
	if (Result != GL_TRUE) {
		ERROR("Shader link ERROR!!!");
		printProgramInfoLog(programID);
		return 0;
	} else {
		LOGNP("OK\n");
		// validate program
		glValidateProgram(programID);
		int validResult;
		glGetProgramiv(programID, GL_VALIDATE_STATUS, &validResult);
		if (validResult != GL_TRUE) {
			ERROR("Shader program validation failed!");
			printProgramInfoLog(programID);
			return 0;
		}
	}
	checkGLError("link program");

	return programID;
}

void Shaders::reloadAllShaders() {
	LOGPREFIX("SHADERS");
	LOGLN("Reloading all shaders . . .");
	for (auto &d : loadedShaders_) {
		if (d.destroyed)
			continue;
		if (d.shaderId)
			glDeleteShader(d.shaderId), d.shaderId = 0;
		string shaderCode = readShaderFile(d.filename.c_str());
		LOG("Compiling shader : " << d.filename << " . . . ");
		d.shaderId = createAndCompileShader(shaderCode, d.shaderType);
		if (d.callback)
			d.callback(d.shaderId);
		checkGLError("reloadShader::compile");
	}
	for (auto &d : loadedPrograms_) {
		if (d.destroyed)
			continue;
		if (d.programId)
			glDeleteProgram(d.programId), d.programId = 0;
#if (0)
		// verbose program linking info
		LOGLN("Linking program from: \n\tVS: " << loadedShaders_[d.vertexDescIdx].filename
					<< "\n\tFS: " << loadedShaders_[d.fragDescIdx].filename);
#endif
		d.programId = linkProgram(d.vertexDescIdx >= 0 ? loadedShaders_[d.vertexDescIdx].shaderId : 0,
									d.fragDescIdx >= 0 ? loadedShaders_[d.fragDescIdx].shaderId : 0,
									d.geomDescIdx >= 0 ? loadedShaders_[d.geomDescIdx].shaderId : 0);
		checkGLError("reloadShader::link");
		if (d.callback)
			d.callback(d.programId);
		checkGLError("reloadShader::callback");
	}
	LOGLN("All shaders reloaded.");
}

void Shaders::deleteLoadedShader(unsigned index) {
	if (loadedShaders_[index].shaderId) {
		glDeleteShader(loadedShaders_[index].shaderId);
		loadedShaders_[index].shaderId = 0;
	}
	loadedShaders_[index].destroyed = true;
	loadedShaders_[index].callback = nullptr;
}

void Shaders::deleteProgram(unsigned programId) {
	assertDbg(programId && "invalid program provided");
	for (auto &d : loadedPrograms_) {
		if (d.programId != programId)
			continue;
		glDeleteProgram(programId);
		d.programId = 0;
		d.destroyed = true;
		d.callback = nullptr;
		if (d.vertexDescIdx >=0 )
			deleteLoadedShader(d.vertexDescIdx);
		if (d.fragDescIdx >=0 )
			deleteLoadedShader(d.fragDescIdx);
		if (d.geomDescIdx >=0 )
			deleteLoadedShader(d.geomDescIdx);

		break;
	}
}

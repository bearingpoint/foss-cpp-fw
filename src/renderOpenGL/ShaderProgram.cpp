#include <boglfw/renderOpenGL/ShaderProgram.h>
#include <boglfw/renderOpenGL/UniformPackProxy.h>

#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>

#include <GL/glew.h>

struct ShaderProgram::VertexAttribDescriptor {
	std::string name;
	int componentType;
	unsigned componentCount;

	static bool validateParams(int type) {
		switch (type) {
			case GL_BYTE:
			case GL_UNSIGNED_BYTE:
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
			case GL_INT:
			case GL_UNSIGNED_INT:
			case GL_FLOAT:
			case GL_HALF_FLOAT:
				return true;
			default:
				return false;
		}
	}
};

ShaderProgram::ShaderProgram() {
}

ShaderProgram::~ShaderProgram() {
	reset();
}

// Assigns a uniform pack to be used by this program.
// This can be called multiple times with different packs, all of which will be used.
void ShaderProgram::useUniformPack(std::shared_ptr<UniformPack> pack) {
	uniformPackProxies_.push_back(UniformPackProxy{pack});
	if (programId_ != 0) {
		// program has already been linked, let's update the uniforms mapping
		uniformPackProxies_.back().updateMappings(programId_);
	}
}

// loads and compiles the shaders, then links the program and fetches all uniform locations that have been mapped
bool ShaderProgram::load(std::string const& vertPath, std::string const& fragPath, std::string const& geomPath) {
	assertDbg(programId_ == 0 && "This ShaderProgram has already been loaded!");
	if (geomPath.empty()) {
		Shaders::createProgram(vertPath.c_str(), fragPath.c_str(), std::bind(&ShaderProgram::onProgramLinked, this, std::placeholders::_1));
	} else {
		Shaders::createProgramGeom(vertPath.c_str(), geomPath.c_str(), fragPath.c_str(), std::bind(&ShaderProgram::onProgramLinked, this, std::placeholders::_1));
	}
#ifdef DEBUG
	vertPath_ = vertPath;
	fragPath_ = fragPath;
	geomPath_ = geomPath;
#endif
	return programId_ != 0;
}

void ShaderProgram::onProgramLinked(unsigned programId) {
	programId_ = programId;
	if (programId != 0) {
		for (auto &pack : uniformPackProxies_)
			pack.updateMappings(programId);

		onProgramReloaded.trigger(std::ref(*this));
	}
}

// sets this program up for rendering. This will also push all uniform values from all
// assigned uniform packs into the openGL pipeline
void ShaderProgram::begin() {
	assertDbg(programId_ != 0 && "This ShaderProgram has not been loaded (or there was a compile/link error)!");
	glUseProgram(programId_);
	for (auto &pack : uniformPackProxies_)
		pack.pushValues();
}

// resets the openGL state after you finished rendering with this program.
void ShaderProgram::end() {
	glUseProgram(0);
}

void ShaderProgram::defineVertexAttrib(std::string name, int componentType, unsigned componentCount) {
	assertDbg(VertexAttribDescriptor::validateParams(componentType) && "Invalid vertex attribute type!");
	vertexAttribs_.push_back({
		name,
		componentType,
		componentCount
	});
}

void ShaderProgram::setupVertexStreams(std::map<std::string, VertexAttribSource> const& mapAttribSource) {
	assertDbg(programId_ != 0 && "This ShaderProgram has not been loaded (or there was a compile/link error)!");
	unsigned boundVBO = 0;
	for (auto &vad : vertexAttribs_) {
		auto it = mapAttribSource.find(vad.name);
		if (it == mapAttribSource.end()) {
			ERROR("Source for attribute '" << vad.name << "' not specified in attribute source map!");
			continue;
		}
		const VertexAttribSource &attrSrc = it->second;
		int location = glGetAttribLocation(programId_, vad.name.c_str());
		if (location >= 0) {
			if (attrSrc.VBO != boundVBO) {
				glBindBuffer(GL_ARRAY_BUFFER, attrSrc.VBO);
				boundVBO = attrSrc.VBO;
			}
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location, vad.componentCount, vad.componentType, GL_FALSE, attrSrc.stride, (void*)attrSrc.offset);
		}
	}
}

int ShaderProgram::getUniformLocation(const char* uName) {
	assertDbg(programId_ != 0 && "This ShaderProgram has not been loaded (or there was a compile/link error)!");
	return glGetUniformLocation(programId_, uName);
}

void ShaderProgram::reset() {
	if (programId_)
		Shaders::deleteProgram(programId_), programId_ = 0;
	uniformPackProxies_.clear();
	vertexAttribs_.clear();
}

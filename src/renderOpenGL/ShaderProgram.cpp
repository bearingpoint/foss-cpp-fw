#include <boglfw/renderOpenGL/ShaderProgram.h>
#include <boglfw/renderOpenGL/UniformPackProxy.h>

#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/utils/assert.h>

#include <GL/glew.h>

ShaderProgram::ShaderProgram() {
}

ShaderProgram::~ShaderProgram() {
}

// Assigns a uniform pack to be used by this program.
// This can be called multiple times with different packs, all of which will be used.
// The method must be called before loading the program.
void ShaderProgram::assignUniformPack(std::shared_ptr<UniformPack> pack) {
	uniformPackProxies_.push_back(UniformPackProxy{pack});
}

// loads and compiles the shaders, then links the program and fetches all uniform locations that have been mapped
void ShaderProgram::load(std::string const& vertPath, std::string const& fragPath, std::string const& geomPath) {
	assertDbg(programId_ == 0 && "This ShaderProgram has already been loaded!");
	if (geomPath.empty()) {
		Shaders::createProgram(vertPath.c_str(), fragPath.c_str(), std::bind(&ShaderProgram::onProgramLinked, this, std::placeholders::_1));
	} else {
		Shaders::createProgramGeom(vertPath.c_str(), geomPath.c_str(), fragPath.c_str(), std::bind(&ShaderProgram::onProgramLinked, this, std::placeholders::_1));
	}
}

void ShaderProgram::onProgramLinked(unsigned programId) {
	programId_ = programId;
	if (programId != 0)
		for (auto &pack : uniformPackProxies_)
			pack.updateMappings(programId);

	onProgramReloaded.trigger(std::ref(*this));
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

int ShaderProgram::getAttribLocation(const char* name) const {
	assertDbg(programId_ != 0 && "This ShaderProgram has not been loaded (or there was a compile/link error)!");
	return glGetAttribLocation(programId_, name);
}

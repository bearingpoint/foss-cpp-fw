#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <boglfw/utils/Event.h>

#include <memory>
#include <vector>

class UniformPack;
class UniformPackProxy;

class ShaderProgram
{
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	// Assigns a uniform pack to be used by this program.
	// This can be called multiple times with different packs, all of which will be used.
	// The method must be called before loading the program.
	void assignUniformPack(std::shared_ptr<UniformPack> pack);

	// loads and compiles the shaders, then links the program and fetches all uniform locations that have been mapped;
	// this should only be called one time.
	void load(std::string const& vertPath, std::string const& fragPath, std::string const& geomPath="");

	// gets an attribute location (only call this after the program has been loaded).
	// The attribute location values may change when the program is reloaded,
	// thus you need to call this again when the [onProgramReloaded] event is fired.
	int getAttribLocation(const char* name) const;

	// sets this program up for rendering. This will also push all uniform values from all
	// assigned uniform packs into the openGL pipeline
	void begin();

	// resets the openGL state after you finished rendering with this program.
	void end();

	// checks whether this object contains a valid openGL shader program that has been successfuly loaded and linked.
	bool isValid() const { return programId_ != 0; }

	// this event is triggered whith a reference to this object every time the shader program has been successfully linked.
	// This happens the first time the program is loaded and on subsequent Shader::reloadAllShaders() calls.
	Event<void(ShaderProgram const&)> onProgramReloaded;

protected:
	unsigned programId_ = 0;
	std::vector<UniformPackProxy> uniformPackProxies_;

	void onProgramLinked(unsigned programId);
};

#endif // SHADERPROGRAM_H

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
	void useUniformPack(std::shared_ptr<UniformPack> pack);

	// defines vertex attribute parameters for this program.
	// 		The actual vertex attrib pointers are set up using this data when [setupVAO] is called.
	// [name] is the name of the attribute as it appears in the shader;
	// [componentType] must be one of GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_HALF_FLOAT;
	// [componentCount] represents the number of [componentType] components within this attribute; must be 1,2,3,4
	// 		for example, for a vec3, [componentCount] is 3 and [componentType] is GL_FLOAT.
	// [name] specifies the name of the attribute within the shader.
	// [stride] represents the stride (distance between consecutive attribute values) within the vertex buffer object
	// [offset] represents the offset of the attribute within the vertex buffer object
	void defineVertexAttrib(std::string name, int componentType, unsigned componentCount, unsigned stride, unsigned offset);

	// loads and compiles the shaders, then links the program and fetches all uniform locations that have been mapped;
	// this should only be called one time.
	// returns true on success or false if there was an error with loading/compiling/linking the shaders.
	bool load(std::string const& vertPath, std::string const& fragPath, std::string const& geomPath="");

	// sets up the vertex attribute arrays for a specified VAO, using the previously defined vertex attributes.
	// This call is only valid after the program has been loaded.
	// this needs to be called again if the shaders are reloaded, since the attribute locations may change.
	// Thus, call this again when the [onProgramReloaded] event is triggered.
	void setupVAO(unsigned VAO);

	// sets this program up for rendering. This will also push all uniform values from all
	// assigned uniform packs into the openGL pipeline
	void begin();

	// resets the openGL state after you finished rendering with this program.
	void end();

	// checks whether this object contains a valid openGL shader program that has been successfuly loaded and linked.
	bool isValid() const { return programId_ != 0; }

	// returns the gl location of a uniform identified by name.
	// This call is only allowed after the program has been loaded.
	int getUniformLocation(const char* uName);

	// this event is triggered whith a reference to this object every time the shader program has been successfully linked.
	// This happens the first time the program is loaded and on subsequent Shader::reloadAllShaders() calls.
	Event<void(ShaderProgram const&)> onProgramReloaded;

protected:
	unsigned programId_ = 0;
	std::vector<UniformPackProxy> uniformPackProxies_;

	struct VertexAttribDescriptor;
	std::vector<VertexAttribDescriptor> vertexAttribs_;

	void onProgramLinked(unsigned programId);
};

#endif // SHADERPROGRAM_H

#ifndef UNIFORMPACK_H
#define UNIFORMPACK_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <string>

enum class UniformType {
	INT,
	FLOAT,
	VEC2,
	iVEC2,
	VEC3,
	iVEC3,
	VEC4,
	iVEC4,
	MAT3,
	MAT4
};

// provides a collection of uniforms that can be shared accross multiple programs
class UniformPack
{
public:
	UniformPack() = default;
	virtual ~UniformPack() = default;

	struct UniformDescriptor {
		// name of the uniform as it appears in the shader
		std::string name;
		// type of the uniform as it is declared in the shader
		UniformType type;
		// number of array elements if the uniform is an array, or zero otherwise.
		unsigned arrayLength = 0;
	};

	// adds a uniform description to the pack;
	// returns the index of the new uniform within this pack.
	unsigned addUniform(UniformDescriptor desc);

	// returns the number of uniforms held by this pack
	unsigned count() const { return elements_.size(); }
	// returns the ith uniform descriptor within this pack.
	UniformDescriptor const& element(unsigned i) { return elements_[i].descriptor; }

	// the following methods set values for the uniforms within this pack.
	// Note that these values are not sent to OpenGL yet, that only happens
	// when a ShaderProgram that uses this pack is being set up for rendering.

	// set a simple uniform value;
	// [indexInPack] represents the uniform index within the current UniformPack, as the value
	// 		returned by addUniform()
	template <class C>
	void setUniform(unsigned indexInPack, C value) {
		setUniformIndexed(indexInPack, 0, value);
	}

	// set an indexed uniform value (for array uniforms);
	// [indexInPack] represents the uniform index within the current UniformPack, as the value
	// 		returned by addUniform()
	// [locationIndex] represents the array index to set the value for
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, int value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, float value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec2 const& value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec2 const& value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec3 const& value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec3 const& value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec4 const& value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec4 const& value);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::mat3 const& value, bool transpose=false);
	void setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::mat4 const& value, bool transpose=false);

	// pushes a uniform value from this pack into OpenGL's pipeline at the specified location.
	// for array uniforms, all array elements are pushed at locations starting from [glLocation] incrementally.
	void pushValue(unsigned indexInPack, unsigned glLocation);

private:
	union UniformValue {
		int int_;
		float float_;
		glm::vec2 vec2_;
		glm::ivec2 ivec2_;
		glm::vec3 vec3_;
		glm::ivec3 ivec3_;
		glm::vec4 vec4_;
		glm::ivec4 ivec4_;
		glm::mat3 mat3_;
		glm::mat4 mat4_;
	};
	struct Element {
		UniformDescriptor descriptor;
		UniformValue* values;
		bool transposed = false;

		Element(UniformDescriptor desc, bool transposed)
			: descriptor(desc), transposed(transposed) {
			values = new UniformValue[desc.arrayLength];
		}

		Element(Element && el)
			: descriptor(el.descriptor), transposed(el.transposed)
			, values(el.values) {
			el.values = nullptr;
			el.descriptor.arrayLength = 0;
		}

		~Element() {
			if (values)
				delete [] values;
		}
	};
	std::vector<Element> elements_;
};

#endif // UNIFORMPACK_H

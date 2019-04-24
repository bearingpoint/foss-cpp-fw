#include <boglfw/renderOpenGL/UniformPack.h>

#include <boglfw/utils/assert.h>

#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

unsigned UniformPack::addUniform(UniformDescriptor desc) {
	elements_.push_back(
		Element {
			desc,
			UniformValue{},
			false
		});
	return elements_.size() - 1;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, int value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::INT);
	el.value.int_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, float value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::FLOAT);
	el.value.float_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec2 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::VEC2);
	el.value.vec2_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec2 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::iVEC2);
	el.value.ivec2_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec3 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::VEC3);
	el.value.vec3_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec3 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::iVEC3);
	el.value.ivec3_ = value;
}


void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec4 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::VEC4);
	el.value.vec4_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec4 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::iVEC4);
	el.value.ivec4_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::mat3 const& value, bool transpose) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::MAT3);
	el.value.mat3_ = value;
	el.transposed = transpose;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::mat4 const& value, bool transpose) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::MAT4);
	el.value.mat4_ = value;
	el.transposed = transpose;
}

// pushes a uniform value from this pack into OpenGL's pipeline at the specified location.
void UniformPack::pushValue(unsigned indexInPack, unsigned locationIndex, unsigned glLocation) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];

	switch (el.descriptor.type) {
	case UniformType::INT:
		glUniform1i(glLocation, el.value.int_);
	break;
	case UniformType::FLOAT:
		glUniform1f(glLocation, el.value.float_);
	break;
	case UniformType::VEC2:
		glUniform2fv(glLocation, 1, glm::value_ptr(el.value.vec2_));
	break;
	case UniformType::iVEC2:
		glUniform2iv(glLocation, 1, glm::value_ptr(el.value.ivec2_));
	break;
	case UniformType::VEC3:
		glUniform3fv(glLocation, 1, glm::value_ptr(el.value.vec3_));
	break;
	case UniformType::iVEC3:
		glUniform3iv(glLocation, 1, glm::value_ptr(el.value.ivec3_));
	break;
	case UniformType::VEC4:
		glUniform4fv(glLocation, 1, glm::value_ptr(el.value.vec4_));
	break;
	case UniformType::iVEC4:
		glUniform4iv(glLocation, 1, glm::value_ptr(el.value.ivec4_));
	break;
	case UniformType::MAT3:
		glUniformMatrix3fv(glLocation, 1, el.transposed, glm::value_ptr(el.value.mat3_));
	break;
	case UniformType::MAT4:
		glUniformMatrix4fv(glLocation, 1, el.transposed, glm::value_ptr(el.value.mat4_));
	break;
	}
}

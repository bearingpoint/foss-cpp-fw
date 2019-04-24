#include <boglfw/renderOpenGL/UniformPack.h>

#include <boglfw/utils/assert.h>

#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

unsigned UniformPack::addUniform(UniformDescriptor desc) {
	if (desc.arrayLength == 0)
		desc.arrayLength = 1;
	elements_.emplace_back(
		desc,
		false
	);
	return elements_.size() - 1;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, int value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::INT);
	el.values[locationIndex].int_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, float value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::FLOAT);
	el.values[locationIndex].float_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec2 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(el.descriptor.type == UniformType::VEC2);
	el.values[locationIndex].vec2_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec2 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::iVEC2);
	el.values[locationIndex].ivec2_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec3 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::VEC3);
	el.values[locationIndex].vec3_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec3 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::iVEC3);
	el.values[locationIndex].ivec3_ = value;
}


void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::vec4 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::VEC4);
	el.values[locationIndex].vec4_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::ivec4 const& value) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::iVEC4);
	el.values[locationIndex].ivec4_ = value;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::mat3 const& value, bool transpose) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::MAT3);
	el.values[locationIndex].mat3_ = value;
	el.transposed = transpose;
}

void UniformPack::setUniformIndexed(unsigned indexInPack, unsigned locationIndex, glm::mat4 const& value, bool transpose) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];
	assertDbg(locationIndex < el.descriptor.arrayLength);
	assertDbg(el.descriptor.type == UniformType::MAT4);
	el.values[locationIndex].mat4_ = value;
	el.transposed = transpose;
}

// pushes a uniform value from this pack into OpenGL's pipeline at the specified location.
void UniformPack::pushValue(unsigned indexInPack, unsigned glLocation) {
	assertDbg(indexInPack < elements_.size());
	Element& el = elements_[indexInPack];

	for (unsigned i=0; i<el.descriptor.arrayLength; i++) {
		switch (el.descriptor.type) {
		case UniformType::INT:
			glUniform1i(glLocation + i, el.values[i].int_);
		break;
		case UniformType::FLOAT:
			glUniform1f(glLocation + i, el.values[i].float_);
		break;
		case UniformType::VEC2:
			glUniform2fv(glLocation + i, 1, glm::value_ptr(el.values[i].vec2_));
		break;
		case UniformType::iVEC2:
			glUniform2iv(glLocation + i, 1, glm::value_ptr(el.values[i].ivec2_));
		break;
		case UniformType::VEC3:
			glUniform3fv(glLocation + i, 1, glm::value_ptr(el.values[i].vec3_));
		break;
		case UniformType::iVEC3:
			glUniform3iv(glLocation + i, 1, glm::value_ptr(el.values[i].ivec3_));
		break;
		case UniformType::VEC4:
			glUniform4fv(glLocation + i, 1, glm::value_ptr(el.values[i].vec4_));
		break;
		case UniformType::iVEC4:
			glUniform4iv(glLocation + i, 1, glm::value_ptr(el.values[i].ivec4_));
		break;
		case UniformType::MAT3:
			glUniformMatrix3fv(glLocation + 3*i, 1, el.transposed, glm::value_ptr(el.values[i].mat3_));
		break;
		case UniformType::MAT4:
			glUniformMatrix4fv(glLocation + 4*i, 1, el.transposed, glm::value_ptr(el.values[i].mat4_));
		break;
		}
	}
}

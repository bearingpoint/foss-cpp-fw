/*
 * TextureLoader.h
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_TEXTURELOADER_H_
#define RENDEROPENGL_TEXTURELOADER_H_

#include <string>

class TextureLoader {
public:
	// [linearizeValues] - if true, it will apply gamma correction to bring the image from sRGB space into linear space
	// [outWidth] and [outHeight] will be filled with the texture size, if provided.
	static unsigned loadFromPNG(const std::string filename, bool linearizeValues, int* outWidth=nullptr, int* outHeight=nullptr);
};

#endif /* RENDEROPENGL_TEXTURELOADER_H_ */

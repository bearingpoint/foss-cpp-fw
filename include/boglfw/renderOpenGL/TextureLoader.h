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
	static unsigned loadFromPNG(std::string const& filename, bool linearizeValues, unsigned* out_width=nullptr, unsigned* out_height=nullptr);

	// loads a cube texture from 6 files.
	// the order of the filenames must be:
	// X+, X-, Y+, Y-, Z+, Z-
	// if one or more of the cube faces are not required, specify empty strings for their respective filenames
	static unsigned loadCubeFromPNG(const std::string filenames[], bool linearizeValues);
};

#endif /* RENDEROPENGL_TEXTURELOADER_H_ */

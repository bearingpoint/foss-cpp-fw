/*
 * TextureLoader.cpp
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#include <boglfw/renderOpenGL/TextureLoader.h>
#include <boglfw/utils/log.h>

#include <GL/glew.h>
#include <png.h>
#include <memory.h>
#include <stdio.h>

#include <iostream>
#include <functional>
#include <cmath>

using namespace std;

using textureCallback = std::function<void(unsigned format, unsigned width, unsigned height,
											unsigned dataType, void* dataPtr)>;

static void internalLoadPNG(string const& filename, bool linearizeValues, textureCallback cb) {
	// This function was originally written by David Grayson for
    // https://github.com/DavidEGrayson/ahrs-visualizer

	LOGPREFIX("TextureLoader");
	LOGLN("Loading texture from " << filename << " ...");

    png_byte header[8];

    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp == 0)
    {
		ERROR("Failed to open file.");
        perror(filename.c_str());
        return;
    }

    // read the header
    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
    {
        ERROR(filename << " is not a PNG.");
        fclose(fp);
        return;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        ERROR("png_create_read_struct returned 0.");
        fclose(fp);
        return;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
    	ERROR("png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        ERROR("png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        ERROR("png_jmpbuf from libpng");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

    if (bit_depth != 8)
    {
        ERROR(filename << ": Unsupported bit depth " << bit_depth << ".  Must be 8.");
        return;
    }

    GLint format;
    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        format = GL_RGB;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = GL_RGBA;
        break;
    default:
        ERROR(filename << ": Unknown libpng color type " << color_type << ".");
        return;
    }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL)
    {
        ERROR("Failed to allocate memory for PNG image data");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
    if (row_pointers == NULL)
    {
        ERROR("Failed to allocate memory for PNG row pointers");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

	if (linearizeValues) {
		unsigned bpp = format == GL_RGBA ? 4 : 3; // bytes per pixel
		float gamma = 2.2f;
		// apply gamma correction
		for (unsigned i=0; i<temp_height; i++)
			for (unsigned j=0; j<temp_width; j++) {
				row_pointers[i][j*bpp+0] = (png_byte)(255 * pow(row_pointers[i][j*bpp+0] / 255.f, gamma));
				row_pointers[i][j*bpp+1] = (png_byte)(255 * pow(row_pointers[i][j*bpp+1] / 255.f, gamma));
				row_pointers[i][j*bpp+2] = (png_byte)(255 * pow(row_pointers[i][j*bpp+2] / 255.f, gamma));
				if (bpp == 4)
					row_pointers[i][j*bpp+3] = (png_byte)(255 * pow(row_pointers[i][j*bpp+3] / 255.f, gamma));
			}
	}

	// call user callback with loaded image details:
    cb(format, temp_width, temp_height, GL_UNSIGNED_BYTE, image_data);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
	LOGNP(" OK.\n");
}

unsigned TextureLoader::loadFromPNG(string const& filename, bool linearizeValues, unsigned* out_width, unsigned* out_height) {
	unsigned texture = 0;
	auto callback = [out_width, out_height, &texture]
		(unsigned format, unsigned width, unsigned height, unsigned dataType, void* dataPtr)
	{
		if (out_width)
			*out_width = width;
		if (out_height)
			*out_height = height;

		// Generate the OpenGL texture object
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, dataType, dataPtr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	};

	internalLoadPNG(filename, linearizeValues, callback);

    return texture;
}

unsigned TextureLoader::loadCubeFromPNG(const std::string filenames[], bool linearizeValues) {
	unsigned texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	auto callback = [] (unsigned faceId, unsigned format, unsigned width, unsigned height, unsigned dataType, void* dataPtr) {
		glTexImage2D(faceId, 0, format, width, height, 0, format, dataType, dataPtr);
	};

	unsigned faceIds[] {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

	for (int i=0; i<6; i++) {
		internalLoadPNG(filenames[i], linearizeValues, [i, &faceIds, &callback]
			(unsigned format, unsigned width, unsigned height, unsigned dataType, void* dataPtr)
		{
			callback(faceIds[i], format, width, height, dataType, dataPtr);
		});
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return texture;
}

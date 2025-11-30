#include "texture.h"
#include <stb_image.h>
#include <assert.h>

bool texture_new_from_encoded(Texture* this, const unsigned char* png_data, size_t png_size) {
	assert(png_size <= INT_MAX);
	unsigned char* pixels = stbi_load_from_memory(
		png_data,
		(int)png_size,
		&this->width,
		&this->height,
		NULL,
		4
	);
	if (!pixels) return false;

	glGenTextures(1, &this->id);
	glBindTexture(GL_TEXTURE_2D, this->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	bool error = false;
	while (glGetError() != GL_NO_ERROR) error = true;

	if (error) {
		glDeleteTextures(1, &this->id);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(pixels);
		return false;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pixels);

	return true;
}

void texture_delete(Texture* this) {
	glDeleteTextures(1, &this->id);
}

ImTextureRef texture_to_imtextureref(const Texture* texture) {
	return (ImTextureRef) {
		._TexData = NULL,
			._TexID = texture->id
	};
}

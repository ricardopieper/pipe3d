#include "stb_image.h"
#include <glad/glad.h>
#include "Image.h"
#include <fstream>
#include <vector>
#include <iterator>


const bool CACHE_ENABLED = true;

Image Image::Load(std::string& path, bool loadInverted) {
	//try load from cache
	Image img;
	std::string cachedPath = path + ".cached_img";
	std::ifstream cachedFile(cachedPath, std::ios::in | std::ios::binary);
	//is >> std::noskipws;
	if (cachedFile.good() && CACHE_ENABLED) {
		img.Path = cachedPath;
		// get size of file
		cachedFile.seekg(0, std::ifstream::end);
		int fileSize = cachedFile.tellg();
		cachedFile.seekg(0);

		
		cachedFile.read((char*)&img.Width, sizeof(int));
		cachedFile.read((char*)&img.Height, sizeof(int));
		cachedFile.read((char*)&img.Channels, sizeof(int));
		img.Data = new unsigned char[fileSize - (sizeof(int) * 3)];

		cachedFile.read((char*)img.Data, fileSize - (sizeof(int) * 3));

		img.IsFromCache = true;
		img.Valid = true;
		cachedFile.close();
	}
	else {
		img.IsFromCache = false;
		img.Path = path;
		if (loadInverted) {
			stbi_set_flip_vertically_on_load(1);
		}
		else {
			stbi_set_flip_vertically_on_load(0);
		}
		unsigned char* localBuffer = stbi_load(path.c_str(), &img.Width, &img.Height, &img.Channels, 0);
		if (img.Width == 0 && img.Height == 0) {
			if (localBuffer) {
				stbi_image_free(localBuffer);
			}
			img.Valid = false;
		}
		else {
			img.Data = localBuffer;
			img.Valid = true;
		}
	}

	return img;
}

void Image::Dispose() {
	if (IsFromCache) {
		if (Data != nullptr) {
			delete[] Data;
		}
	}
	else {
		stbi_image_free(Data);
	}
}

int Image::GetOpenglFormat(bool srgb) {
	if (srgb) {
		if (Channels == 1) {
			return GL_RED;
		}
		else if (Channels == 2) {
			return GL_RG;
		}
		else if (Channels == 3) {
			return GL_SRGB;
		}
		else if (Channels == 4) {
			return GL_SRGB_ALPHA;
		}
	}
	else {
		if (Channels == 1) {
			return GL_RED;
		}
		else if (Channels == 2) {
			return GL_RG;
		}
		else if (Channels == 3) {
			return GL_RGB;
		}
		else if (Channels == 4) {
			return GL_RGBA;
		}
	}
}

void Image::SaveCache() {
	if (IsFromCache) return;
	std::string cachedPath = Path + ".cached_img";
	std::ofstream cachedFile(cachedPath, std::ios::binary);
	cachedFile.write((char*)&Width, sizeof(int));
	cachedFile.write((char*)&Height, sizeof(int));
	cachedFile.write((char*)&Channels, sizeof(int));
	cachedFile.write((char*)Data, Width * Height * Channels);
	cachedFile.close();
}

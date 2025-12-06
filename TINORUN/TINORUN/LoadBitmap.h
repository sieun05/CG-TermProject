#pragma once
#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

extern "C" {
	unsigned char* stbi_load(char const* filename, int* x, int* y, int* channels_in_file, int desired_channels);
	void stbi_image_free(void* retval_from_stbi_load);
	void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);
}


inline GLubyte* LoadDIBitmap(const char* filename, BITMAPINFO** info)
{
    FILE* fp;
    GLubyte* bits;
    int bitsize, infosize;
    BITMAPFILEHEADER header;

    if ((fp = fopen(filename, "rb")) == NULL)
        return NULL;

    if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1)
    {
        fclose(fp);
        return NULL;
    }
    if (header.bfType != 'MB') {
        fclose(fp);
        return NULL;
    }
    infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);

    if ((*info = (BITMAPINFO*)malloc(infosize)) == NULL) {
        fclose(fp);
        return NULL;
    }
    if (fread(*info, 1, infosize, fp) < (size_t)infosize) {
        free(*info);
        fclose(fp);
        return NULL;
    }
    if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
        bitsize = (*info)->bmiHeader.biWidth * abs((*info)->bmiHeader.biHeight) *
        (*info)->bmiHeader.biBitCount / 8;
    if ((bits = (GLubyte*)malloc(bitsize)) == NULL) {
        free(info);
        fclose(fp);
        return NULL;
    }
    if (fread(bits, 1, bitsize, fp) < (size_t)bitsize) {
        free(info);
        free(bits);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    return bits;
}

inline GLubyte* LoadPNGTexture(const char* file, int* width, int* height, int* chaanels) {
    unsigned char* data = stbi_load(file, width, height, chaanels, 0);
    if (data == NULL) {
        printf("Failed to load PNG file: %s\n", file);
        return NULL;
    }
    return data;
}

inline void FreePNGTexture(GLubyte* data) {
    if(data)
	    stbi_image_free(data);
}
#ifdef TEXTURE_H
#define TEXTURE_H

#include <stdbool.h>

typedef struct {
    int width;
    int height;
    const char* channels;
} Texture;

Texture* texture_load(const char* path);
void texture_bind(Texture texture);
void texture_unbind();

typedef struct {
    unsigned int id;
    int width;
    int height;
    int tile_size;
    int tile_count;
} TextureArray;

TextureArray texture_array_create(const char* path, int tile_size);
void texture_array_bind(TextureArray array, unsigned int unit);
int texture_array_add(TextureArray* array, const char* path);


#endif
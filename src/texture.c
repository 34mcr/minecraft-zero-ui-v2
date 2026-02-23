#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glew.h>

Texture texture_load(const char* path, bool flip) {
    Texture texture = {0};
    
    stbi_set_flip_vertically_on_load(flip);
    
    unsigned char* data = stbi_load(path, &texture.width, &texture.height, 
                                    &texture.channels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture: %s\\n", path);
        return texture;
    }
    
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // 根据通道数选择格式
    GLenum format = GL_RGB;
    if (texture.channels == 4) {
        format = GL_RGBA;
    } else if (texture.channels == 1) {
        format = GL_RED;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 
                 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(data);
    
    printf("Texture loaded: %s (%dx%d, %d channels)\\n", 
           path, texture.width, texture.height, texture.channels);
    
    return texture;
}

void texture_bind(Texture texture, unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void texture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

// 纹理数组实现
TextureArray texture_array_create(const char* path, int tile_size, int tile_count) {
    TextureArray array = {0};
    array.tile_size = tile_size;
    
    // 加载完整纹理
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture array: %s\\n", path);
        return array;
    }
    
    // 计算行列数
    int cols = width / tile_size;
    int rows = height / tile_size;
    array.tile_count = cols * rows;
    
    if (array.tile_count < tile_count) {
        fprintf(stderr, "Texture array too small: %d < %d\\n", 
                array.tile_count, tile_count);
        stbi_image_free(data);
        return array;
    }
    
    // 创建纹理数组
    glGenTextures(1, &array.id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, array.id);
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // 分配存储
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 
                 tile_size, tile_size, array.tile_count,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    // 提取每个瓦片
    for (int i = 0; i < array.tile_count; i++) {
        int col = i % cols;
        int row = i / cols;
        
        // 提取瓦片数据
        unsigned char* tile_data = malloc(tile_size * tile_size * 4);
        for (int y = 0; y < tile_size; y++) {
            for (int x = 0; x < tile_size; x++) {
                int src_x = col * tile_size + x;
                int src_y = row * tile_size + y;
                int src_idx = (src_y * width + src_x) * channels;
                int dst_idx = (y * tile_size + x) * 4;
                
                if (channels == 4) {
                    tile_data[dst_idx] = data[src_idx];
                    tile_data[dst_idx+1] = data[src_idx+1];
                    tile_data[dst_idx+2] = data[src_idx+2];
                    tile_data[dst_idx+3] = data[src_idx+3];
                } else if (channels == 3) {
                    tile_data[dst_idx] = data[src_idx];
                    tile_data[dst_idx+1] = data[src_idx+1];
                    tile_data[dst_idx+2] = data[src_idx+2];
                    tile_data[dst_idx+3] = 255;
                } else if (channels == 1) {
                    tile_data[dst_idx] = data[src_idx];
                    tile_data[dst_idx+1] = data[src_idx];
                    tile_data[dst_idx+2] = data[src_idx];
                    tile_data[dst_idx+3] = 255;
                }
            }
        }
        
        // 上传瓦片
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                        0, 0, i,  // x, y, z offset
                        tile_size, tile_size, 1,
                        GL_RGBA, GL_UNSIGNED_BYTE, tile_data);
        
        free(tile_data);
    }
    
    stbi_image_free(data);
    printf("Texture array created: %s (%dx%d tiles)\\n", 
           path, cols, rows);
    
    return array;
}

void texture_array_bind(TextureArray array, unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, array.id);
}

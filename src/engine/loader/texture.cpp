#define STB_IMAGE_IMPLEMENTATION

#include "texture.h"
#include <engine/logging.h>
#include <stb_image.h>
#include <unordered_map>
#include <gl/glew.h>
#ifndef NDEBUG
#include <chrono>
#endif

namespace Engine::Loader {
    struct ImageData {
        int width, height, channelCount;
        stbi_uc *imgData;
    };
    /*!
     * Internal helper function to load an image from a file.
     * @attention You are responsible for freeing the memory allocated suing `stbi_image_free`.
     */
    std::expected<ImageData, std::string> loadImage(const char *filePath) {
#ifndef NDEBUG
        const auto startTimer = std::chrono::high_resolution_clock::now();
#endif
        int width, height, channelCount;
        stbi_uc *imgData = stbi_load(filePath, &width, &height, &channelCount, 0);
        if (!imgData) {
            stbi_image_free(imgData);
            logError("Failed to load texture \"%s\": %s", filePath, stbi_failure_reason());
            return std::unexpected(FILE_REF + std::string("Failed to load texture: \"") + filePath + "\": " + stbi_failure_reason());
        }
#ifndef NDEBUG
        logDebug("Loaded texture \"%s\" with dimensions %dx%d and %d channels in %dms",
            filePath, width, height, channelCount,
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTimer).count());
#endif

        return ImageData{width, height, channelCount, imgData};
    }

    GLint getChannelCount(const int channelCount) {
        switch (channelCount) {
            case 1: return GL_RED;
            case 3: return GL_RGB;
            case 4: return GL_RGBA;
            default: {
                logWarn("Unknown channel count %d, defaulting to RGB", channelCount);
                return GL_RGB;
            }
        }
    }

    std::expected<unsigned int, std::string> loadTexture(const char *filePath) {
        std::expected<ImageData, std::string> imgData = loadImage(filePath);
        if (!imgData)
            return std::unexpected(FW_UNEXP(imgData, "Failed to load texture"));

        const GLint format = getChannelCount(imgData->channelCount);

        unsigned int textureID;
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, imgData->width, imgData->height, 0, format, GL_UNSIGNED_BYTE, imgData->imgData);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(imgData->imgData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return textureID;
    }

    std::expected<unsigned int, std::string> loadCubeMap(const std::string &filePath) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        const std::pmr::unordered_map<GLint, std::string> dirMap = {
            {GL_TEXTURE_CUBE_MAP_POSITIVE_X, "right"},
            {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "left"},
            {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "top"},
            {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "bottom"},
            {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "front"},
            {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "back"}
        };

        for (int i = 0; i < 6; i++) {
            const GLint faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

            const auto extension_index = filePath.find_last_of('.');
            const auto path = filePath.substr(0, extension_index) + "_" + dirMap.at(faceDir) + filePath.substr(extension_index);

            std::expected<ImageData, std::string> imgData = loadImage(path.c_str());
            if (!imgData) {
                glDeleteTextures(1, &textureID);
                return std::unexpected(FW_UNEXP(imgData, "Failed to load cubemap texture"));
            }

            const GLint format = getChannelCount(imgData->channelCount);
            glTexImage2D(faceDir,
                0, format, imgData->width, imgData->height, 0, format, GL_UNSIGNED_BYTE, imgData->imgData);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }
}
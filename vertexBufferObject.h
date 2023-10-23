#pragma once
#include <vector>
#include <GL/glew.h>        
#include <GLFW/glfw3.h>     

class VertexBufferObject
{
public:
    // Create new VBO
    void createVBO(size_t reserveSizeBytes = 0);

    // Bind VBO
    void bindVBO(GLenum bufferType = GL_ARRAY_BUFFER);

    // Bind data from static mesh
    void addRawData(const void* ptrData, size_t dataSizeBytes, int repeat = 1);
    template<typename T>
    void addData(const T& ptrObj, int repeat = 1)
    {
        addRawData(&ptrObj, sizeof(T), repeat);
    }
    // Get raw data from static mesh pointer
    void* getRawDataPointer();
    void uploadDataToGPU(GLenum usageHint);
    void* mapBufferToMemory(GLenum usageHint) const;
    void* mapSubBufferToMemory(GLenum usageHint, size_t offset, size_t length) const;

    // Unbind VBO buffer
    void unmapBuffer() const;

    GLuint getBufferID() const;
    size_t getBufferSize();
    // Destroy VBO
    void deleteVBO();

private:
    GLuint _bufferID = 0;
    int _bufferType;

    std::vector<unsigned char> _rawData;
    size_t _bytesAdded = 0;
    size_t _uploadedDataSize;

    bool _isBufferCreated = false; 
    bool _isDataUploaded = false; 
};

#include <iostream>
#include <cstring>

#include "vertexBufferObject.h"

// Create VBO for mesh generation
void VertexBufferObject::createVBO(size_t reserveSizeBytes)
{
    if (_isBufferCreated)
    {
        return;
    }

    glGenBuffers(1, &_bufferID);
    _rawData.reserve(reserveSizeBytes > 0 ? reserveSizeBytes : 1024);
    _isBufferCreated = true;
}

// Bind VBO function
void VertexBufferObject::bindVBO(GLenum bufferType)
{
    if (!_isBufferCreated)
    {
        return;
    }

    _bufferType = bufferType;
    glBindBuffer(_bufferType, _bufferID);
}

// Receive data from static mesh function call
void VertexBufferObject::addRawData(const void* ptrData, size_t dataSize, int repeat)
{
    const auto bytesToAdd = dataSize * repeat;
    const auto requiredCapacity = _bytesAdded + bytesToAdd;
    if (requiredCapacity > _rawData.capacity())
    {
        auto newCapacity = _rawData.capacity() * 2;
        while (newCapacity < requiredCapacity) {
            newCapacity *= 2;
        }

        std::vector<unsigned char> newRawData;
        newRawData.reserve(newCapacity);
        memcpy(newRawData.data(), _rawData.data(), _bytesAdded);
        _rawData = std::move(newRawData);
    }

    for (int i = 0; i < repeat; i++)
    {
        memcpy(_rawData.data() + _bytesAdded, ptrData, dataSize);
        _bytesAdded += dataSize;
    }
}

// Gather pointer data from the static mesh call
void* VertexBufferObject::getRawDataPointer()
{
    return _rawData.data();
}

// Bind and send to GPU 
void VertexBufferObject::uploadDataToGPU(GLenum usageHint)
{
    if (!_isBufferCreated)
    {
        return;
    }

    glBufferData(_bufferType, _bytesAdded, _rawData.data(), usageHint);
    _isDataUploaded = true;
    _uploadedDataSize = _bytesAdded;
    _bytesAdded = 0;
}

void* VertexBufferObject::mapBufferToMemory(GLenum usageHint) const
{
    if (!_isDataUploaded) {
        return nullptr;
    }

    return glMapBuffer(_bufferType, usageHint);
}

void* VertexBufferObject::mapSubBufferToMemory(GLenum usageHint, size_t offset, size_t length) const
{
    if (!_isDataUploaded) {
        return nullptr;
    }

    return glMapBufferRange(_bufferType, offset, length, usageHint);
}

void VertexBufferObject::unmapBuffer() const
{
    glUnmapBuffer(_bufferType);
}
// ID from buffer assigned to memory
GLuint VertexBufferObject::getBufferID() const
{
    return _bufferID;
}
// Gather buffer size data
size_t VertexBufferObject::getBufferSize()
{
    return _isDataUploaded ? _uploadedDataSize : _bytesAdded;
}

// Destroy Vertex buffer object
void VertexBufferObject::deleteVBO()
{
    if (!_isBufferCreated) {
        return;
    }
    glDeleteBuffers(1, &_bufferID);
    _isDataUploaded = false;
    _isBufferCreated = false;
}

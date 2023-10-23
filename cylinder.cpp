
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "cylinder.h"

namespace static_meshes_3D {

    Cylinder::Cylinder(float radius, int numSlices, float height, bool withPositions, bool withTextureCoordinates, bool withNormals)
        : StaticMesh3D(withPositions, withTextureCoordinates, withNormals)
        , _radius(radius)
        , _numSlices(numSlices)
        , _height(height)
    {
        initializeData();
    }

    float Cylinder::getRadius() const
    {
        return _radius;
    }

    int Cylinder::getSlices() const
    {
        return _numSlices;
    }

    float Cylinder::getHeight() const
    {
        return _height;
    }

    void Cylinder::initializeData()
    {
        if (_isInitialized) {
            return;
        }


        _numVerticesSide = (_numSlices + 1) * 2;
        _numVerticesTopBottom = _numSlices + 2;
        _numVerticesTotal = _numVerticesSide + _numVerticesTopBottom * 2;

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        _vbo.createVBO(getVertexByteSize() * _numVerticesTotal);


        const auto sliceAngleStep = 2.0f * glm::pi<float>() / static_cast<float>(_numSlices);
        auto currentSliceAngle = 0.0f;
        std::vector<float> sines, cosines;
        for (auto i = 0; i <= _numSlices; i++)
        {
            sines.push_back(sin(currentSliceAngle));
            cosines.push_back(cos(currentSliceAngle));

            currentSliceAngle += sliceAngleStep;
        }

        if (hasPositions())
        {

            std::vector<float> x;
            std::vector<float> z;
            for (auto i = 0; i <= _numSlices; i++)
            {
                x.push_back(cosines[i] * _radius);
                z.push_back(sines[i] * _radius);
            }


            for (auto i = 0; i <= _numSlices; i++)
            {
                const auto topPosition = glm::vec3(x[i], _height / 2.0f, z[i]);
                const auto bottomPosition = glm::vec3(x[i], -_height / 2.0f, z[i]);
                _vbo.addRawData(&topPosition, sizeof(glm::vec3));
                _vbo.addRawData(&bottomPosition, sizeof(glm::vec3));
            }

            glm::vec3 topCenterPosition(0.0f, _height / 2.0f, 0.0f);
            _vbo.addRawData(&topCenterPosition, sizeof(glm::vec3));
            for (auto i = 0; i <= _numSlices; i++)
            {
                const auto topPosition = glm::vec3(x[i], _height / 2.0f, z[i]);
                _vbo.addRawData(&topPosition, sizeof(glm::vec3));
            }


            glm::vec3 bottomCenterPosition(0.0f, -_height / 2.0f, 0.0f);
            _vbo.addRawData(&bottomCenterPosition, sizeof(glm::vec3));
            for (auto i = 0; i <= _numSlices; i++)
            {
                const auto bottomPosition = glm::vec3(x[i], -_height / 2.0f, -z[i]);
                _vbo.addRawData(&bottomPosition, sizeof(glm::vec3));
            }
        }

        if (hasTextureCoordinates())
        {

            const auto sliceTextureStepU = 2.0f / static_cast<float>(_numSlices);

            auto currentSliceTexCoordU = 0.0f;
            for (auto i = 0; i <= _numSlices; i++)
            {
                _vbo.addData(glm::vec2(currentSliceTexCoordU, 1.0f));
                _vbo.addData(glm::vec2(currentSliceTexCoordU, 0.0f));
 
                currentSliceTexCoordU += sliceTextureStepU;
            }

            glm::vec2 topBottomCenterTexCoord(0.5f, 0.5f);
            _vbo.addData(topBottomCenterTexCoord);
            for (auto i = 0; i <= _numSlices; i++) {
                _vbo.addData(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y + cosines[i] * 0.5f));
            }

            _vbo.addData(topBottomCenterTexCoord);
            for (auto i = 0; i <= _numSlices; i++) {
                _vbo.addData(glm::vec2(topBottomCenterTexCoord.x + sines[i] * 0.5f, topBottomCenterTexCoord.y - cosines[i] * 0.5f));
            }
        }

        if (hasNormals())
        {
            for (auto i = 0; i <= _numSlices; i++) {
                _vbo.addData(glm::vec3(cosines[i], 0.0f, sines[i]), 2);
            }


            _vbo.addData(glm::vec3(0.0f, 1.0f, 0.0f), _numVerticesTopBottom);


            _vbo.addData(glm::vec3(0.0f, -1.0f, 0.0f), _numVerticesTopBottom);
        }

        _vbo.bindVBO();
        _vbo.uploadDataToGPU(GL_STATIC_DRAW);
        setVertexAttributesPointers(_numVerticesTotal);

        _isInitialized = true;
    }

    void Cylinder::render() const
    {
        if (!_isInitialized) {
            return;
        }

        glBindVertexArray(_vao);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, _numVerticesSide);


        glDrawArrays(GL_TRIANGLE_FAN, _numVerticesSide, _numVerticesTopBottom);

        glDrawArrays(GL_TRIANGLE_FAN, _numVerticesSide + _numVerticesTopBottom, _numVerticesTopBottom);
    }

    void Cylinder::renderPoints() const
    {
        if (!_isInitialized) {
            return;
        }


        glBindVertexArray(_vao);
        glDrawArrays(GL_POINTS, 0, _numVerticesTotal);
    }

}

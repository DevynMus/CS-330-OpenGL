#pragma once
#include "staticMesh3D.h"

namespace static_meshes_3D {

    class Cylinder : public StaticMesh3D
    {
    public:
        // Cylinder generation with radius, number of slices, and height
        Cylinder(float radius, int numSlices, float height,
            bool withPositions = true, bool withTextureCoordinates = true, bool withNormals = true);

        void render() const override;
        void renderPoints() const override;

        float getRadius() const;

        int getSlices() const;

        float getHeight() const;

    private:
        // Private cylinder variables
        float _radius; 
        int _numSlices;
        float _height; 

        int _numVerticesSide; 
        int _numVerticesTopBottom;
        int _numVerticesTotal;

        void initializeData() override;
    };

}

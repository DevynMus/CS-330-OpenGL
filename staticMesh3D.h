#pragma once
#include "vertexBufferObject.h"

namespace static_meshes_3D {

	class StaticMesh3D
	{
	public:
		// Init variables
		static const int POSITION_ATTRIBUTE_INDEX; 
		static const int TEXTURE_COORDINATE_ATTRIBUTE_INDEX;
		static const int NORMAL_ATTRIBUTE_INDEX; 

		StaticMesh3D(bool withPositions, bool withTextureCoordinates, bool withNormals);
		virtual ~StaticMesh3D();

		// Render static mesh data
		virtual void render() const = 0;
		virtual void renderPoints() const {}

		// Destroy Mesh and its' data
		virtual void deleteMesh();

		// Checks for vertex, texture, and normal coordinates
		bool hasPositions() const;
		bool hasTextureCoordinates() const;
		bool hasNormals() const;
		int getVertexByteSize() const;

	protected:
		bool _hasPositions = false;
		bool _hasTextureCoordinates = false;
		bool _hasNormals = false;

		bool _isInitialized = false;
		GLuint _vao = 0;
		VertexBufferObject _vbo;

		// Set data for static mesh generation
		virtual void initializeData() {}
		void setVertexAttributesPointers(int numVertices);
	};

};

///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  ORIGINAL AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//
//  CS-499 Capstone - Enhancement Two (Algorithms & Data Structure)
//  Modified by Gary Travis
//  Reworked the scene from a list of hard-coded draw calls into a small
//  data-driven scene graph. Objects are now described by data (a SceneObject
//  struct) and stored in a vector, meshes are looked up from a map, and the
//  world transforms are built by walking the parent chain.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor
	SceneManager(ShaderManager *pShaderManager);
	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		float ambientStrength;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

	// ---- Enhancement Two: scene-graph data types ----

	// Using an enum class for the mesh type instead of passing strings around
	// at draw time. This way the compiler catches a bad mesh name and there is
	// no chance of a typo picking the wrong shape.
	enum class MeshType
	{
		Plane,
		Box,
		Cylinder,
		TaperedCylinder,
		Torus,
		Cone,
		Prism,
		Sphere
	};

	// One record per object in the scene. Keeping every value an object needs
	// in a single struct instead of spreading it across the render code. Each
	// object can point at a parent (by index) so I can build a simple
	// hierarchy, like the grill being attached to the heater body.
	struct SceneObject
	{
		std::string name = "object";        // id, also handy for debugging
		MeshType    mesh = MeshType::Box;    // which primitive to draw

		// transform is stored relative to the parent (if it has one)
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 rotation = glm::vec3(0.0f); // degrees, X/Y/Z
		glm::vec3 scale    = glm::vec3(1.0f);

		// look of the object
		glm::vec4   color        = glm::vec4(1.0f); // used when there is no texture
		std::string textureTag   = "";              // empty means use the color
		std::string materialTag  = "";              // empty means no lighting material
		glm::vec2   uvScale      = glm::vec2(1.0f);
		bool        useLighting  = false;

		// index of the parent object in the vector, or -1 if this is a root
		int parentIndex = -1;
	};

public:

	// The following methods are for the students to
	// customize for their own 3D scene
	void PrepareScene();
	void RenderScene();
	void SetWarmSunLight();

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// ---- Enhancement Two: scene-graph storage ----

	// every object that gets drawn lives in here
	std::vector<SceneObject> m_sceneObjects;

	// maps a mesh type to the little function that draws it. I fill this in
	// once in PrepareScene so the render loop can just look the mesh up
	// instead of running through a big if/else every frame.
	std::unordered_map<MeshType, std::function<void()>> m_meshRegistry;

	// helpers that build the scene data
	int  AddObject(const SceneObject& object);   // returns the new object's index
	void BuildSceneGraph();                       // describe the whole scene as data
	void RegisterMeshes();                        // fill in the mesh lookup map

	// scene-graph math/drawing
	// walks up the parent chain to build the final world matrix for an object
	glm::mat4 ComputeWorldMatrix(int objectIndex) const;
	// draws one object using its stored data and the mesh map
	void RenderObject(const SceneObject& object, const glm::mat4& worldMatrix);

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	// set the transformation values
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ);

	// push an already-built world matrix straight to the shader (scene graph uses this)
	void SetModelMatrix(const glm::mat4& modelMatrix);

	// set the color values into the shader
	void SetShaderColor(
		float redColorValue,
		float greenColorValue,
		float blueColorValue,
		float alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);
};

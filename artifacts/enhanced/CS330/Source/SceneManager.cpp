///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>   // needed for std::sort when depth sorting the objects
#include <vector>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
	// always initialize counters in the constructor - leaving this uninitialized
	// was letting the first texture land in a garbage slot of the array
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetModelMatrix()
 *
 *  Scene-graph version of SetTransformations(). The world
 *  matrix is already built by walking the parent chain, so
 *  here I just hand it straight to the shader.
 ***********************************************************/
void SceneManager::SetModelMatrix(const glm::mat4& modelMatrix)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelMatrix);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
 *  SetWarmSunLight()
 *
 *  Sets up a warm sun light coming from the front-left.
 ***********************************************************/
void SceneManager::SetWarmSunLight()
{
	if (m_pShaderManager == NULL)
		return;

	// Warm sun coming from front-left, above
	glm::vec3 lightPosDir = glm::vec3(-15.0f, 10.0f, 5.0f);

	glm::vec3 ambientColor = glm::vec3(0.08f, 0.06f, 0.04f);
	glm::vec3 diffuseColor = glm::vec3(1.0f, 0.9f, 0.7f);
	glm::vec3 specularColor = glm::vec3(1.0f, 0.9f, 0.8f);

	float focalStrength = 32.0f;
	float specularIntensity = 0.6f;

	m_pShaderManager->setVec3Value("lightSources[0].position", lightPosDir);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", ambientColor);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", diffuseColor);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", specularColor);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", focalStrength);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", specularIntensity);

	for (int i = 1; i < 4; ++i)
	{
		m_pShaderManager->setVec3Value(
			"lightSources[" + std::to_string(i) + "].ambientColor",
			glm::vec3(0.0f));
		m_pShaderManager->setVec3Value(
			"lightSources[" + std::to_string(i) + "].diffuseColor",
			glm::vec3(0.0f));
		m_pShaderManager->setVec3Value(
			"lightSources[" + std::to_string(i) + "].specularColor",
			glm::vec3(0.0f));
		m_pShaderManager->setFloatValue(
			"lightSources[" + std::to_string(i) + "].specularIntensity",
			0.0f);
	}

	m_pShaderManager->setVec3Value("viewPosition", glm::vec3(0.0f, 2.0f, 5.0f));
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/



/***********************************************************
 *  RegisterMeshes()
 *
 *  Fills in the mesh lookup map. Each mesh type is paired with
 *  a small lambda that knows how to draw it. Doing this once up
 *  front means the render loop can grab the right draw call from
 *  the map (about O(1)) instead of running a long if/else every
 *  single frame.
 ***********************************************************/
void SceneManager::RegisterMeshes()
{
	m_meshRegistry[MeshType::Plane]           = [this]() { m_basicMeshes->DrawPlaneMesh(); };
	m_meshRegistry[MeshType::Box]             = [this]() { m_basicMeshes->DrawBoxMesh(); };
	m_meshRegistry[MeshType::Cylinder]        = [this]() { m_basicMeshes->DrawCylinderMesh(); };
	m_meshRegistry[MeshType::TaperedCylinder] = [this]() { m_basicMeshes->DrawTaperedCylinderMesh(); };
	m_meshRegistry[MeshType::Torus]           = [this]() { m_basicMeshes->DrawTorusMesh(); };
	m_meshRegistry[MeshType::Cone]            = [this]() { m_basicMeshes->DrawConeMesh(); };
	m_meshRegistry[MeshType::Prism]           = [this]() { m_basicMeshes->DrawPrismMesh(); };
	m_meshRegistry[MeshType::Sphere]          = [this]() { m_basicMeshes->DrawSphereMesh(); };
}

/***********************************************************
 *  AddObject()
 *
 *  Adds one object to the scene graph and hands back its
 *  index. I return the index so a child can point at this
 *  object as its parent when I build the next one.
 ***********************************************************/
int SceneManager::AddObject(const SceneObject& object)
{
	m_sceneObjects.push_back(object);
	return static_cast<int>(m_sceneObjects.size()) - 1;
}

/***********************************************************
 *  BuildSceneGraph()
 *
 *  Describes the whole scene as DATA instead of as draw calls.
 *  This is the part that replaces all of the old hard-coded
 *  DrawHeater / DrawBasket / etc. calls. Each object is just a
 *  filled-in struct that gets pushed into the vector. Children
 *  (like the heater grill) reference their parent by index so
 *  their transform is built on top of the parent's.
 ***********************************************************/
void SceneManager::BuildSceneGraph()
{
	// start clean in case this ever gets called twice
	m_sceneObjects.clear();

	const float heaterScale = 2.5f;

	// ---- floor ----
	SceneObject floor;
	floor.name        = "floor";
	floor.mesh        = MeshType::Plane;
	floor.scale       = glm::vec3(20.0f, 1.0f, 10.0f);
	floor.position    = glm::vec3(0.0f, 0.0f, 0.0f);
	floor.textureTag  = "floor_carpet";
	floor.materialTag = "carpet";
	floor.uvScale     = glm::vec2(2.0f, 1.0f);
	floor.useLighting = true;
	AddObject(floor);

	// ---- heater body (parent of the grill + trim) ----
	SceneObject heaterBody;
	heaterBody.name       = "heater_body";
	heaterBody.mesh       = MeshType::Box;
	heaterBody.scale      = glm::vec3(2.0f * heaterScale, 1.6f * heaterScale, 0.5f * heaterScale);
	heaterBody.position   = glm::vec3(0.0f, 0.8f * heaterScale, -2.5f);
	heaterBody.textureTag = "heater_walnut";
	heaterBody.uvScale    = glm::vec2(1.5f, 1.0f);
	int heaterIndex = AddObject(heaterBody);

	// ---- front grill (child of the body) ----
	// positions here are RELATIVE to the body, so if the heater moves the
	// grill follows it automatically - that is the whole point of parenting
	SceneObject grill;
	grill.name        = "heater_grill";
	grill.mesh        = MeshType::Box;
	grill.scale       = glm::vec3(0.85f, 0.70f, 0.08f);
	grill.position    = glm::vec3(0.0f, -0.12f * heaterScale, 0.26f * heaterScale);
	grill.textureTag  = "heater_fire";
	grill.parentIndex = heaterIndex;
	AddObject(grill);

	// ---- top trim strip (also a child of the body) ----
	SceneObject trim;
	trim.name        = "heater_trim";
	trim.mesh        = MeshType::Box;
	trim.scale       = glm::vec3(0.8f, 0.09f, 0.10f);
	trim.position    = glm::vec3(0.0f, 0.65f * heaterScale, 0.28f * heaterScale);
	trim.color       = glm::vec4(0.55f, 0.38f, 0.20f, 1.0f);
	trim.parentIndex = heaterIndex;
	AddObject(trim);

	// ---- basket sitting on top of the heater ----
	SceneObject basket;
	basket.name       = "basket";
	basket.mesh       = MeshType::TaperedCylinder;
	basket.scale      = glm::vec3(1.5f, 1.2f, 1.5f);
	basket.rotation   = glm::vec3(0.0f, 0.0f, 180.0f);
	basket.position   = glm::vec3(0.0f, 5.0f, -2.5f);
	basket.textureTag = "basket_tex";
	basket.uvScale    = glm::vec2(1.5f, 1.0f);
	AddObject(basket);

	// ---- noise maker to the right ----
	SceneObject noiseMaker;
	noiseMaker.name        = "noise_maker";
	noiseMaker.mesh        = MeshType::Cylinder;
	noiseMaker.scale       = glm::vec3(0.3f, 0.80f, 0.3f);
	noiseMaker.position    = glm::vec3(2.0f, 4.0f, -2.5f);
	noiseMaker.color       = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	noiseMaker.materialTag = "plastic_white";
	noiseMaker.useLighting = true;
	AddObject(noiseMaker);

	// ---- fan: a stand (parent) with the ring head as its child ----
	SceneObject fanStand;
	fanStand.name     = "fan_stand";
	fanStand.mesh     = MeshType::Cylinder;
	fanStand.scale    = glm::vec3(0.5f, 1.0f, 0.5f);
	fanStand.position = glm::vec3(4.0f, 1.0f, -2.0f);
	fanStand.color    = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	int fanIndex = AddObject(fanStand);

	SceneObject fanHead;
	fanHead.name        = "fan_head";
	fanHead.mesh        = MeshType::Torus;
	fanHead.scale       = glm::vec3(0.75f, 2.2f, 0.5f);
	fanHead.rotation    = glm::vec3(0.0f, 110.0f, 0.0f);
	fanHead.position    = glm::vec3(0.0f, 2.5f, 0.0f); // relative to the stand
	fanHead.color       = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	fanHead.parentIndex = fanIndex;
	AddObject(fanHead);
}

/***********************************************************
 *  ComputeWorldMatrix()
 *
 *  Builds the final world matrix for one object by walking up
 *  its parent chain (a depth-first walk toward the root). A
 *  child's local transform is multiplied on top of its parent's
 *  world transform, which is what makes the hierarchy work.
 *
 *  I added guards here on purpose: a bad index or a parent that
 *  points at itself / forms a loop would otherwise run forever
 *  or read out of bounds, so I bail out safely if I see that.
 ***********************************************************/
glm::mat4 SceneManager::ComputeWorldMatrix(int objectIndex) const
{
	// guard against a bad index instead of indexing blindly
	if (objectIndex < 0 || objectIndex >= static_cast<int>(m_sceneObjects.size()))
	{
		return glm::mat4(1.0f);
	}

	const SceneObject& obj = m_sceneObjects.at(objectIndex);

	// build this object's own local transform: T * Rx * Ry * Rz * S
	glm::mat4 local = glm::mat4(1.0f);
	local = glm::translate(local, obj.position);
	local = glm::rotate(local, glm::radians(obj.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	local = glm::rotate(local, glm::radians(obj.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	local = glm::rotate(local, glm::radians(obj.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	local = glm::scale(local, obj.scale);

	// a parent of -1 means this is a root object, so its local IS its world
	int parent = obj.parentIndex;

	// only recurse when the parent looks valid and is not itself (no self-loop)
	if (parent >= 0 && parent < static_cast<int>(m_sceneObjects.size()) && parent != objectIndex)
	{
		return ComputeWorldMatrix(parent) * local;
	}

	return local;
}

/***********************************************************
 *  RenderObject()
 *
 *  Draws a single object from its stored data. Sets up the
 *  texture/color/material first, then looks the mesh up in the
 *  map and calls whatever draw function is registered for it.
 ***********************************************************/
void SceneManager::RenderObject(const SceneObject& object, const glm::mat4& worldMatrix)
{
	if (m_pShaderManager == NULL)
	{
		return;
	}

	// push the world matrix we already built from the parent chain
	SetModelMatrix(worldMatrix);

	// lighting flag per object
	m_pShaderManager->setIntValue("bUseLighting", object.useLighting);

	// material is optional
	if (!object.materialTag.empty())
	{
		SetShaderMaterial(object.materialTag);
	}

	// texture if it has one, otherwise fall back to the solid color
	if (!object.textureTag.empty())
	{
		SetShaderTexture(object.textureTag);
		SetTextureUVScale(object.uvScale.x, object.uvScale.y);
	}
	else
	{
		SetShaderColor(object.color.r, object.color.g, object.color.b, object.color.a);
	}

	// look the mesh up in the map and draw it. find() keeps us safe if a
	// mesh type was somehow never registered - we just skip instead of crash
	auto it = m_meshRegistry.find(object.mesh);
	if (it != m_meshRegistry.end() && it->second)
	{
		it->second();
	}
}

/***********************************************************
 *  PrepareScene()
 *
 *  Loads the meshes and textures, sets up materials, fills in
 *  the mesh lookup map, and builds the scene-graph data. After
 *  this runs everything the renderer needs is just data sitting
 *  in the vector and the map.
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadSphereMesh();

	// hook each mesh type up to its draw call once, up front
	RegisterMeshes();

	// ---- load the textures ----
	m_loadedTextures = 0;
	CreateGLTexture("Textures/Carpet.jpg", "floor_carpet");
	CreateGLTexture("Textures/Walnut.jpg", "heater_walnut");
	CreateGLTexture("Textures/Fire.jpg", "heater_fire");
	CreateGLTexture("Textures/Wicker.jpg", "basket_tex");

	// bind all loaded textures to OpenGL texture units
	BindGLTextures();

	// ---- materials for the Phong lighting ----
	m_objectMaterials.clear();

	OBJECT_MATERIAL carpetMat;
	carpetMat.tag = "carpet";
	carpetMat.ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	carpetMat.ambientStrength = 0.15f;              // keeps the floor from going fully dark
	carpetMat.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	carpetMat.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	carpetMat.shininess = 8.0f;                     // soft highlights
	m_objectMaterials.push_back(carpetMat);

	OBJECT_MATERIAL plasticMat;
	plasticMat.tag = "plastic_white";
	plasticMat.ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	plasticMat.ambientStrength = 0.2f;
	plasticMat.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	plasticMat.specularColor = glm::vec3(0.7f, 0.7f, 0.7f); // bright highlight
	plasticMat.shininess = 32.0f;                           // fairly glossy
	m_objectMaterials.push_back(plasticMat);

	// finally, build the scene as data
	BuildSceneGraph();
}

/***********************************************************
 *  RenderScene()
 *
 *  New data-driven render loop. Instead of a fixed list of
 *  DrawHeater/DrawBasket/etc. calls, it:
 *    1) builds a draw order (sorted back-to-front by depth so
 *       see-through objects blend correctly),
 *    2) builds each object's world matrix from the parent chain,
 *    3) draws each object from its stored data.
 *  Adding a new object is now just pushing one more struct in
 *  BuildSceneGraph - the loop below never has to change.
 ***********************************************************/
void SceneManager::RenderScene()
{
	SetWarmSunLight();

	// nothing to do if the scene is empty
	if (m_sceneObjects.empty())
	{
		return;
	}

	// build a list of indices to draw, then sort that instead of moving the
	// objects themselves around (parent indices have to stay valid)
	std::vector<int> drawOrder;
	drawOrder.reserve(m_sceneObjects.size());
	for (int i = 0; i < static_cast<int>(m_sceneObjects.size()); ++i)
	{
		drawOrder.push_back(i);
	}

	// pre-compute each object's world matrix once so the sort and the draw
	// pass do not both recompute it
	std::vector<glm::mat4> worldMatrices(m_sceneObjects.size());
	for (int i = 0; i < static_cast<int>(m_sceneObjects.size()); ++i)
	{
		worldMatrices[i] = ComputeWorldMatrix(i);
	}

	// depth sort: farthest object first. The world Z (column 3, row 2 of the
	// matrix) is the object's position, so sorting on that draws back-to-front
	// which is what transparency blending needs.
	std::sort(drawOrder.begin(), drawOrder.end(),
		[&worldMatrices](int a, int b)
		{
			return worldMatrices[a][3].z < worldMatrices[b][3].z;
		});

	// now draw everything in the sorted order
	for (int index : drawOrder)
	{
		RenderObject(m_sceneObjects[index], worldMatrices[index]);
	}
}

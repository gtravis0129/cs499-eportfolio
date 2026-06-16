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
 *  DrawFan()
 *
 *  This method is used for drawing the fan using
 *  a cylinder (stand) and a torus (fan head)
 ***********************************************************/
void SceneManager::DrawFan(const glm::vec3& positionXYZ)
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;

	/**************************************************************
	**  Fan Stand  (cylinder from floor up)
	**************************************************************/
	scaleXYZ = glm::vec3(0.5f, 1.0f, 0.5f);          
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	SetTransformations(scaleXYZ, XrotationDegrees,
		YrotationDegrees, ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.8f, 0.8f, 0.8f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	/**************************************************************
	**  Fan Head  (torus on top of the stand)
	**************************************************************/
	scaleXYZ = glm::vec3(.75f, 2.2f, 0.5f);            // overall ring size
	XrotationDegrees = 0.0;
	YrotationDegrees = 110.0f;                          // face toward +Z 
	ZrotationDegrees = 0.0f;

	// place torus above top of stand
	glm::vec3 headPos = positionXYZ + glm::vec3(0.0f, 2.5f, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees,
		YrotationDegrees, ZrotationDegrees,
		headPos);

	SetShaderColor(0.8f, 0.8f, 0.8f, 1.0f);
	m_basicMeshes->DrawTorusMesh();
}


/***********************************************************
 *  DrawNoiseMaker()
 *
 *  Tall cylinder to the right of the basket.
 ***********************************************************/
void SceneManager::DrawNoiseMaker(const glm::vec3& positionXYZ)
{
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;

	// Tall, thin cylinder
	glm::vec3 scaleXYZ = glm::vec3(
		0.3f,   // radius X
		0.80f,   // height Y
		0.3f    // radius Z
	);

	glm::vec3 pos = positionXYZ;


	// Enable lighting and use plastic material
	m_pShaderManager->setIntValue("bUseLighting", true);
	SetShaderMaterial("plastic_white");
	m_pShaderManager->setIntValue("bUseTexture", false); // pure material color

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, pos);

	// For now, plain white; we can texture it later
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);

	m_basicMeshes->DrawCylinderMesh();
}   


/***********************************************************
 *  DrawBasket()
 *
 *  Basket using one tapered cylinder mesh.
 ***********************************************************/
void SceneManager::DrawBasket(const glm::vec3& positionXYZ)
{
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 180.0f;

	// Use walnut basket texture
	SetShaderTexture("basket_tex");
	SetTextureUVScale(1.5f, 1.0f);

	// Overall basket size (adjust to match what you had)
	glm::vec3 scaleXYZ = glm::vec3(
		1.5f,   // width (X)
		1.2f,   // height (Y)
		1.5f    // depth (Z)
	);

	glm::vec3 pos = positionXYZ;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, pos);

	// Single tapered cylinder mesh
	m_basicMeshes->DrawTaperedCylinderMesh();
}


/***********************************************************
 *  DrawHeater()
 *
 *  This method is used for constructing the heater model
 *  from multiple box meshes and positioning it in the scene.
 ***********************************************************/
void SceneManager::DrawHeater(const glm::vec3& positionXYZ)
{
	float heaterScale = 2.5f;
	glm::vec3 scaleXYZ;
	glm::vec3 pos;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;

	// Heater body: main wooden box.
	scaleXYZ = glm::vec3(
		2.0f * heaterScale,
		1.6f * heaterScale,
		0.5f * heaterScale
	);
	pos = positionXYZ + glm::vec3(0.0f, 0.8f * heaterScale, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, pos);
	
	// Use walnut texture instead of flat color
	SetShaderTexture("heater_walnut");
	SetTextureUVScale(1.5f, 1.0f);
	
	m_basicMeshes->DrawBoxMesh();

	// Front grill: inset panel on front face.
	float grillInset = 0.85f;  // width inset ratio.
	float grillHeight = 0.70f;  // height ratio relative to body.

	scaleXYZ = glm::vec3(
		2.0f * heaterScale * grillInset,
		1.6f * heaterScale * grillHeight,
		0.04f * heaterScale
	);

	pos = positionXYZ + glm::vec3(
		0.0f,
		0.8f * heaterScale - (0.12f * heaterScale),
		0.26f * heaterScale
	);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, pos);
	
	// Fire texture on front grill
	SetShaderTexture("heater_fire");
	SetTextureUVScale(1.0f, 1.0f);
	
	m_basicMeshes->DrawBoxMesh();

	// Top strip: thin decorative strip along top front edge.
	scaleXYZ = glm::vec3(
		1.6f * heaterScale,
		0.15f * heaterScale,
		0.05f * heaterScale
	);
	pos = positionXYZ + glm::vec3(0.0f, 1.45f * heaterScale, 0.28f * heaterScale);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, pos);
	SetShaderColor(0.55f, 0.38f, 0.20f, 1.0f);
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
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

	// ============================
	// Milestone Four: load textures
	// ============================

	// reset texture count (if not already done in constructor)
	m_loadedTextures = 0;

	// Loading the textures that I have fornd from my file
	CreateGLTexture("Textures/Carpet.jpg", "floor_carpet");
	CreateGLTexture("Textures/Walnut.jpg", "heater_walnut");
	CreateGLTexture("Textures/Fire.jpg", "heater_fire");
	CreateGLTexture("Textures/Wicker.jpg", "basket_tex");


	// bind all loaded textures to OpenGL texture units
	BindGLTextures();

	// ============================
	// Milestone Five: materials
	// ============================

	// Clear any previous materials (optional but safe)
	m_objectMaterials.clear();

	// Carpet (floor) material for Phong lighting
	OBJECT_MATERIAL carpetMat;
	carpetMat.tag = "carpet";
	carpetMat.ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	carpetMat.ambientStrength = 0.15f;              // keeps floor from going fully dark
	carpetMat.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	carpetMat.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	carpetMat.shininess = 8.0f;              // soft highlights

	m_objectMaterials.push_back(carpetMat);

	//For a plastic look 
	OBJECT_MATERIAL plasticMat;
	plasticMat.tag = "plastic_white";
	plasticMat.ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	plasticMat.ambientStrength = 0.2f;
	plasticMat.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	plasticMat.specularColor = glm::vec3(0.7f, 0.7f, 0.7f); // bright highlight
	plasticMat.shininess = 32.0f;                       // fairly glossy

	m_objectMaterials.push_back(plasticMat);
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	SetWarmSunLight();

	// =========================
	// Floor (lit with Phong)
	// =========================
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	// turn ON lighting for the plane
	m_pShaderManager->setIntValue("bUseLighting", true);
	SetShaderMaterial("carpet");          // use the material you added in PrepareScene
	SetShaderTexture("floor_carpet");     // carpet texture
	SetTextureUVScale(2.0f, 1.0f);

	m_basicMeshes->DrawPlaneMesh();

	// =========================
	// Heater (texture only)
	// =========================
	// turn OFF lighting so heater just shows textures
	m_pShaderManager->setIntValue("bUseLighting", false);

	DrawHeater(glm::vec3(0.0f, 0.0f, -2.5f));


	// =========================
	// Basket on top of heater
	// =========================

	// same lighting setting as heater 
	glm::vec3 basketPos = glm::vec3(0.0f,5.0f, -2.5f);   
	DrawBasket(basketPos);



	// =========================
	// Noise maker to the right
	// =========================

	// Slightly right (X) and same depth (Z) as basket; Y about same as heater top
	glm::vec3 noisePos = glm::vec3(2.0f, 4.0f, -2.5f);
	DrawNoiseMaker(noisePos);

	//=================================================
	// Dyson Fan to the right of the main heater scene
	//=================================================
	glm::vec3 fanPos = glm::vec3(4.0f, 1.0f, -2.0f);
	DrawFan(fanPos);


}
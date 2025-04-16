#pragma once

enum ShaderType
{
	VS = 0,				
	HS = 1,				
	DS = 2,				
	GS = 3,				
	PS = 4,				
	CS = 5,				
	ShaderTypeEnd = 6	
};

enum TextureType
{
	TextureType_DIFFUSE = 0,			
	TextureType_SPECULAR = 1,			 
	TextureType_AMBIENT = 2,			
	TextureType_EMISSIVE = 3,			
	TextureType_HEIGHT = 4,				
	TextureType_NORMALS = 5,			
	TextureType_SHININESS = 6,			
	TextureType_OPACITY = 7,			
	TextureType_DISPLACEMENT = 8,		
	TextureType_LIGHTMAP = 9,			
	TextureType_REFLECTION = 10,		
	TextureType_BASE_COLOR = 11,		
	TextureType_NORMAL_CAMERA = 12,		
	TextureType_EMISSION_COLOR = 13,	
	TextureType_METALNESS = 14,			
	TextureType_DIFFUSE_ROUGHNESS = 15,	
	TextureType_AMBIENT_OCCLUSION = 16,	
	TextureType_SHEEN = 17,				
	TextureType_CLEARCOAT = 18,			
	TextureType_TRANSMISSION = 19,		
	TextureType_END = 20				
};

enum VIBufferType
{
	VIBufferType_POSTEX_RECT = 0,
	VIBufferType_POSCOLOR_CUBE = 1,
	VIBufferType_END = 2
};

enum ApplicationMode
{
	NONE,
	EDITOR,
	CLIENT,
	PREFAB
};
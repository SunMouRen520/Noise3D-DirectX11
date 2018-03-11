
/***********************************************************************

							class��NOISE texture Manger

							texture management/modification

************************************************************************/

#include "Noise3D.h"

using namespace Noise3D;

ITextureManager::ITextureManager():
	IFactory<ITexture>(50000)//maxCount of Textures
{

};

ITextureManager::~ITextureManager()
{
	DeleteAllTexture();
}


//--------------------------------TEXTURE CREATION-----------------------------
ITexture* ITextureManager::CreatePureColorTexture(N_UID texName, UINT pixelWidth, UINT pixelHeight, NVECTOR4 color, bool keepCopyInMemory)
{
	//create New Texture Object
	HRESULT hr = S_OK;

	//we must check if new name has been used
	if(ValidateUID(texName)== true)
	{
			ERROR_MSG("CreateTextureFromFile : Texture name has been used!! name: " + texName);
			return nullptr;//invalid
	}

#pragma region CreateTex2D&SRV

	//a temporary Texture Object
	/*N_TextureObject tmpTexObj;
	tmpTexObj.mIsPixelBufferInMemValid = keepCopyInMemory;
	tmpTexObj.mTexName = tmpStringTextureName;
	tmpTexObj.mTextureType = NOISE_TEXTURE_TYPE_COMMON;*/

	//assign a lot of same NVECTOR4 color to vector
	std::vector<NColor4u> initPixelBuffer(pixelWidth*pixelHeight, color);


	//texture2D desc (create a default usage texture)
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = pixelWidth;//determined by the loaded picture
	texDesc.Height = pixelHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = c_DefaultPixelDxgiFormat;// DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;//allow update subresource
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = NULL;
	texDesc.MiscFlags = NULL;

	//SRV desc
	D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
	SRViewDesc.Format = texDesc.Format;
	SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SRViewDesc.TextureCube.MostDetailedMip = 0;

	//init data
	D3D11_SUBRESOURCE_DATA texInitDataDesc;
	texInitDataDesc.pSysMem = &initPixelBuffer.at(0);
	texInitDataDesc.SysMemPitch = (texDesc.Width) * NOISE_MACRO_DEFAULT_COLOR_BYTESIZE;//bytesize for 1 row
	texInitDataDesc.SysMemSlicePitch = 0;//available for tex3D

	 //create texture2D first 
	ID3D11Texture2D* pTmpTexture2D;
	hr = g_pd3dDevice11->CreateTexture2D(&texDesc, &texInitDataDesc, &pTmpTexture2D);
	if (FAILED(hr))
	{
		ReleaseCOM(pTmpTexture2D);
		ERROR_MSG("CreateTextureFromFile : Create ID3D11Texture2D failed!");
		return nullptr;
	}

	//Create SRV from texture 2D (to a tmp textureObject)
	ID3D11ShaderResourceView* tmp_pSRV = nullptr;
	hr = g_pd3dDevice11->CreateShaderResourceView(pTmpTexture2D, &SRViewDesc, &tmp_pSRV);
	if (FAILED(hr))
	{
		ReleaseCOM(pTmpTexture2D);
		ERROR_MSG("CreateTextureFromFile : Create ID3D11SRV failed!");
		return nullptr;
	}


	//clear tmp interfaces
	ReleaseCOM(pTmpTexture2D);

	//clear Memory of the picture
	if (!keepCopyInMemory)initPixelBuffer.clear();

#pragma endregion CreateTex2D&SRV

	//at last push back a new texture object
	ITexture* pTexObj= IFactory<ITexture>::CreateObject(texName);
	pTexObj->mFunction_InitTexture(tmp_pSRV, texName, std::move(initPixelBuffer), keepCopyInMemory, NOISE_TEXTURE_TYPE_COMMON);

	return pTexObj;
};

ITexture* ITextureManager::CreateTextureFromFile(NFilePath filePath, N_UID texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight,bool keepCopyInMemory)
{
	//read file to memory
	IFileIO fileIO;
	std::vector<char> fileBuff;
	bool isReadFileSucceeded = fileIO.ImportFile_PURE(filePath, fileBuff);
	if (!isReadFileSucceeded)
	{
		ERROR_MSG("CreateTextureFromFile : failed to read file!!");
		return nullptr;//invalid
	}

	//check if new name has been used
	//count() will return 0 if given key dont exists
	if (ValidateUID(texName) == true)
	{
		ERROR_MSG("CreateTextureFromFile : Texture name has been used!!");
		return nullptr;//invalid
	}

#pragma region DirectXTex load Image

	//enumerate file formats, and parse image file using different functions
	std::string fileSubfix = gFunc_GetFileSubFixFromPath(filePath);
	NOISE_IMAGE_FILE_FORMAT fileFormat = mFunction_GetImageFileFormat(fileSubfix);
	DirectX::TexMetadata scrMetaData;//meta data is loaded from target image file
	DirectX::ScratchImage srcImage;

	switch (fileFormat)
	{
		//Supported by DirectXTex.WIC
		case NOISE_IMAGE_FILE_FORMAT_BMP:
		case NOISE_IMAGE_FILE_FORMAT_JPG:
		case NOISE_IMAGE_FILE_FORMAT_PNG:
		case NOISE_IMAGE_FILE_FORMAT_TIFF:
		case NOISE_IMAGE_FILE_FORMAT_GIF:
		{
			DirectX::LoadFromWICMemory(&fileBuff.at(0), fileBuff.size(), DirectX::WIC_FLAGS_NONE, &scrMetaData, srcImage);
			break;
		}

		case NOISE_IMAGE_FILE_FORMAT_HDR:
		{
			DirectX::LoadFromHDRMemory(&fileBuff.at(0), fileBuff.size(), &scrMetaData, srcImage);
			break;
		}

		case NOISE_IMAGE_FILE_FORMAT_TGA:
		{
			DirectX::LoadFromTGAMemory(&fileBuff.at(0), fileBuff.size(), &scrMetaData, srcImage);
			break;
		}

		case NOISE_IMAGE_FILE_FORMAT_DDS :
		{
			DirectX::LoadFromDDSMemory(&fileBuff.at(0), fileBuff.size(),DirectX::DDS_FLAGS_NONE , &scrMetaData, srcImage);
			break;
		}

		case NOISE_IMAGE_FILE_FORMAT_NOT_SUPPORTED:
		default:
			ERROR_MSG("CreateTextureFromFile : image file format not supported!! format:"+ fileSubfix);
			return nullptr;//invalid
			break;
	}

	//load and resize image data to a memory block
	uint32_t resizedImageWidth = useDefaultSize ? scrMetaData.width : pixelWidth;
	uint32_t resizedImageHeight = useDefaultSize ? scrMetaData.height : pixelHeight;

	//re-sampling of original images

	DirectX::ScratchImage resizedImage;
	DirectX::ScratchImage convertedImage;

	//resize the image
	DirectX::Resize(
		srcImage.GetImages(), 
		srcImage.GetImageCount(),
		srcImage.GetMetadata(), 
		resizedImageWidth, 
		resizedImageHeight,
		DirectX::TEX_FILTER_DEFAULT,//DirectX::TEX_FILTER_LINEAR,
		resizedImage);

	//pixel format conversion (this format must match the format defined in ID3D11Texture2D desc)
	if (resizedImage.GetMetadata().format != c_DefaultPixelDxgiFormat)
	{
		DirectX::Convert(
			resizedImage.GetImages(),
			resizedImage.GetImageCount(),
			resizedImage.GetMetadata(),
			c_DefaultPixelDxgiFormat,
			DirectX::TEX_FILTER_DEFAULT,
			DirectX::TEX_THRESHOLD_DEFAULT,
			convertedImage);
	}
	else
	{
		//copy
		convertedImage.InitializeFromImage(*resizedImage.GetImages());
	}

#pragma endregion DirectXTex load Image

#pragma region CreateTexture2D & SRV

	//texture2D desc (create a default usage texture)
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = resizedImageWidth;//determined by the loaded picture
	texDesc.Height = resizedImageHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = c_DefaultPixelDxgiFormat;//DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;//allow update subresource
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = NULL;
	texDesc.MiscFlags = NULL;

	//SRV desc
	D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
	SRViewDesc.Format = texDesc.Format;
	SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SRViewDesc.TextureCube.MostDetailedMip = 0;

	//init data
	D3D11_SUBRESOURCE_DATA texInitDataDesc;
	texInitDataDesc.pSysMem = convertedImage.GetPixels() ;// &pixelBuffer.at(0);
	texInitDataDesc.SysMemPitch = (texDesc.Width) * NOISE_MACRO_DEFAULT_COLOR_BYTESIZE;//bytesize for 1 row
	texInitDataDesc.SysMemSlicePitch = 0;//available for tex3D

	//create ID3D11Texture2D first 
	ID3D11Texture2D* pTmpTexture2D;
	HRESULT hr = g_pd3dDevice11->CreateTexture2D(&texDesc, &texInitDataDesc, &pTmpTexture2D);
	if (FAILED(hr))
	{
		ReleaseCOM(pTmpTexture2D);
		ERROR_MSG("CreateTextureFromFile : Create ID3D11Texture2D failed!");
		return nullptr;
	}

	//Create SRV from texture 2D (to a tmp textureObject)
	ID3D11ShaderResourceView* tmp_pSRV = nullptr;
	hr = g_pd3dDevice11->CreateShaderResourceView(pTmpTexture2D, &SRViewDesc, &tmp_pSRV);
	if (FAILED(hr))
	{
		ReleaseCOM(pTmpTexture2D);
		ERROR_MSG("CreateTextureFromFile : Create ID3D11SRV failed!");
		return nullptr;
	}

#pragma endregion CreateTexture2D & SRV

	//clear tmp interfaces
	ReleaseCOM(pTmpTexture2D);

	//at last, create a new texture object ptr
	ITexture* pTexObj = IFactory<ITexture>::CreateObject(texName);
	if (keepCopyInMemory)
	{
		uint32_t pixelCount = resizedImageWidth*resizedImageHeight;
		uint8_t* pData = convertedImage.GetPixels();
		std::vector<NColor4u> pixelBuffer(pixelCount);

		//copy data of converted image to ITexture's system memory
		for (uint32_t pixelId = 0; pixelId<pixelCount; ++pixelId)
		{
			pixelBuffer[pixelId] = *(NColor4u*)(pData + pixelId*NOISE_MACRO_DEFAULT_COLOR_BYTESIZE);
		}

		pTexObj->mFunction_InitTexture(tmp_pSRV, texName, std::move(pixelBuffer), true, NOISE_TEXTURE_TYPE_COMMON);
	}
	else
	{
		std::vector<NColor4u> emptyBuff;
		pTexObj->mFunction_InitTexture(tmp_pSRV, texName, std::move(emptyBuff), false, NOISE_TEXTURE_TYPE_COMMON);
	}

	return pTexObj;//invalid file or sth else
}

ITexture* ITextureManager::CreateCubeMapFromDDS(NFilePath dds_FileName, N_UID cubeTextureName, NOISE_CUBEMAP_SIZE faceSize)
{
	bool isFileNameValid;
	for (UINT i = 0; i < 6;i++)
	{
		//try opening a file
		isFileNameValid = std::fstream(dds_FileName).good();

		//check if  one of the texture id is illegal
		if (!isFileNameValid)
		{
			ERROR_MSG("Noise Tex Mgr :CreateCubeTextureFromDDS : file not exist!! ; Index : ");
			return nullptr;
		}
	}

#pragma region CreateSRVFromDDS

	//some settings about loading image
	D3DX11_IMAGE_LOAD_INFO loadInfo;

	//we must check if user want to use default image size
	switch(faceSize)
	{
	case NOISE_CUBEMAP_SIZE_64x64:
		loadInfo.Width = 64;
		loadInfo.Height = 64;
		break;
	case NOISE_CUBEMAP_SIZE_128x128:
		loadInfo.Width = 128;
		loadInfo.Height = 128;
		break;
	case NOISE_CUBEMAP_SIZE_256x256:
		loadInfo.Width = 256;
		loadInfo.Height = 256;
		break;
	case NOISE_CUBEMAP_SIZE_512x512:
		loadInfo.Width = 512;
		loadInfo.Height = 512;
		break;
	case NOISE_CUBEMAP_SIZE_1024x1024:
		loadInfo.Width = 1024;
		loadInfo.Height = 1024;
		break;
	};
	

	//continue filling the settings
	loadInfo.Filter = D3DX11_FILTER_LINEAR;
	loadInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	loadInfo.Format = c_DefaultPixelDxgiFormat;

	//create New Texture Object
	HRESULT hr = S_OK;

	//then endow a pointer to new SRV
	ID3D11ShaderResourceView* tmp_pSRV = nullptr;
	D3DX11CreateShaderResourceViewFromFileA(
		g_pd3dDevice11,
		dds_FileName.c_str(),
		&loadInfo,
		nullptr,
		&tmp_pSRV,
		&hr
		);

	//................
	if (FAILED(hr))
	{
		ERROR_MSG("CreateCubeMapFromDDS : Create SRV failed!");
		return nullptr;
	}


#pragma endregion CreateSRVFromDDS

	//Create a new Texture object
	ITexture* pTexObj = IFactory<ITexture>::CreateObject(cubeTextureName);
	std::vector<NColor4u> emptyBuff;
	pTexObj->mFunction_InitTexture(tmp_pSRV, cubeTextureName, std::move(emptyBuff), false, NOISE_TEXTURE_TYPE_CUBEMAP);

	//return new texObj ptr
	return pTexObj;
}

ITexture * ITextureManager::GetTexture(N_UID texName)
{
	return IFactory<ITexture>::GetObjectPtr(texName);
}

UINT	 ITextureManager::GetTextureCount()
{
	return IFactory<ITexture>::GetObjectCount();
}

bool ITextureManager::DeleteTexture(ITexture * pTex)
{
	if (pTex == nullptr)
	{
		return false;
	}
	else
	{
		return IFactory<ITexture>::DestroyObject(pTex->GetTextureName());
	}
}

bool ITextureManager::DeleteTexture(N_UID uid)
{
	auto texturePtr = IFactory<ITexture>::GetObjectPtr(uid);
	return IFactory<ITexture>::DestroyObject(uid);
}

void ITextureManager::DeleteAllTexture()
{
	IFactory<ITexture>::DestroyAllObject();
}

bool ITextureManager::ValidateUID(N_UID texName)
{
	return IFactory<ITexture>::FindUid(texName);
}

bool ITextureManager::ValidateUID(N_UID texName, NOISE_TEXTURE_TYPE texType)
{
	if (IFactory<ITexture>::FindUid(texName) == false)
	{
		return false;
	}

	//then check the texture type : common / cubemap / volumn
	D3D11_SHADER_RESOURCE_VIEW_DESC tmpSRViewDesc;
	IFactory<ITexture>::GetObjectPtr(texName)->m_pSRV->GetDesc(&tmpSRViewDesc);

	switch (tmpSRViewDesc.ViewDimension)
	{
	case D3D11_SRV_DIMENSION_TEXTURECUBE:
		if (texType != NOISE_TEXTURE_TYPE_CUBEMAP)
		{
			return false;
		}
		break;

	case D3D11_SRV_DIMENSION_TEXTURE3D:
		if (texType != NOISE_TEXTURE_TYPE_VOLUME)
		{
			return false;
		}
		break;

	case D3D11_SRV_DIMENSION_TEXTURE2D:
		if (texType != NOISE_TEXTURE_TYPE_COMMON)
		{
			return false;
		}
		break;

	}

	//a no-problem texID
	return true;
}


/*************************************************************
									P R I V A T E
*************************************************************/
NOISE_IMAGE_FILE_FORMAT Noise3D::ITextureManager::mFunction_GetImageFileFormat(const std::string & fileSubfix)
{
	std::string lowCaseSubfix;
	for (auto c : fileSubfix)lowCaseSubfix.push_back(::tolower(c));

	if (lowCaseSubfix == "bmp" )return NOISE_IMAGE_FILE_FORMAT_BMP;
	if (lowCaseSubfix == "png")return NOISE_IMAGE_FILE_FORMAT_PNG;
	if (lowCaseSubfix == "jpg")return NOISE_IMAGE_FILE_FORMAT_JPG;
	if (lowCaseSubfix == "tiff")return NOISE_IMAGE_FILE_FORMAT_TIFF;
	if (lowCaseSubfix == "gif")return NOISE_IMAGE_FILE_FORMAT_GIF;
	if (lowCaseSubfix == "tga")return NOISE_IMAGE_FILE_FORMAT_TGA;
	if (lowCaseSubfix == "hdr")return NOISE_IMAGE_FILE_FORMAT_HDR;
	if (lowCaseSubfix == "dds")return NOISE_IMAGE_FILE_FORMAT_DDS;
	return NOISE_IMAGE_FILE_FORMAT_NOT_SUPPORTED;
}


/*ITexture* ITextureManager::mFunction_CreateTextureFromFile_DirectlyLoadToGpu(NFilePath filePath, std::string& texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight)
{
	//!!!!!!!!!!!!!!!!File Size Maybe a problem???

	//check if the file existed
	std::fstream tmpFile(filePath, std::ios::binary | std::ios::in);
	if (tmpFile.bad())
	{
		ERROR_MSG("Texture file not found!!");
		return nullptr;//invalid
	}

	//some settings about loading image
	D3DX11_IMAGE_LOAD_INFO loadInfo;

	//we must check if user want to use default image size
	if (useDefaultSize)
	{
		loadInfo.Width = D3DX11_DEFAULT;
		loadInfo.Height = D3DX11_DEFAULT;
	}
	else
	{
		loadInfo.Width = (pixelWidth > 0 ? pixelWidth : D3DX11_DEFAULT);
		loadInfo.Height = (pixelHeight > 0 ? pixelHeight : D3DX11_DEFAULT);
	}

	//continue filling the settings
	loadInfo.Filter = D3DX11_FILTER_LINEAR;
	loadInfo.MiscFlags = D3DX11_DEFAULT;
	loadInfo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	loadInfo.MiscFlags = D3DX11_DEFAULT;
	loadInfo.CpuAccessFlags = NULL;// cpuAccessFlag;
	loadInfo.Usage =  D3D11_USAGE_DEFAULT;//bufferUsage;


	//create New Texture Object
	HRESULT hr = S_OK;

	//we must check if new name has been used
	if(ValidateUID(texName)== true)
	{
		ERROR_MSG("CreateTextureFromFile : Texture name has been used!!");
		return nullptr;//invalid
	}

	//then endow a pointer to new SRV
	ID3D11ShaderResourceView* tmp_pSRV = nullptr;
	D3DX11CreateShaderResourceViewFromFileA(
		g_pd3dDevice11,
		filePath.c_str(),
		&loadInfo,
		nullptr,
		&tmp_pSRV,
		&hr
		);
		
	//................
	HR_DEBUG_CREATETEX(hr, "CreateTextureFromFile : Create SRV failed ! keepCopyInMem:false");

	//at last create  a new texture object
	ITexture* pTexObj = IFactory<ITexture>::CreateObject(texName);
	std::vector<NVECTOR4> emptyBuff;
	pTexObj->mFunction_InitTexture(tmp_pSRV, texName, std::move(emptyBuff), false, NOISE_TEXTURE_TYPE_COMMON);

	return pTexObj;//invalid file or sth else
}*/

/*ITexture* ITextureManager::mFunction_CreateTextureFromFile_KeepACopyInMemory(NFilePath filePath, std::string& texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight)
{
	//!!!!!!!!!!!!!!!!File Size Maybe a problem???

	//check if the file existed
	std::fstream tmpFile(filePath, std::ios::binary | std::ios::in);
	if (tmpFile.bad())
	{
		ERROR_MSG("CreateTextureFromFile : Texture file not found!!");
		return nullptr;//invalid
	}


	//create New Texture Object
	HRESULT hr = S_OK;

	//we must check if new name has been used
	//count() will return 0 if given key dont exists
	if (ValidateUID(texName)== true)
	{
		ERROR_MSG("CreateTextureFromFile : Texture name has been used!!");
		return nullptr;//invalid
	}


#pragma region LoadPixelMatrix


	//some settings about loading image
	D3DX11_IMAGE_LOAD_INFO loadInfo;

	//we must check if user want to use default image size
	if (useDefaultSize)
	{
		loadInfo.Width = D3DX11_DEFAULT;
		loadInfo.Height = D3DX11_DEFAULT;
	}
	else
	{
		loadInfo.Width = (pixelWidth > 0 ? pixelWidth : D3DX11_DEFAULT);
		loadInfo.Height = (pixelHeight > 0 ? pixelHeight : D3DX11_DEFAULT);
	}

	//we must create a texture with  STAGING / DYNAMIC or whatever not default   usage
	loadInfo.Filter = D3DX11_FILTER_LINEAR;
	loadInfo.MiscFlags = D3DX11_DEFAULT;
	loadInfo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	loadInfo.BindFlags = NULL;// D3D11_BIND_SHADER_RESOURCE;
	loadInfo.MiscFlags = NULL;
	loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_READ;// cpuAccessFlag;
	loadInfo.Usage = D3D11_USAGE_STAGING;//bufferUsage;

	//create texture from files
	ID3D11Texture2D* pOriginTexture2D;
	/*D3DX11CreateTextureFromFileA(
		g_pd3dDevice11,
		filePath.c_str(),
		&loadInfo,
		NULL,
		(ID3D11Resource**)&pOriginTexture2D,
		&hr
		);

	HR_DEBUG_CREATETEX(hr, "CreateTexture : Load Texture From File failed! (keep memory copy)");

	//use Map() to get data
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	g_pImmediateContext->Map(pOriginTexture2D, 0, D3D11_MAP_READ, NULL, &mappedSubresource);

	//get picture size info and load pixels to memory
	D3D11_TEXTURE2D_DESC	 originTexDesc;
	pOriginTexture2D->GetDesc(&originTexDesc);

	//I DONT KNOW WHY CreateTextureFromFile()  WILL CHANGE THE SIZE OF TEXTURE !!
	//(2018.3.9)the 'pitch' can be round off to the multiple of 4 to make mem access faster
	UINT loadedTexWidth = mappedSubresource.RowPitch / NOISE_MACRO_DEFAULT_COLOR_BYTESIZE;//loadedTexDesc.Width;
	UINT loadedTexHeight = originTexDesc.Height;

	//I DONT KNOW WHY CreateTextureFromFile() WILL CHANGE THE SIZE OF TEXTURE !!
	//it seems that the width will be scaled to ..... round off....
	//(it's some kind of optimization that will round off to some value)
	std::vector<NVECTOR4> pixelBuffer(loadedTexWidth*loadedTexHeight);
	pixelBuffer.assign(
		(NVECTOR4*)mappedSubresource.pData,
		(NVECTOR4*)mappedSubresource.pData + loadedTexWidth*loadedTexHeight);



	//remember to Unmap() after every mapping
	g_pImmediateContext->Unmap(pOriginTexture2D, 0);

	ReleaseCOM(pOriginTexture2D);
#pragma endregion LoadPixelMatrix

	
#pragma region CreateTex2D&SRV

	//texture2D desc (create a default usage texture)
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = loadedTexWidth;//determined by the loaded picture
	texDesc.Height = loadedTexHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage =  D3D11_USAGE_DEFAULT;//allow update subresource
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags =NULL;
	texDesc.MiscFlags = NULL;

	//SRV desc
	D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
	SRViewDesc.Format = texDesc.Format;
	SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SRViewDesc.TextureCube.MostDetailedMip = 0;

	//init data
	D3D11_SUBRESOURCE_DATA texInitDataDesc;
	texInitDataDesc.pSysMem = &pixelBuffer.at(0);
	texInitDataDesc.SysMemPitch = (texDesc.Width) * NOISE_MACRO_DEFAULT_COLOR_BYTESIZE;//bytesize for 1 row
	texInitDataDesc.SysMemSlicePitch = 0;//available for tex3D

	 //create texture2D first 
	ID3D11Texture2D* pTmpTexture2D;
	hr = g_pd3dDevice11->CreateTexture2D(&texDesc, &texInitDataDesc, &pTmpTexture2D);
	if (FAILED(hr))
	{
		ReleaseCOM(pTmpTexture2D);
		ERROR_MSG("CreateTextureFromFile : Create ID3D11Texture2D failed!");
		return nullptr;
	}

	//Create SRV from texture 2D (to a tmp textureObject)
	ID3D11ShaderResourceView* tmp_pSRV = nullptr;
	hr = g_pd3dDevice11->CreateShaderResourceView(pTmpTexture2D, &SRViewDesc, &tmp_pSRV);
	if (FAILED(hr))
	{
		ReleaseCOM(pTmpTexture2D);
		ERROR_MSG("CreateTextureFromFile : Create ID3D11SRV failed!");
		return nullptr;
	}

#pragma endregion CreateTex2D&SRV

	//clear tmp interfaces
	ReleaseCOM(pTmpTexture2D);

	//at last create  a new texture object
	ITexture* pTexObj = IFactory<ITexture>::CreateObject(texName);
	pTexObj->mFunction_InitTexture(tmp_pSRV, texName, std::move(pixelBuffer), true, NOISE_TEXTURE_TYPE_COMMON);

	return pTexObj;
}*/

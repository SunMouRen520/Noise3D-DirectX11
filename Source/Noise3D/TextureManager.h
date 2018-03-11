
/***********************************************************************

                           h�� Texture manager

************************************************************************/
#pragma once

namespace Noise3D
{

	class /*_declspec(dllexport)*/ ITextureManager :
		public IFactory<ITexture>
	{
	public:
		ITexture*		CreatePureColorTexture(N_UID texName, UINT pixelWidth, UINT pixelHeight, NVECTOR4 color, bool keepCopyInMemory = false);

		ITexture*		CreateTextureFromFile(NFilePath filePath, N_UID texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight, bool keepCopyInMemory = false);

		ITexture*		CreateCubeMapFromDDS(NFilePath dds_FileName, N_UID cubeTextureName, NOISE_CUBEMAP_SIZE faceSize);

		ITexture*		GetTexture(N_UID texName);

		UINT			GetTextureCount();

		bool				DeleteTexture(ITexture* pTex);

		bool				DeleteTexture(N_UID texName);

		void				DeleteAllTexture();

		bool				ValidateUID(N_UID texName);

		bool				ValidateUID(N_UID texName, NOISE_TEXTURE_TYPE texType);

	private:

		friend class IRenderInfrastructure;

		friend IFactory<ITextureManager>;

		//���캯��
		ITextureManager();

		~ITextureManager();

		const DXGI_FORMAT c_DefaultPixelDxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		NOISE_IMAGE_FILE_FORMAT mFunction_GetImageFileFormat(const std::string& fileSubfix);

		//void mFunction_LoadPixelBuffer(uint32_t resizedWidth, uint32_t resizedHeight, std::vector<NColor4u>& targetPixelBuff,const DirectX::TexMetadata& metaData, const DirectX::ScratchImage& image);

		//ITexture*		mFunction_CreateTextureFromFile_DirectlyLoadToGpu(NFilePath filePath, std::string& texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight);

		//ITexture*		mFunction_CreateTextureFromFile_KeepACopyInMemory(NFilePath filePath, std::string& texName, bool useDefaultSize, UINT pixelWidth, UINT pixelHeight);
	
	};

}

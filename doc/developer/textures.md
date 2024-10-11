\page develTextures  Textures management
[TOC]

Texture data and management class.
First setup  Ra::Engine::Data::TextureParameters.
Optionnaly load image with TextureManager::loadTextureImage
Create a Texture directly, or use TextureManager and get access to Texture by TextureManager::getTexture

Loading Texture to the gpu is done asynchronously, when needed, a texture register gpu task to the engine, to load or delete texture.
If you have an active GL context, and want immediate gpu texture upload or delete, call Texture::initializeNow()
or Texture::destroyNow()

\see

- @ref material-textures in Material
- Ra::Engine::Data::MaterialTextureSet
- Ra::Engine::Data::TextureManager
- Ra::Engine::Data::Texture
- Ra::Engine::Data::TextureParameters
- Ra::Engine::Data::SamplerParameters
- Ra::Engine::Data::ImageParameters

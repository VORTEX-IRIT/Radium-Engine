# ----------------------------------------------------
# This file can be generated from a script:
# To do so, run "./generateFilelistForModule.sh Engine"
# from ./scripts directory
# ----------------------------------------------------

set(ENGINE_SOURCES
    Data/BlinnPhongMaterial.cpp
    Data/DrawPrimitives.cpp
    Data/EnvironmentTexture.cpp
    Data/LambertianMaterial.cpp
    Data/MaterialConverters.cpp
    Data/Material.cpp
    Data/Mesh.cpp
    Data/PlainMaterial.cpp
    Data/RawShaderMaterial.cpp
    Data/RenderParameters.cpp
    Data/ShaderConfigFactory.cpp
    Data/ShaderConfiguration.cpp
    Data/ShaderProgram.cpp
    Data/ShaderProgramManager.cpp
    Data/SimpleMaterial.cpp
    Data/Texture.cpp
    Data/TextureManager.cpp
    Data/VolumeObject.cpp
    Data/VolumetricMaterial.cpp
    RadiumEngine.cpp
    Rendering/DebugRender.cpp
    Rendering/ForwardRenderer.cpp
    Rendering/Renderer.cpp
    Rendering/RenderObject.cpp
    Rendering/RenderObjectManager.cpp
    Rendering/RenderTechnique.cpp
    Scene/CameraComponent.cpp
    Scene/CameraManager.cpp
    Scene/Component.cpp
    Scene/ComponentMessenger.cpp
    Scene/DefaultCameraManager.cpp
    Scene/DefaultLightManager.cpp
    Scene/DirLight.cpp
    Scene/Entity.cpp
    Scene/EntityManager.cpp
    Scene/GeometryComponent.cpp
    Scene/GeometrySystem.cpp
    Scene/ItemEntry.cpp
    Scene/Light.cpp
    Scene/LightManager.cpp
    Scene/PointLight.cpp
    Scene/SignalManager.cpp
    Scene/SkeletonBasedAnimationSystem.cpp
    Scene/SkeletonComponent.cpp
    Scene/SkinningComponent.cpp
    Scene/SpotLight.cpp
    Scene/System.cpp
    Scene/SystemDisplay.cpp
)

set(ENGINE_HEADERS
    Data/BlinnPhongMaterial.hpp
    Data/DisplayableObject.hpp
    Data/DrawPrimitives.hpp
    Data/EnvironmentTexture.hpp
    Data/LambertianMaterial.hpp
    Data/MaterialConverters.hpp
    Data/Material.hpp
    Data/Mesh.hpp
    Data/PlainMaterial.hpp
    Data/RawShaderMaterial.hpp
    Data/RenderParameters.hpp
    Data/ShaderConfigFactory.hpp
    Data/ShaderConfiguration.hpp
    Data/ShaderProgram.hpp
    Data/ShaderProgramManager.hpp
    Data/SimpleMaterial.hpp
    Data/Texture.hpp
    Data/TextureManager.hpp
    Data/ViewingParameters.hpp
    Data/VolumeObject.hpp
    Data/VolumetricMaterial.hpp
    FrameInfo.hpp
    OpenGL.hpp
    RadiumEngine.hpp
    RaEngine.hpp
    Rendering/DebugRender.hpp
    Rendering/ForwardRenderer.hpp
    Rendering/Renderer.hpp
    Rendering/RenderObject.hpp
    Rendering/RenderObjectManager.hpp
    Rendering/RenderObjectTypes.hpp
    Rendering/RenderTechnique.hpp
    Scene/CameraComponent.hpp
    Scene/CameraManager.hpp
    Scene/CameraStorage.hpp
    Scene/Component.hpp
    Scene/ComponentMessenger.hpp
    Scene/CouplingSystem.hpp
    Scene/DefaultCameraManager.hpp
    Scene/DefaultLightManager.hpp
    Scene/DirLight.hpp
    Scene/Entity.hpp
    Scene/EntityManager.hpp
    Scene/GeometryComponent.hpp
    Scene/GeometrySystem.hpp
    Scene/ItemEntry.hpp
    Scene/Light.hpp
    Scene/LightManager.hpp
    Scene/LightStorage.hpp
    Scene/PointLight.hpp
    Scene/SignalManager.hpp
    Scene/SkeletonBasedAnimationSystem.hpp
    Scene/SkeletonComponent.hpp
    Scene/SkinningComponent.hpp
    Scene/SpotLight.hpp
    Scene/SystemDisplay.hpp
    Scene/System.hpp
)

set(ENGINE_INLINES
    Data/BlinnPhongMaterial.inl
    Data/EnvironmentTexture.inl
    Data/Material.inl
    Data/Mesh.inl
    Data/RawShaderMaterial.inl
    Data/RenderParameters.inl
    Data/ShaderProgram.inl
    Data/SimpleMaterial.inl
    Data/VolumeObject.inl
    Data/VolumetricMaterial.inl
    Rendering/Renderer.inl
    Rendering/RenderTechnique.inl
    Scene/Component.inl
    Scene/ComponentMessenger.inl
    Scene/DirLight.inl
    Scene/Entity.inl
    Scene/GeometryComponent.inl
    Scene/ItemEntry.inl
    Scene/Light.inl
    Scene/PointLight.inl
    Scene/SpotLight.inl
)

set(ENGINE_SHADERS
    2DShaders/Basic2D.vert.glsl
    2DShaders/CircleBrush.frag.glsl
    2DShaders/ComposeOIT.frag.glsl
    2DShaders/DepthDisplay.frag.glsl
    2DShaders/DrawScreen.frag.glsl
    2DShaders/DrawScreenI.frag.glsl
    2DShaders/Hdr2Ldr.frag.glsl
    HdrToLdr/Hdr2Ldr.vert.glsl
    Lights/DefaultLight.glsl
    Lights/DirectionalLight.glsl
    Lights/PointLight.glsl
    Lights/SpotLight.glsl
    Lines/LinesAdjacency.geom.glsl
    Lines/Lines.frag.glsl
    Lines/Lines.geom.glsl
    Lines/Lines.vert.glsl
    Lines/Wireframe.frag.glsl
    Lines/Wireframe.geom.glsl
    Lines/Wireframe.vert.glsl
    Materials/BlinnPhong/BlinnPhong.frag.glsl
    Materials/BlinnPhong/BlinnPhong.glsl
    Materials/BlinnPhong/BlinnPhong.vert.glsl
    Materials/BlinnPhong/BlinnPhongZPrepass.frag.glsl
    Materials/BlinnPhong/LitOITBlinnPhong.frag.glsl
    Materials/Lambertian/Lambertian.frag.glsl
    Materials/Lambertian/Lambertian.glsl
    Materials/Lambertian/Lambertian.vert.glsl
    Materials/Lambertian/LambertianZPrepass.frag.glsl
    Materials/Plain/Plain.frag.glsl
    Materials/Plain/Plain.glsl
    Materials/Plain/Plain.vert.glsl
    Materials/Plain/PlainZPrepass.frag.glsl
    Materials/VertexAttribInterface.frag.glsl
    Materials/Volumetric/ComposeVolumeRender.frag.glsl
    Materials/Volumetric/Volumetric.frag.glsl
    Materials/Volumetric/Volumetric.glsl
    Materials/Volumetric/VolumetricOIT.frag.glsl
    Materials/Volumetric/Volumetric.vert.glsl
    Picking/Picking.frag.glsl
    Picking/PickingLinesAdjacency.geom.glsl
    Picking/PickingLines.geom.glsl
    Picking/PickingPoints.geom.glsl
    Picking/PickingTriangles.geom.glsl
    Picking/Picking.vert.glsl
    Points/PointCloud.geom.glsl
    Transform/TransformStructs.glsl
)

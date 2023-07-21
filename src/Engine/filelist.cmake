# ----------------------------------------------------
# This file can be generated from a script:
# To do so, run "./generateFilelistForModule.sh Engine"
# from ./scripts directory
# ----------------------------------------------------

set(engine_sources
    Data/BlinnPhongMaterial.cpp
    Data/DrawPrimitives.cpp
    Data/EnvironmentTexture.cpp
    Data/LambertianMaterial.cpp
    Data/Material.cpp
    Data/MaterialConverters.cpp
    Data/Mesh.cpp
    Data/PlainMaterial.cpp
    Data/RawShaderMaterial.cpp
    Data/RenderParameters.cpp
    Data/ShaderConfigFactory.cpp
    Data/ShaderConfiguration.cpp
    Data/ShaderProgram.cpp
    Data/ShaderProgramManager.cpp
    Data/SimpleMaterial.cpp
    Data/SphereSampler.cpp
    Data/Texture.cpp
    Data/TextureManager.cpp
    Data/VolumeObject.cpp
    Data/VolumetricMaterial.cpp
    Data/stb.cpp
    RadiumEngine.cpp
    Rendering/DebugRender.cpp
    Rendering/ForwardRenderer.cpp
    Rendering/RenderObject.cpp
    Rendering/RenderObjectManager.cpp
    Rendering/RenderTechnique.cpp
    Rendering/Renderer.cpp
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

set(engine_headers
    Data/BlinnPhongMaterial.hpp
    Data/DisplayableObject.hpp
    Data/DrawPrimitives.hpp
    Data/EnvironmentTexture.hpp
    Data/LambertianMaterial.hpp
    Data/Material.hpp
    Data/MaterialConverters.hpp
    Data/Mesh.hpp
    Data/PlainMaterial.hpp
    Data/RawShaderMaterial.hpp
    Data/RenderParameters.hpp
    Data/ShaderConfigFactory.hpp
    Data/ShaderConfiguration.hpp
    Data/ShaderProgram.hpp
    Data/ShaderProgramManager.hpp
    Data/SimpleMaterial.hpp
    Data/SphereSampler.hpp
    Data/Texture.hpp
    Data/TextureManager.hpp
    Data/ViewingParameters.hpp
    Data/VolumeObject.hpp
    Data/VolumetricMaterial.hpp
    FrameInfo.hpp
    OpenGL.hpp
    RaEngine.hpp
    RadiumEngine.hpp
    Rendering/DebugRender.hpp
    Rendering/ForwardRenderer.hpp
    Rendering/RenderObject.hpp
    Rendering/RenderObjectManager.hpp
    Rendering/RenderObjectTypes.hpp
    Rendering/RenderTechnique.hpp
    Rendering/Renderer.hpp
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
    Scene/System.hpp
    Scene/SystemDisplay.hpp
)

set(engine_shaders
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
    Lines/Lines.frag.glsl
    Lines/Lines.geom.glsl
    Lines/Lines.vert.glsl
    Lines/LinesAdjacency.geom.glsl
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
    Materials/Volumetric/Volumetric.vert.glsl
    Materials/Volumetric/VolumetricOIT.frag.glsl
    Picking/Picking.frag.glsl
    Picking/Picking.vert.glsl
    Picking/PickingLines.geom.glsl
    Picking/PickingLinesAdjacency.geom.glsl
    Picking/PickingPoints.geom.glsl
    Picking/PickingTriangles.geom.glsl
    Points/PointCloud.geom.glsl
    Transform/TransformStructs.glsl
)

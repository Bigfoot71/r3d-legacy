# This function takes input the path of a shader, minifies it via
# a python script and copies the output of the minifier to a source file
function(process_shader shader_file var_name)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} "${R3D_ROOT_PATH}/scripts/shader_minifier.py" "${shader_file}"
        OUTPUT_VARIABLE shader_output
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set("${var_name}" "${shader_output}" PARENT_SCOPE)
endfunction()

# Find the Python executable
find_program(PYTHON_EXECUTABLE python3 REQUIRED)

# Shader Process
process_shader("${R3D_ROOT_PATH}/shaders/material.vs" VS_CODE_MATERIAL)
process_shader("${R3D_ROOT_PATH}/shaders/material.fs" FS_CODE_MATERIAL)
process_shader("${R3D_ROOT_PATH}/shaders/depth/depth.vs" VS_CODE_DEPTH)
process_shader("${R3D_ROOT_PATH}/shaders/depth/depth.fs" FS_CODE_DEPTH)
process_shader("${R3D_ROOT_PATH}/shaders/depth/depthCube.vs" VS_CODE_DEPTH_CUBE)
process_shader("${R3D_ROOT_PATH}/shaders/depth/depthCube.fs" FS_CODE_DEPTH_CUBE)
process_shader("${R3D_ROOT_PATH}/shaders/blur.vs" VS_CODE_BLUR)
process_shader("${R3D_ROOT_PATH}/shaders/blur.fs" FS_CODE_BLUR)
process_shader("${R3D_ROOT_PATH}/shaders/postfx.vs" VS_CODE_POSTFX)
process_shader("${R3D_ROOT_PATH}/shaders/postfx.fs" FS_CODE_POSTFX)
process_shader("${R3D_ROOT_PATH}/shaders/skybox.vs" VS_CODE_SKYBOX)
process_shader("${R3D_ROOT_PATH}/shaders/skybox.fs" FS_CODE_SKYBOX)
process_shader("${R3D_ROOT_PATH}/shaders/cubemap/cubemap.vs" VS_CODE_CUBEMAP)
process_shader("${R3D_ROOT_PATH}/shaders/cubemap/prefilter.fs" FS_CODE_PREFILTER)
process_shader("${R3D_ROOT_PATH}/shaders/cubemap/irradianceConvolution.fs" FS_CODE_IRRADIANCE_CONVOLUTION)
process_shader("${R3D_ROOT_PATH}/shaders/cubemap/cubemapFromEquirectangular.fs" FS_CODE_CUBEMAP_FROM_EQUIRECTANGULAR)
process_shader("${R3D_ROOT_PATH}/shaders/debug/debugDepth.vs" VS_CODE_DEBUG_DEPTH)
process_shader("${R3D_ROOT_PATH}/shaders/brdf.vs" VS_CODE_BRDF)
process_shader("${R3D_ROOT_PATH}/shaders/brdf.fs" FS_CODE_BRDF)
process_shader("${R3D_ROOT_PATH}/shaders/debug/debugDepthTexture2D.fs" FS_CODE_DEBUG_DEPTH_TEXTURE_2D)
process_shader("${R3D_ROOT_PATH}/shaders/debug/debugDepthCubemap.fs" FS_CODE_DEBUG_DEPTH_CUBEMAP)

# Set the path of the generated header file
set(R3D_SOURCES_GENERATED "${CMAKE_BINARY_DIR}/generated/src/ShaderCodes.cpp")

# Header file generation with built-in shaders
configure_file(
    ${R3D_ROOT_PATH}/samples/ShaderCodes.cpp.in
    ${R3D_SOURCES_GENERATED}
)

# Define path to 'detail/ShaderCode.hpp' header
add_definitions(-DR3D_SHADER_CODE_HEADER="${R3D_ROOT_PATH}/src/detail/shader_code.hpp")

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
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/material.vs" VS_CODE_MATERIAL)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/material.fs" FS_CODE_MATERIAL)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/depth/depth.vs" VS_CODE_DEPTH)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/depth/depth.fs" FS_CODE_DEPTH)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/depth/depthCube.vs" VS_CODE_DEPTH_CUBE)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/depth/depthCube.fs" FS_CODE_DEPTH_CUBE)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/blur.vs" VS_CODE_BLUR)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/blur.fs" FS_CODE_BLUR)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/postfx.vs" VS_CODE_POSTFX)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/postfx.fs" FS_CODE_POSTFX)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/skybox.vs" VS_CODE_SKYBOX)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/skybox.fs" FS_CODE_SKYBOX)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/cubemap/cubemap.vs" VS_CODE_CUBEMAP)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/cubemap/prefilter.fs" FS_CODE_PREFILTER)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/cubemap/irradianceConvolution.fs" FS_CODE_IRRADIANCE_CONVOLUTION)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/cubemap/cubemapFromEquirectangular.fs" FS_CODE_CUBEMAP_FROM_EQUIRECTANGULAR)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/debug/debugDepth.vs" VS_CODE_DEBUG_DEPTH)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/brdf.vs" VS_CODE_BRDF)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/brdf.fs" FS_CODE_BRDF)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/debug/debugDepthTexture2D.fs" FS_CODE_DEBUG_DEPTH_TEXTURE_2D)
process_shader("${R3D_ROOT_PATH}/shaders/glsl330/debug/debugDepthCubemap.fs" FS_CODE_DEBUG_DEPTH_CUBEMAP)

# Set the path of the generated header file
set(R3D_SOURCES_GENERATED "${CMAKE_BINARY_DIR}/generated/src/shader_code.cpp")

# Header file generation with built-in shaders
configure_file(
    ${R3D_ROOT_PATH}/shaders/shader_code.cpp.in
    ${R3D_SOURCES_GENERATED}
)

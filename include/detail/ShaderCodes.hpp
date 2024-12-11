#ifndef R3D_DETAIL_SHADER_CODES_HPP
#define R3D_DETAIL_SHADER_CODES_HPP

namespace r3d {

extern const char VS_CODE_MATERIAL[];
extern const char FS_CODE_MATERIAL[];

extern const char VS_CODE_DEPTH[];
extern const char FS_CODE_DEPTH[];

extern const char VS_CODE_DEPTH_CUBE[];
extern const char FS_CODE_DEPTH_CUBE[];

extern const char VS_CODE_BLUR[];
extern const char FS_CODE_BLUR[];

extern const char VS_CODE_POSTFX[];
extern const char FS_CODE_POSTFX[];

extern const char VS_CODE_SKYBOX[];
extern const char FS_CODE_SKYBOX[];

extern const char VS_CODE_CUBEMAP[];
extern const char FS_CODE_PREFILTER[];
extern const char FS_CODE_IRRADIANCE_CONVOLUTION[];
extern const char FS_CODE_CUBEMAP_FROM_EQUIRECTANGULAR[];

extern const char VS_CODE_BRDF[];
extern const char FS_CODE_BRDF[];

extern const char VS_CODE_DEBUG_DEPTH[];
extern const char FS_CODE_DEBUG_DEPTH_TEXTURE_2D[];
extern const char FS_CODE_DEBUG_DEPTH_CUBEMAP[];

} // namespace r3d

#endif // R3D_DETAIL_SHADER_CODES_HPP

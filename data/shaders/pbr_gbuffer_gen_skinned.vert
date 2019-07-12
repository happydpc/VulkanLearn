#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec2 inUv;
layout (location = 4) in vec3 inTangent;
layout (location = 5) in vec4 inBoneWeight;
layout (location = 6) in uint inBoneIndices;

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outBitangent;
layout (location = 4) flat out int perMaterialIndex;
layout (location = 5) flat out int perObjectIndex;
layout (location = 6) out vec3 outWorldPos;
layout (location = 7) out vec3 outEyePos;
layout (location = 8) noperspective out vec2 outScreenPos;

#include "uniform_layout.sh"
#include "quaternion.sh"

void main() 
{
	perObjectIndex = objectDataIndex[gl_DrawID].perObjectIndex;

	int perAnimationChunkIndex = objectDataIndex[gl_DrawID].perAnimationIndex;

	vec4 bone_weights = inBoneWeight;

	mat2x4 dq0 = perFrameBoneData[perFrameBoneChunkIndirect[perAnimationChunkIndex + (inBoneIndices >> 0) & 255]].animationDQ;
    mat2x4 dq1 = perFrameBoneData[perFrameBoneChunkIndirect[perAnimationChunkIndex + (inBoneIndices >> 8) & 255]].animationDQ;
    mat2x4 dq2 = perFrameBoneData[perFrameBoneChunkIndirect[perAnimationChunkIndex + (inBoneIndices >> 16) & 255]].animationDQ;
    mat2x4 dq3 = perFrameBoneData[perFrameBoneChunkIndirect[perAnimationChunkIndex + (inBoneIndices >> 24) & 255]].animationDQ;

    // Ensure all bone transforms are in the same neighbourhood
    if (dot(dq0[0], dq1[0]) < 0.0) bone_weights.y *= -1.0;
    if (dot(dq0[0], dq2[0]) < 0.0) bone_weights.z *= -1.0;
    if (dot(dq0[0], dq3[0]) < 0.0) bone_weights.w *= -1.0;

    // Blend
    mat2x4 result =
        bone_weights.x * dq0 +
        bone_weights.y * dq1 +
        bone_weights.z * dq2 +
        bone_weights.w * dq3;

	// NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Since minus dual quaternion equals original one:
	// DQ = p + sigma * q, p = r, q = 0.5 * t * r
	// r = cos(theta) + nsin(theta)
	// r = -r
	// =========>-r + sigma * 0.5 * t * (-r) = -r - sigma * 0.5 * t * r = -p - sigma * q = -DQ
	// This proves minus DQ equals transform of original DQ
	// However, linear combination will be changed, e.g:
	// DQ0 * 0.3 + DQ1 * 0.7 ===> DQ0 * 0.3 + DQ1 * (-0.7)
	// Though DQ1 and -DQ1 represent same transform, it still breaks linear combination that the result is no longer a normalized dual quaternion
	// SO, WE HAVE TO RE NORMALIZE INTERPOLATED RESULT, WE HAVE TO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	float len = length(result[0]);
	result /= len;

	vec3 animated_pos = DualQuaternionTransformPoint(result, inPos);

	gl_Position = perObjectData[perObjectIndex].MVPN * vec4(animated_pos.xyz, 1.0);

	outNormal = normalize(vec3(perObjectData[perObjectIndex].model * vec4(inNormal, 0.0)));
	outWorldPos = (perObjectData[perObjectIndex].model * vec4(animated_pos.xyz, 1.0)).xyz;
	outEyePos = (perFrameData.view * vec4(outWorldPos, 1.0)).xyz;
	outScreenPos = gl_Position.xy / gl_Position.w;

	outUv = inUv;
	outUv.t = 1.0 - inUv.t;

	outTangent = normalize(vec3(perObjectData[perObjectIndex].model * vec4(inTangent, 0.0)));
	outBitangent = normalize(cross(outNormal, outTangent));

	perMaterialIndex = objectDataIndex[gl_DrawID].perMaterialIndex;
}

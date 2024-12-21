#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);

PostProcessVertexToPixel main(const PostProcessVertexInput aInput)
{
    PostProcessVertexToPixel output;

    output.position = aInput.position;

    output.uv = aInput.uv;

    return output;
}
/////////////////////////////////////////////////////////////////////////
// Pixel shader for Shadow
////////////////////////////////////////////////////////////////////////
#version 330
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     roomId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;
uniform int objectId;

in vec4 position;

void main()
{
	gl_FragData[0] =vec4(position.x,position.y,position.z,position.w);
}

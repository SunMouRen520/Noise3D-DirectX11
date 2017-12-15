/******************************************

	Module: Draw Solid 3D(header)
	Author: Jige
	
	draw solid 3D mesh 
	(pure color, simplified vertex)

******************************************/


technique11 DrawSolid3D
{
	pass EmptyTextureSky
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Solid3D()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Solid3D()));
	}
}


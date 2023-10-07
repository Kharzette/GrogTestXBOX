#pragma once
#include	<XTL.h>

typedef struct	SolarMat_t			SolarMat;
typedef struct	GraphicsDevice_t	GraphicsDevice;


SolarMat	*SolarMat_Init(GraphicsDevice *pGD, float aspect);

void	SolarMat_SetShaderVars(const SolarMat *pSM, GraphicsDevice *pGD);

const D3DXMATRIX	*SolarMat_GetProj(const SolarMat *pSM);
const D3DXVECTOR4	*SolarMat_GetLightDir(const SolarMat *pSM);
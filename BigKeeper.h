#pragma once
#include	<XTL.h>

typedef struct	BigKeeper_t			BigKeeper;
typedef struct	GraphicsDevice_t	GraphicsDevice;


BigKeeper	*BK_Init(GraphicsDevice *pGD);

void	BK_Draw(const BigKeeper *pBK, GraphicsDevice *pGD,
				int secX, int secY, int secZ,
				const D3DXVECTOR3 *pPlayerPos,
				const D3DXVECTOR4 *pLightDir,
				const D3DXQUATERNION *pView,
				const D3DXMATRIX *pProj);

D3DXVECTOR3	BK_GetSectorDistanceVec(const BigKeeper *pBK, int secX, int secY, int secZ);
D3DXVECTOR3	BK_GetSectorDistanceVec2(const BigKeeper *pBK, int secX, int secY, int secZ,
									const D3DXVECTOR3 *pPlayerPos);

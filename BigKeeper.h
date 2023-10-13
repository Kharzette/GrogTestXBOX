#pragma once
#include	<XTL.h>

typedef struct	BigKeeper_t			BigKeeper;
typedef struct	GraphicsDevice_t	GraphicsDevice;
typedef struct	WayPoints_t			WayPoints;


BigKeeper	*BK_Init(GraphicsDevice *pGD);

void	BK_Draw(const BigKeeper *pBK, GraphicsDevice *pGD,
				const Vec3Int32 *pSec,
				const D3DXVECTOR3 *pPlayerPos,
				const D3DXVECTOR4 *pLightDir,
				const D3DXQUATERNION *pView,
				const D3DXMATRIX *pProj);
void	BK_SetWayPoints(const BigKeeper *pBK, WayPoints *pWP);				

D3DXVECTOR3	BK_GetSectorDistanceVec(const BigKeeper *pBK, const Vec3Int32 *pSec);
D3DXVECTOR3	BK_GetSectorDistanceVec2(const BigKeeper *pBK, const Vec3Int32 *pSec,
									const D3DXVECTOR3 *pPlayerPos);
void	BK_MakeSectorDistStr(char *szDist, INT64 dist);
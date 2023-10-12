#pragma once
#include	<XTL.h>


typedef struct	Vec3Int32_t
{
	int	x, y, z;
}	Vec3Int32;


BOOL	Vec3Int32_Equals(const Vec3Int32 *pA, const Vec3Int32 *pB);
void	Vec3Int32_Subtract(Vec3Int32 *pOut, const Vec3Int32 *pA, const Vec3Int32 *pB);
void	Vec3Int32_Convert(D3DXVECTOR3 *pOut, const Vec3Int32 *pVec);
INT64	Vec3Int32_Length(const Vec3Int32 *pVec);
#include	"Vec3Int32.h"


BOOL	Vec3Int32_Equals(const Vec3Int32 *pA, const Vec3Int32 *pB)
{
	return	(pA->x == pB->x && pA->y == pB->y && pA->z == pB->z);
}

void	Vec3Int32_Subtract(Vec3Int32 *pOut, const Vec3Int32 *pA, const Vec3Int32 *pB)
{
	pOut->x	=pA->x - pB->x;
	pOut->y	=pA->y - pB->y;
	pOut->z	=pA->z - pB->z;
}

void	Vec3Int32_Convert(D3DXVECTOR3 *pOut, const Vec3Int32 *pVec)
{
	pOut->x	=(float)pVec->x;
	pOut->y	=(float)pVec->y;
	pOut->z	=(float)pVec->z;
}

INT64	Vec3Int32_Length(const Vec3Int32 *pVec)
{
	INT64	vx, vy, vz, lenSq;

	vx	=pVec->x;
	vy	=pVec->y;
	vz	=pVec->z;

	//make sure this accumulation doesn't overflow
	lenSq	=((vx * vx) + (vy * vy) + (vz * vz));

	return	sqrt(lenSq);
}
#include	"Vec3Int32.h"
#include	"Ship.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"

#define	MIN_ARRAY_SIZE		8
#define	MAX_ARRAY_SIZE		256	//for initial array, can double later
#define	WAYPOINT_NAME_SIZE	16	//fixed sized names


typedef struct	WayPoints_t
{
	Vec3Int32	*mpSector;		//sector coordinates
	D3DXVECTOR3	*mpInSector;	//within sector coordinates
	char		*mpNames;		//fixed size
	BOOL		*mpDisposed;	//slot can be re-used

	int	mNumWPs;	//size of the array, might be some inactive

}	WayPoints;


int	WayPoints_ValidIndex(const WayPoints *pWP, int idx)
{
	int	ret, i;
	if(idx > 0 && idx < pWP->mNumWPs)
	{
		if(!pWP->mpDisposed[idx])
		{
			return	idx;
		}
	}

	if(idx < 0)
	{
		//wrap to highest
		idx	=pWP->mNumWPs - 1;

		for(i=idx;i >= 0;i--)
		{
			if(pWP->mpDisposed[i])
			{
				continue;
			}
			return	i;
		}
	}

	ret	=ClampInt(idx, 0, pWP->mNumWPs - 1);

	for(i=ret;i < pWP->mNumWPs;i++)
	{
		if(pWP->mpDisposed[i])
		{
			continue;
		}
		return	i;
	}

	//start over at zero
	for(i=0;i < pWP->mNumWPs;i++)
	{
		if(pWP->mpDisposed[i])
		{
			continue;
		}
		return	i;
	}
	return	-1;
}

const char	*WayPoints_GetName(const WayPoints *pWP, int idx)
{
	return	pWP->mpNames + WAYPOINT_NAME_SIZE * idx;
}

WayPoints	*WayPoints_Init(int arraySize)
{
	int			i, saneSize	=ClampInt(arraySize, MIN_ARRAY_SIZE, MAX_ARRAY_SIZE);
	WayPoints	*pRet		=malloc(sizeof(WayPoints));

	memset(pRet, 0, sizeof(WayPoints));

	pRet->mpSector		=malloc(sizeof(Vec3Int32) * saneSize);
	pRet->mpInSector	=malloc(sizeof(D3DXVECTOR3) * saneSize);
	pRet->mpNames		=malloc(WAYPOINT_NAME_SIZE * saneSize);
	pRet->mpDisposed	=malloc(sizeof(BOOL) * saneSize);

	//set all slots to disposed
	for(i=0;i < saneSize;i++)
	{
		pRet->mpDisposed[i]	=TRUE;
	}

	pRet->mNumWPs	=saneSize;

	return	pRet;
}

void	WayPoints_Add(WayPoints *pWP, const char *pName,
						Vec3Int32 secPos, D3DXVECTOR3 inSectorPos)
{
	//check for free slots
	int	newSize, i, freeIDX	=-1;

	for(i=0;i < pWP->mNumWPs;i++)
	{
		if(pWP->mpDisposed[i])
		{
			freeIDX	=i;
			break;
		}
	}

	if(freeIDX == -1)
	{
		Vec3Int32	*pNewSecs;
		D3DXVECTOR3	*pNewInSecs;
		char		*pNewNames;
		BOOL		*pNewDisposed;

		//double it
		newSize	=pWP->mNumWPs * 2;

		//grow the arrays
		pNewSecs		=malloc(sizeof(Vec3Int32) * newSize);
		pNewInSecs		=malloc(sizeof(D3DXVECTOR3) * newSize);
		pNewNames		=malloc(WAYPOINT_NAME_SIZE * newSize);
		pNewDisposed	=malloc(sizeof(BOOL) * newSize);

		//copy in existing data
		memcpy(pNewSecs, pWP->mpSector, sizeof(Vec3Int32) * pWP->mNumWPs);
		memcpy(pNewInSecs, pWP->mpInSector, sizeof(D3DXVECTOR3) * pWP->mNumWPs);
		memcpy(pNewNames, pWP->mpNames, WAYPOINT_NAME_SIZE * pWP->mNumWPs);
		memcpy(pNewDisposed, pWP->mpDisposed, sizeof(BOOL) * pWP->mNumWPs);

		//free old data
		free(pWP->mpSector);
		free(pWP->mpInSector);
		free(pWP->mpNames);
		free(pWP->mpDisposed);

		//replace old pointers
		pWP->mpSector	=pNewSecs;
		pWP->mpInSector	=pNewInSecs;
		pWP->mpNames	=pNewNames;
		pWP->mpDisposed	=pNewDisposed;

		freeIDX	=pWP->mNumWPs;

		pWP->mNumWPs	=newSize;
	}

	pWP->mpSector[freeIDX]		=secPos;
	pWP->mpInSector[freeIDX]	=inSectorPos;
	pWP->mpDisposed[freeIDX]	=FALSE;

	//leave space for a null
	strncpy(pWP->mpNames + WAYPOINT_NAME_SIZE * freeIDX,
			pName, WAYPOINT_NAME_SIZE - 1);
}


void	WayPoints_DisposeByIndex(WayPoints *pWP, int idx)
{
	if(idx < 0 || idx > pWP->mNumWPs)
	{
		//TODO: warn
		return;
	}

	pWP->mpDisposed[idx]	=TRUE;
}


void	WayPoints_ComputeHeadingToIndex(const WayPoints *pWP, const Ship *pShip,
	int idx, int *pHeading, int *pNadir, INT64 *pDist)
{
	D3DXVECTOR3	dirVec;

	const Vec3Int32	*pShipSec;

	if(idx < 0 || idx > pWP->mNumWPs)
	{
		//TODO: warn
		return;
	}

	//see if we are within the same sector
	pShipSec	=Ship_GetSector(pShip);

	if(Vec3Int32_Equals(pShipSec, &pWP->mpSector[idx]))
	{
		D3DXVec3Subtract(&dirVec, &pWP->mpInSector[idx], Ship_GetPosition(pShip));

		*pDist	=D3DXVec3Length(&dirVec);
	}
	else
	{
		Vec3Int32	dirInt;
		Vec3Int32_Subtract(&dirInt, &pWP->mpSector[idx], pShipSec);

		*pDist	=Vec3Int32_Length(&dirInt);

		Vec3Int32_Convert(&dirVec, &dirInt);
	}

	D3DXVec3Normalize(&dirVec, &dirVec);

	{
		float		dot, dot2, heading;
		D3DXVECTOR3	zax	={	0.0f, 0.0f, 1.0f	};
		D3DXVECTOR3	xax	={	1.0f, 0.0f, 0.0f	};
		D3DXVECTOR3	yax	={	0.0f, 1.0f, 0.0f	};

		//dots... more dots
		dot		=D3DXVec3Dot(&dirVec, &zax);
		dot2	=D3DXVec3Dot(&xax, &dirVec);

		//clamp to valid acos values
		//can get a nan if it is outside a little
		dot	=Clamp(dot, -1.0f, 1.0f);

		heading	=(float)acos((float)dot);

		//this determines the quadrant
		if(dot2 < 0.0f)
		{
			heading	=(D3DX_PI * 2.0f) - heading;
		}

		*pHeading	=(int)D3DXToDegree(heading);

		//nadir
		dot		=D3DXVec3Dot(&dirVec, &yax);

		*pNadir	=(int)D3DXToDegree(dot * (D3DX_PI * 0.5f));
	}
}
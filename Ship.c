#include	<XTL.h>
#include	"PilotUI.h"
#include	"GrogLibsXBOX/MeshLib/Mesh.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/UtilityLib/Physics.h"

#define	SECTOR_BOUNDARY	32768.0f
#define	SOLAR_WIND_DRAG	0.00001f
#define	INERTIA_TENSOR	69	//this doesn't do anything yet

typedef struct	Ship_t
{
	//appearance stuff
	Mesh		*mpMesh;
	D3DXVECTOR4	mMatColour;

	Physics		*mpPhysics;

	//big position, little stored in physics
	int	mSectorX, mSectorY, mSectorZ;

	//orientation
	D3DXQUATERNION	mRot;

	//movement
	D3DXVECTOR3	mLastV;			//track velocity over update time for accel calc
	float		mUIAccel;		//smoothed accel for the UI
	float		mTotalDT;		//time smoothed for UI updates
	int			mNumUpdates;	//num physics update per render

	//statistics
	INT64	mFuel, mFuelMax;	//in grams
	INT64	mCargo, mCargoMax;	//in grams
	INT64	mMaxThrust;			//max output from main engines
	INT64	mMass;				//track here to update phys + weight of fuel and cargo etc
	int		mHull, mHullMax;
	int		mO2, mO2Max;	
	float	mHeat;
	float	mRadiatorsExtendPercent;
}	Ship;


Ship	*Ship_Init(Mesh *pMesh, INT64 maxThrust, INT64 fuelMax, int o2Max,
					INT64 cargoMax, int hullMax, INT64 mass)
{
	Ship	*pRet	=malloc(sizeof(Ship));

	memset(pRet, 0, sizeof(Ship));

	pRet->mpPhysics	=Physics_Init();

	//default white
	pRet->mMatColour.x	=pRet->mMatColour.y
		=pRet->mMatColour.z	=pRet->mMatColour.w	=1.0f;

	//temp put near a world
	pRet->mSectorZ	=99500;

	pRet->mpMesh	=pMesh;

	pRet->mFuel		=pRet->mFuelMax		=fuelMax;
	pRet->mO2		=pRet->mO2Max		=o2Max;
	pRet->mHull		=pRet->mHullMax		=hullMax;
	pRet->mMass		=mass;
	pRet->mHeat		=300;	//default 80ish deg F
	pRet->mCargoMax	=cargoMax;

	pRet->mMaxThrust	=maxThrust;

	D3DXQuaternionIdentity(&pRet->mRot);

	Physics_SetProps(pRet->mpPhysics,
		mass / 1000000.0f,	//mass in fractional tons?
		INERTIA_TENSOR, SOLAR_WIND_DRAG);

	return	pRet;
}


//degrees
static int	Ship_GetHeading(const Ship *pShip)
{
	float		dot, dot2, heading;
	D3DXVECTOR3	forward, zax	={	0.0f, 0.0f, 1.0f	};
	D3DXVECTOR3	xax				={	1.0f, 0.0f, 0.0f	};

	//rotate basis vectors into quat space
	RotateVec(&pShip->mRot, &zax, &forward);

	//dots... more dots
	dot		=D3DXVec3Dot(&forward, &zax);
	dot2	=D3DXVec3Dot(&xax, &forward);

	//clamp to valid acos values
	//can get a nan if it is outside a little
	dot	=Clamp(dot, -1.0f, 1.0f);

	heading	=(float)acos((float)dot);

	//this determines the quadrant
	if(dot2 < 0.0f)
	{
		heading	=(D3DX_PI * 2.0f) - heading;
	}
	return	(int)D3DXToDegree(heading);
}

static int	Ship_GetNadir(const Ship *pShip)
{
	float		dot;
	D3DXVECTOR3	forward, zax	={	0.0f, 0.0f, 1.0f	};
	D3DXVECTOR3	yax				={	0.0f, 1.0f, 0.0f	};

	//rotate basis vectors into quat space
	RotateVec(&pShip->mRot, &zax, &forward);

	dot		=D3DXVec3Dot(&forward, &yax);

	return	(int)D3DXToDegree(dot * (D3DX_PI * 0.5f));
}

static int	Ship_GetVelocityHeading(const Ship *pShip)
{
	float		dot, dot2, heading;
	D3DXVECTOR3	vel, zax	={	0.0f, 0.0f, 1.0f	};
	D3DXVECTOR3	xax			={	1.0f, 0.0f, 0.0f	};

	float	len	=D3DXVec3Length(&pShip->mLastV);
	if(len <= 0.0f)
	{
		return	0;
	}

	//normalize
	D3DXVec3Scale(&vel, &pShip->mLastV, 1.0f / len);

	//dots... more dots
	dot		=D3DXVec3Dot(&vel, &zax);
	dot2	=D3DXVec3Dot(&xax, &vel);

	//clamp to valid acos values
	//can get a nan if it is outside a little
	dot	=Clamp(dot, -1.0f, 1.0f);
	
	heading	=(float)acos((float)dot);

	//this determines the quadrant
	if(dot2 < 0.0f)
	{
		heading	=(D3DX_PI * 2.0f) - heading;
	}
	return	(int)D3DXToDegree(heading);
}

static int	Ship_GetVelocityNadir(const Ship *pShip)
{
	float		dot;
	D3DXVECTOR3	vel, yax	={	0.0f, 1.0f, 0.0f	};

	float	len	=D3DXVec3Length(&pShip->mLastV);
	if(len <= 0.0f)
	{
		return	0;
	}

	//normalize
	D3DXVec3Scale(&vel, &pShip->mLastV, 1.0f / len);

	dot		=D3DXVec3Dot(&vel, &yax);

	return	(int)D3DXToDegree(dot * (D3DX_PI * 0.5f));
}

const D3DXQUATERNION	*Ship_GetRotation(const Ship *pShip)
{
	return	&pShip->mRot;
}

void	Ship_GetPosition(const Ship *pShip, const D3DXVECTOR3 *pPos)
{
	Physics_GetPosition(pShip->mpPhysics, pPos);
}

void	Ship_GetSector(const Ship *pShip, int *pX, int *pY, int *pZ)
{
	*pX	=pShip->mSectorX;
	*pY	=pShip->mSectorY;
	*pZ	=pShip->mSectorZ;
}


//deltaTime in seconds
void	Ship_Update(Ship *pShip, float dt)
{
	D3DXVECTOR3	deltaPos;
	BOOL		bAdjusted	=FALSE;


	Physics_Update(pShip->mpPhysics, dt);

	//check for sector changes
	Physics_GetPosition(pShip->mpPhysics, &deltaPos);

	while(deltaPos.x > SECTOR_BOUNDARY)
	{
		pShip->mSectorX++;
		deltaPos.x	-=SECTOR_BOUNDARY;
		bAdjusted	=TRUE;
	}
	while(deltaPos.x < -SECTOR_BOUNDARY)
	{
		pShip->mSectorX--;
		deltaPos.x	+=SECTOR_BOUNDARY;
		bAdjusted	=TRUE;
	}

	while(deltaPos.y > SECTOR_BOUNDARY)
	{
		pShip->mSectorY++;
		deltaPos.y	-=SECTOR_BOUNDARY;
		bAdjusted	=TRUE;
	}
	while(deltaPos.y < -SECTOR_BOUNDARY)
	{
		pShip->mSectorY--;
		deltaPos.y	+=SECTOR_BOUNDARY;
		bAdjusted	=TRUE;
	}

	while(deltaPos.z > SECTOR_BOUNDARY)
	{
		pShip->mSectorZ++;
		deltaPos.z	-=SECTOR_BOUNDARY;
		bAdjusted	=TRUE;
	}
	while(deltaPos.z < -SECTOR_BOUNDARY)
	{
		pShip->mSectorZ--;
		deltaPos.z	+=SECTOR_BOUNDARY;
		bAdjusted	=TRUE;
	}

	if(bAdjusted)
	{
		Physics_SetPosition(pShip->mpPhysics, &deltaPos);
	}

	pShip->mTotalDT	+=dt;
}


void	Ship_UpdateUI(Ship *pShip, UI *pUI, GraphicsDevice *pGD)
{
	D3DXVECTOR3		v, deltaV;

	int	heading		=Ship_GetHeading(pShip);
	int	nadir		=Ship_GetNadir(pShip);
	int	velHeading	=Ship_GetVelocityHeading(pShip);
	int	velNadir	=Ship_GetVelocityNadir(pShip);

	//opposite direction for braking
	int	brkHeading	=(velHeading + 180) % 360;
	int	brkNadir	=-velNadir;

	Physics_GetVelocity(pShip->mpPhysics, &v);

	//smooth accumulated accel
	if(pShip->mTotalDT > 0.0f)
	{
		D3DXVec3Subtract(&deltaV, &v, &pShip->mLastV);

		pShip->mUIAccel	=D3DXVec3Length(&deltaV);

		pShip->mUIAccel	/=pShip->mTotalDT;

		pShip->mUIAccel	/=9.8f;	//convert to G
	}

	PUI_UpdateValues(pUI, pGD,
		D3DXVec3Length(&v),
		pShip->mUIAccel, pShip->mFuel,
		pShip->mO2, pShip->mCargo, pShip->mCargoMax, pShip->mHull,
		pShip->mHullMax, 1, 1,
		heading, nadir, 0, 0,
		pShip->mHeat, pShip->mRadiatorsExtendPercent,
		velHeading, velNadir, brkHeading, brkNadir);

	pShip->mNumUpdates	=0;
	pShip->mTotalDT		=0.0f;
	pShip->mUIAccel		=0.0f;
	pShip->mLastV		=v;
}


void	Ship_Turn(Ship *pShip, float deltaPitch, float deltaYaw, float deltaRoll)
{	
	D3DXQUATERNION	accum, rotX, rotY;//, rotZ;
	D3DXVECTOR3		up		={	0.0f, 1.0f, 0.0f	};
	D3DXVECTOR3		side	={	1.0f, 0.0f, 0.0f	};

	if(deltaPitch == 0.0f && deltaYaw == 0.0f && deltaRoll == 0.0f)
	{
		return;
	}

	//rotate basis vectors into quat space
	RotateVec(&pShip->mRot, &up, &up);
	RotateVec(&pShip->mRot, &side, &side);

	D3DXQuaternionRotationAxis(&rotX, &side, deltaPitch);
	D3DXQuaternionRotationAxis(&rotY, &up, -deltaYaw);	//NOTE THE -!

	D3DXQuaternionMultiply(&accum, &rotX, &rotY);
	D3DXQuaternionMultiply(&pShip->mRot, &pShip->mRot, &accum);

	D3DXQuaternionNormalize(&pShip->mRot, &pShip->mRot);
}


void	Ship_Throttle(Ship *pShip, BYTE throttle)
{
	D3DXVECTOR3	force, zax	={	0.0f, 0.0f, 1.0f	};

	if(!throttle)
	{
		return;
	}

	//rotate basis vector into quat space
	RotateVec(&pShip->mRot, &zax, &force);

	D3DXVec3Scale(&force, &force,
		(throttle / 255.0f) * pShip->mMaxThrust);

	Physics_ApplyForce(pShip->mpPhysics, &force);
}


void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD,
			const D3DXVECTOR3 *pEyePos,
			const D3DXMATRIX *pView,
			const D3DXMATRIX *pProj)
{
	D3DXMATRIX	wvp, wtrans, w;
	D3DXVECTOR3	pos;

	//get position within sector
	Physics_GetPosition(pShip->mpPhysics, &pos);

	//borrow the transpose temp mat for translation
	D3DXMatrixTranslation(&wtrans, pos.x, pos.y, pos.z);

	D3DXMatrixRotationQuaternion(&w, &pShip->mRot);

	//rotate * translate
	D3DXMatrixMultiply(&wvp, &w, &wtrans);

	//world * view
	D3DXMatrixMultiply(&wvp, &wvp, pView);

	//wv * proj
	D3DXMatrixMultiply(&wvp, &wvp, pProj);

	D3DXMatrixTranspose(&wtrans, &w);
	D3DXMatrixTranspose(&wvp, &wvp);

	GD_SetVShaderConstant(pGD, 0, &wvp, 4);
	GD_SetVShaderConstant(pGD, 8, &wtrans, 4);				//ship world matrix
	GD_SetVShaderConstant(pGD, 12, pEyePos, 1);				//eye position
	GD_SetVShaderConstant(pGD, 17, &pShip->mMatColour, 1);	//mat color

	Mesh_Draw(pShip->mpMesh, pGD);
}
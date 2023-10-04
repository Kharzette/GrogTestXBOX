#include	<XTL.h>
#include	"PilotUI.h"
#include	"GrogLibsXBOX/MeshLib/Mesh.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/UtilityLib/Physics.h"

#define	SOLAR_WIND_DRAG	0.00001f

typedef struct	Ship_t
{
	Mesh		*mpMesh;
	Physics		*mpPhysics;

	//big position
	int	mSectorX, mSectorY, mSectorZ;

	//orientation
	D3DXQUATERNION	mRot;

	//movement
	D3DXVECTOR3	mLastV;			//track velocity over update time for accel calc
	float		mUIAccel;		//smoothed accel for the UI
	float		mTotalDT;		//time smoothed
	int			mNumUpdates;	//num physics update per render

	int	mFuel, mFuelMax;
	int	mO2, mO2Max;
	int	mCargo, mCargoMax;
	int	mHull, mHullMax;
	int	mMaxThrust;			//max output from main engines
	int	mMass;				//track here to update phys + weight of fuel and cargo etc
	
	float	mHeat;
	float	mRadiatorsExtendPercent;
}	Ship;


Ship	*Ship_Init(Mesh *pMesh, int maxThrust, int fuelMax, int o2Max,
					int cargoMax, int hullMax, int mass, int it)
{
	Ship	*pRet	=malloc(sizeof(Ship));

	memset(pRet, 0, sizeof(Ship));

	pRet->mpPhysics	=Physics_Init();

	pRet->mpMesh	=pMesh;
	pRet->mFuel		=pRet->mFuelMax		=fuelMax;
	pRet->mO2		=pRet->mO2Max		=o2Max;
	pRet->mHull		=pRet->mHullMax		=hullMax;
	pRet->mMass		=mass;
	pRet->mHeat		=300;	//default 80ish deg F
	pRet->mCargoMax	=cargoMax;

	pRet->mMaxThrust	=maxThrust;

	D3DXQuaternionIdentity(&pRet->mRot);

	Physics_SetProps(pRet->mpPhysics, mass, it, SOLAR_WIND_DRAG);

	return	pRet;
}


float	Ship_GetHeading(const Ship *pShip)
{
	float		dot, dot2, heading;
	D3DXVECTOR3	forward, zax	={	0.0f, 0.0f, 1.0f	};
	D3DXVECTOR3	xax				={	1.0f, 0.0f, 0.0f	};

	//rotate basis vectors into quat space
	RotateVec(&pShip->mRot, &zax, &forward);

	//dots... more dots
	dot		=D3DXVec3Dot(&forward, &zax);
	dot2	=D3DXVec3Dot(&xax, &forward);

	heading	=acos(dot);

	//this determines the quadrant
	if(dot2 < 0.0f)
	{
		heading	=(D3DX_PI * 2.0f) - heading;
	}
	return	heading;
}

float	Ship_GetNadir(const Ship *pShip)
{
	float		dot;
	D3DXVECTOR3	forward, zax	={	0.0f, 0.0f, 1.0f	};
	D3DXVECTOR3	yax				={	0.0f, 1.0f, 0.0f	};

	//rotate basis vectors into quat space
	RotateVec(&pShip->mRot, &zax, &forward);

	dot		=D3DXVec3Dot(&forward, &yax);

	return	dot * (D3DX_PI * 0.5f);
}

const D3DXQUATERNION	*Ship_GetRotation(const Ship *pShip)
{
	return	&pShip->mRot;
}


//deltaTime in seconds
void	Ship_Update(Ship *pShip, float dt)
{
	D3DXVECTOR3		side	={	1.0f, 0.0f, 0.0f	};
	D3DXVECTOR3		up		={	0.0f, 1.0f, 0.0f	};
	D3DXVECTOR3		forward	={	0.0f, 0.0f, 1.0f	};

	Physics_Update(pShip->mpPhysics, dt);

	pShip->mTotalDT	+=dt;
}


void	Ship_UpdateUI(Ship *pShip, UI *pUI, GraphicsDevice *pGD)
{
	D3DXVECTOR3		v, deltaV;

	float	heading	=Ship_GetHeading(pShip);
	float	nadir	=Ship_GetNadir(pShip);

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
		(int)D3DXToDegree(heading),
		(int)D3DXToDegree(nadir), 0, 0,
		pShip->mHeat, pShip->mRadiatorsExtendPercent);

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

	//rotate basis vectors into quat space
	RotateVec(&pShip->mRot, &up, &up);
	RotateVec(&pShip->mRot, &side, &side);

	D3DXQuaternionRotationAxis(&rotX, &side, deltaPitch);
	D3DXQuaternionRotationAxis(&rotY, &up, -deltaYaw);		//NOTE NEGATION!

	D3DXQuaternionMultiply(&accum, &rotX, &rotY);
	D3DXQuaternionMultiply(&pShip->mRot, &pShip->mRot, &accum);

	D3DXQuaternionNormalize(&pShip->mRot, &pShip->mRot);
}


void	Ship_Throttle(Ship *pShip, BYTE throttle)
{
	D3DXVECTOR3	force	={	pShip->mRot.x, pShip->mRot.y, pShip->mRot.z	};

	if(!throttle)
	{
		return;
	}

	D3DXVec3Scale(&force, &force,
		(throttle / 255.0f) * pShip->mMaxThrust);

	Physics_ApplyForce(pShip->mpPhysics, &force);
}


void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD,
			D3DXMATRIX *pView, D3DXMATRIX *pProj)
{
	D3DXMATRIX	wvp, wtrans, w;

	D3DXMatrixRotationQuaternion(&w, &pShip->mRot);

	D3DXMatrixMultiply(&wvp, &w, pView);
	D3DXMatrixMultiply(&wvp, &wvp, pProj);

	D3DXMatrixTranspose(&wtrans, &w);
	D3DXMatrixTranspose(&wvp, &wvp);

	GD_SetVShaderConstant(pGD, 0, &wvp, 4);
	GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//ship world matrix

	Mesh_Draw(pShip->mpMesh, pGD);
}
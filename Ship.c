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
	D3DXMATRIX	mMat;
	D3DXVECTOR3	mForward, mUp;
	D3DXVECTOR3	mAttitude;	//lazy gamey turning

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

	pRet->mUp.y	=pRet->mForward.z	=1.0f;

	pRet->mpMesh	=pMesh;
	pRet->mFuel		=pRet->mFuelMax		=fuelMax;
	pRet->mO2		=pRet->mO2Max		=o2Max;
	pRet->mHull		=pRet->mHullMax		=hullMax;
	pRet->mMass		=mass;
	pRet->mHeat		=300;	//default 80ish deg F
	pRet->mCargoMax	=cargoMax;

	pRet->mMaxThrust	=maxThrust;

	Physics_SetProps(pRet->mpPhysics, mass, it, SOLAR_WIND_DRAG);

	return	pRet;
}


const D3DXMATRIX	*Ship_GetWorldMatrix(Ship *pShip)
{
	return	&pShip->mMat;
}


//deltaTime in seconds
void	Ship_Update(Ship *pShip, float dt)
{
	D3DXVECTOR3		side	={	1.0f, 0.0f, 0.0f	};
	D3DXVECTOR3		up		={	0.0f, 1.0f, 0.0f	};
	D3DXVECTOR3		forward	={	0.0f, 0.0f, 1.0f	};

	Physics_Update(pShip->mpPhysics, dt);

	D3DXVec3TransformNormal(&pShip->mForward, &forward, &pShip->mMat);
	D3DXVec3TransformNormal(&pShip->mUp, &up, &pShip->mMat);

	pShip->mTotalDT	+=dt;
}


void	Ship_UpdateUI(Ship *pShip, UI *pUI, GraphicsDevice *pGD)
{
	D3DXVECTOR3		v, deltaV;

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
		(int)D3DXToDegree(pShip->mAttitude.y),
		(int)D3DXToDegree(pShip->mAttitude.x), 0, 0,
		pShip->mHeat, pShip->mRadiatorsExtendPercent);

	pShip->mNumUpdates	=0;
	pShip->mTotalDT		=0.0f;
	pShip->mUIAccel		=0.0f;
	pShip->mLastV		=v;
}


void	Ship_Turn(Ship *pShip, float deltaPitch, float deltaYaw, float deltaRoll)
{
    pShip->mAttitude.x    +=deltaPitch;
    pShip->mAttitude.y    +=deltaYaw;
    pShip->mAttitude.z    +=deltaRoll;

    pShip->mAttitude.x    =WrapAngleRadians(pShip->mAttitude.x);
    pShip->mAttitude.y    =WrapAngleRadians(pShip->mAttitude.y);
    pShip->mAttitude.z    =WrapAngleRadians(pShip->mAttitude.z);

    D3DXMatrixRotationYawPitchRoll(&pShip->mMat, pShip->mAttitude.y, pShip->mAttitude.x, pShip->mAttitude.z);
}


void	Ship_Throttle(Ship *pShip, BYTE throttle)
{
	D3DXVECTOR3	force;

	if(!throttle)
	{
		return;
	}

	D3DXVec3Scale(&force, &pShip->mForward,
		(throttle / 255.0f) * pShip->mMaxThrust);

	Physics_ApplyForce(pShip->mpPhysics, &force);
}


void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD,
			D3DXMATRIX *pView, D3DXMATRIX *pProj)
{
	D3DXMATRIX	wvp, wtrans;

	D3DXMatrixMultiply(&wvp, &pShip->mMat, pView);
	D3DXMatrixMultiply(&wvp, &wvp, pProj);

	D3DXMatrixTranspose(&wtrans, &pShip->mMat);
	D3DXMatrixTranspose(&wvp, &wvp);

	GD_SetVShaderConstant(pGD, 0, &wvp, 4);
	GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//ship world matrix

	Mesh_Draw(pShip->mpMesh, pGD);
}
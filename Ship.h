#pragma once
#include	<XTL.h>

typedef struct	Ship_t				Ship;
typedef struct	Mesh_t				Mesh;
typedef struct	UI_t				UI;
typedef struct	GraphicsDevice_t	GraphicsDevice;

Ship	*Ship_Init(Mesh *pMesh, INT64 maxThrust,
		INT64 fuelMax, int o2Max, INT64 cargoMax, int hullMax, INT64 mass);
void	Ship_Update(Ship *pShip, float dt);
void	Ship_UpdateUI(Ship *pShip, UI *pUI, GraphicsDevice *pGD);

void	Ship_Turn(Ship *pShip, float deltaPitch, float deltaYaw, float deltaRoll);
void	Ship_Throttle(Ship *pShip, BYTE throttle);

void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD,
			const D3DXVECTOR3 *pEyePos,
			const D3DXMATRIX *pView,
			const D3DXMATRIX *pProj);

void	Ship_GetPosition(const Ship *pShip, const D3DXVECTOR3 *pPos);
void	Ship_GetSector(const Ship *pShip, int *pX, int *pY, int *pZ);
float	Ship_GetHeading(const Ship *pShip);
float	Ship_GetNadir(const Ship *pShip);

const D3DXQUATERNION	*Ship_GetRotation(const Ship *pShip);
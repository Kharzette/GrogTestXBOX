#pragma once
#include	<XTL.h>

typedef struct	Ship_t				Ship;
typedef struct	Mesh_t				Mesh;
typedef struct	UI_t				UI;
typedef struct	GraphicsDevice_t	GraphicsDevice;

Ship	*Ship_Init(Mesh *pMesh, int maxThrust,
		int fuelMax, int o2Max, int cargoMax, int hullMax, int mass, int it);
void	Ship_Update(Ship *pShip, float dt);
void	Ship_UpdateUI(Ship *pShip, UI *pUI, GraphicsDevice *pGD);

void	Ship_Turn(Ship *pShip, float deltaPitch, float deltaYaw, float deltaRoll);
void	Ship_Throttle(Ship *pShip, BYTE throttle);

void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD,
				D3DXMATRIX *pView, D3DXMATRIX *pProj);

const D3DXVECTOR3	*Ship_GetAttitude(Ship *pShip);
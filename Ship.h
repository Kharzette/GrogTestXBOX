#pragma once
#include	<utstring.h>

typedef struct	Ship_t				Ship;
typedef struct	Mesh_t				Mesh;
typedef struct	UI_t				UI;
typedef struct	GraphicsDevice_t	GraphicsDevice;

extern Ship	*Ship_Init(Mesh *pMesh, int maxThrust,
		int fuelMax, int o2Max, int cargoMax, int hullMax, int mass, int it);
extern void	Ship_Update(Ship *pShip, float dt);
extern void	Ship_UpdateUI(Ship *pShip, UI *pUI, GraphicsDevice *pGD);

extern void	Ship_Turn(Ship *pShip, float deltaPitch, float deltaYaw, float deltaRoll);
extern void	Ship_Throttle(Ship *pShip, BYTE throttle);

extern void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD);
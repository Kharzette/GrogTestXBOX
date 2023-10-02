#pragma once
#include	<utstring.h>

typedef struct	Ship_t				Ship;
typedef struct	Mesh_t				Mesh;
typedef struct	PilotUI_t			PilotUI;
typedef struct	GraphicsDevice_t	GraphicsDevice;

extern Ship	*Ship_Init(Mesh *pMesh);
extern void	Ship_Update(Ship *pShip, float dt);
extern void	Ship_UpdateUI(Ship *pShip, PilotUI *pUI, GraphicsDevice *pGD);

extern void	Ship_Turn(Ship *pShip, float dt, float deltaPitch, float deltaYaw, float deltaRoll);
extern void	Ship_Accelerate(Ship *pShip, float dt, BYTE throttle);

extern void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD);
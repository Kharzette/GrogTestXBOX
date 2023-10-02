#pragma once
#include	<utstring.h>

typedef struct	PilotUI_t			PilotUI;
typedef struct	UI_t				UI;
typedef struct	Font_t				Font;
typedef struct	GraphicsDevice_t	GraphicsDevice;



PilotUI	*PUI_Init(UI *pUI, GraphicsDevice *pGD, Font *pFont, LPDIRECT3DTEXTURE8 pFTex);

void	PUI_UpdateValues(PilotUI *pUI, GraphicsDevice *pGD,
			float v, float accel, int fuel,
			int o2, int cargo, int cargoMax, int hullHealth,
			int hullMax, int passengers, int passengerMax, int heading,
			int nadir, int wayHeading, int wayNadir, int heat,
			float coolingExtendPercent);

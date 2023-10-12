#pragma once
#include	<XTL.h>

typedef struct	UI_t				UI;
typedef struct	Font_t				Font;
typedef struct	GraphicsDevice_t	GraphicsDevice;



void	PUI_Init(UI *pUI, GraphicsDevice *pGD, Font *pFont, LPDIRECT3DTEXTURE8 pFTex);

void	PUI_UpdateValues(UI *pUI, GraphicsDevice *pGD,
			float v, float accel, INT64 fuel,
			int o2, INT64 cargo, INT64 cargoMax, int hullHealth,
			int hullMax, int passengers, int passengerMax, int heading,
			int nadir, float heat, float coolingExtendPercent,
			int velHeading, int velNadir, int brkHeading, int brkNadir);

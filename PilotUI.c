#include	<xtl.h>
#include	"UI.h"
#include	<utstring.h>


typedef struct	PilotUI_t
{
	UI	*pUI;

	UT_string	*mpszVelocity;
	UT_string	*mpszAccel;
	UT_string	*mpszFuelAir;
	UT_string	*mpszCargo;
	UT_string	*mpszPassengers;
	UT_string	*mpszNav;
	UT_string	*mpszHeat;

}	PilotUI;


PilotUI	*PUI_Init(UI *pUI, GraphicsDevice *pGD, Font *pFont, LPDIRECT3DTEXTURE8 pFTex)
{
	UT_string	*pDummy;
	PilotUI		*pRet	=malloc(sizeof(PilotUI));

	memset(pRet, 0, sizeof(PilotUI));

	pRet->pUI	=pUI;

	//strings for piloting UI
	utstring_new(pRet->mpszVelocity);
	utstring_new(pRet->mpszAccel);
	utstring_new(pRet->mpszFuelAir);
	utstring_new(pRet->mpszCargo);
	utstring_new(pRet->mpszPassengers);
	utstring_new(pRet->mpszNav);
	utstring_new(pRet->mpszHeat);

	utstring_new(pDummy);

	utstring_printf(pRet->mpszVelocity, "V");
	utstring_printf(pRet->mpszAccel, "A");
	utstring_printf(pRet->mpszFuelAir, "FA");
	utstring_printf(pRet->mpszCargo, "C");
	utstring_printf(pRet->mpszPassengers, "P");
	utstring_printf(pRet->mpszNav, "N");
	utstring_printf(pRet->mpszHeat, "H");

	utstring_printf(pDummy, "blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 16, pRet->mpszVelocity, pDummy);
	UI_AddString(pUI, pGD, pFont, pFTex, 16, pRet->mpszAccel, pDummy);
	UI_AddString(pUI, pGD, pFont, pFTex, 16, pRet->mpszFuelAir, pDummy);
	UI_AddString(pUI, pGD, pFont, pFTex, 20, pRet->mpszCargo, pDummy);
//	UI_AddString(pUI, pGD, pFont, pFTex, 16, pRet->mpszPassengers, pDummy);
	UI_AddString(pUI, pGD, pFont, pFTex, 16, pRet->mpszNav, pDummy);
	UI_AddString(pUI, pGD, pFont, pFTex, 20, pRet->mpszHeat, pDummy);

	{
		D3DXVECTOR2	half	={	0.5f, 0.5f	};
		UI_TextSetScale(pUI, pRet->mpszVelocity, &half);
		UI_TextSetScale(pUI, pRet->mpszAccel, &half);
		UI_TextSetScale(pUI, pRet->mpszFuelAir, &half);
		UI_TextSetScale(pUI, pRet->mpszCargo, &half);
		UI_TextSetScale(pUI, pRet->mpszNav, &half);
		UI_TextSetScale(pUI, pRet->mpszHeat, &half);
	}

	{
		D3DXVECTOR2	pos	={	20.0f, 20.0f	};

		UI_TextSetPosition(pUI, pRet->mpszVelocity, &pos);

		pos.y	+=30;
		UI_TextSetPosition(pUI, pRet->mpszAccel, &pos);

		pos.x	=600;
		pos.y	=20;
		UI_TextSetPosition(pUI, pRet->mpszFuelAir, &pos);

		pos.y	+=30;
		UI_TextSetPosition(pUI, pRet->mpszCargo, &pos);

		pos.x	=290;
		pos.y	=20;
		UI_TextSetPosition(pUI, pRet->mpszNav, &pos);

		pos.x	=20;
		pos.y	=440;
		UI_TextSetPosition(pUI, pRet->mpszHeat, &pos);
	}

	utstring_free(pDummy);

	UI_ComputeVB(pUI, pGD, pRet->mpszVelocity);
	UI_ComputeVB(pUI, pGD, pRet->mpszAccel);
	UI_ComputeVB(pUI, pGD, pRet->mpszFuelAir);
	UI_ComputeVB(pUI, pGD, pRet->mpszCargo);
	UI_ComputeVB(pUI, pGD, pRet->mpszNav);
	UI_ComputeVB(pUI, pGD, pRet->mpszHeat);

	return	pRet;
}


void	PUI_UpdateValues(PilotUI *pUI, GraphicsDevice *pGD,
			float v, float accel, int fuel,
			int o2, int cargo, int cargoMax, int hullHealth,
			int hullMax, int passengers, int passengerMax, int heading,
			int nadir, int wayHeading, int wayNadir, int heat,
			float coolingExtendPercent)
{
	UT_string	*pDummy;

	utstring_new(pDummy);

	if(v > 1000)
	{
		v	/=1000;
		utstring_printf(pDummy, "%4.2f km/s", v);
	}
	else
	{
		utstring_printf(pDummy, "%4.2f m/s", v);
	}

	UI_TextSetText(pUI->pUI, pUI->mpszVelocity, pDummy);

	utstring_free(pDummy);

	UI_ComputeVB(pUI->pUI, pGD, pUI->mpszVelocity);
}
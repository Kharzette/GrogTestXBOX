#include	<xtl.h>
#include	"UI.h"
#include	<utstring.h>

#define	VELOCITY_IDX	0
#define	ACCEL_IDX		1
#define	FUEL_IDX		2
#define	O2_IDX			3
#define	CARGO_IDX		4
#define	PASSENGER_IDX	5
#define	HEADING_IDX		6
#define	WAY_HEADING_IDX	7
#define	HEAT_IDX		8
#define	HULL_IDX		9
#define	RAD_EXTEND_IDX	10
#define	IDX_COUNT		11


void	PUI_Init(UI *pUI, GraphicsDevice *pGD, Font *pFont, LPDIRECT3DTEXTURE8 pFTex)
{
	UI_AddString(pUI, pGD, pFont, pFTex, 16, VELOCITY_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 16, ACCEL_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 16, FUEL_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 16, O2_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 20, CARGO_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 20, PASSENGER_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 20, HEADING_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 16, WAY_HEADING_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 16, HEAT_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 16, HULL_IDX, "Blort");
	UI_AddString(pUI, pGD, pFont, pFTex, 20, RAD_EXTEND_IDX, "Blort");

	{
		D3DXVECTOR2	half	={	0.5f, 0.5f	};
		int	i;

		for(i=0;i < IDX_COUNT;i++)
		{
			UI_TextSetScale(pUI, i, &half);
		}
	}

	{
		D3DXVECTOR2	pos	={	20.0f, 20.0f	};

		UI_TextSetPosition(pUI, VELOCITY_IDX, &pos);

		pos.y	+=30;
		UI_TextSetPosition(pUI, ACCEL_IDX, &pos);

		pos.x	=540;
		pos.y	=20;
		UI_TextSetPosition(pUI, FUEL_IDX, &pos);

		pos.y	+=30;
		UI_TextSetPosition(pUI, O2_IDX, &pos);

		pos.x	=520;
		pos.y	=400;
		UI_TextSetPosition(pUI, CARGO_IDX, &pos);

		pos.y	+=30;
		UI_TextSetPosition(pUI, HULL_IDX, &pos);

		pos.y	-=60;
		UI_TextSetPosition(pUI, PASSENGER_IDX, &pos);

		pos.x	=290;
		pos.y	=400;
		UI_TextSetPosition(pUI, WAY_HEADING_IDX, &pos);

		pos.x	=290;
		pos.y	=20;
		UI_TextSetPosition(pUI, HEADING_IDX, &pos);

		pos.x	=20;
		pos.y	=400;
		UI_TextSetPosition(pUI, HEAT_IDX, &pos);

		pos.y	+=30;
		UI_TextSetPosition(pUI, RAD_EXTEND_IDX, &pos);
	}

	{
		int	i;
		for(i=0;i < IDX_COUNT;i++)
		{
			UI_ComputeVB(pUI, pGD, i);
		}
	}
}


void	PUI_UpdateValues(UI *pUI, GraphicsDevice *pGD,
			float v, float accel, int fuel,
			int o2, int cargo, int cargoMax, int hullHealth,
			int hullMax, int passengers, int passengerMax, int heading,
			int nadir, int wayHeading, int wayNadir, int heat,
			float coolingExtendPercent)
{
	char	buf[32];

	if(v >= 1000)
	{
		v	/=1000;
		sprintf(buf, "%4.2f km/s", v);
	}
	else
	{
		sprintf(buf, "%4.2f m/s", v);
	}
	UI_TextSetText(pUI, VELOCITY_IDX, buf);

	sprintf(buf, "%4.2f G", accel);
	UI_TextSetText(pUI, ACCEL_IDX, buf);

	//wrap into  positive
	if(heading < 0)
	{
		heading	+=360;
	}
	sprintf(buf, "HDG: %d, %d", heading, nadir);
	UI_TextSetText(pUI, HEADING_IDX, buf);

	if(fuel >= 10000)
	{
		sprintf(buf, "Fuel: %4.2f T", fuel / 10000.0f);
	}
	else if(fuel >= 1000)
	{
		sprintf(buf, "Fuel: %4.2f kg", fuel / 1000.0f);
	}
	else
	{
		sprintf(buf, "Fuel: %d g", fuel);
	}	
	UI_TextSetText(pUI, FUEL_IDX, buf);

	sprintf(buf, "O2: %d", o2);
	UI_TextSetText(pUI, O2_IDX, buf);

	if(cargoMax >= 10000)
	{
		if(cargo >= 10000)
		{
			sprintf(buf, "CRG %4.2f/%4.2f T", cargo, cargoMax);
		}
		else		
		{
			sprintf(buf, "CRG %1.4f/%4.2f T", cargo, cargoMax);
		}
	}
	else if(cargoMax >= 1000)
	{
		if(cargo >= 1000)
		{
			sprintf(buf, "CRG %4.2f/%4.2f kg", cargo, cargoMax);
		}
		else
		{
			sprintf(buf, "CRG %1.4f/%4.2f kg", cargo, cargoMax);
		}
	}
	else
	{
		sprintf(buf, "CRG %d/%d g", cargo, cargoMax);
	}
	UI_TextSetText(pUI, CARGO_IDX, buf);

	sprintf(buf, "PSNGRS: %d/%d", passengers, passengerMax);
	UI_TextSetText(pUI, PASSENGER_IDX, buf);

	//wrap into  positive
	if(wayHeading < 0)
	{
		wayHeading	+=360;
	}
	sprintf(buf, "WAY: %d, %d", wayHeading, -wayNadir);
	UI_TextSetText(pUI, WAY_HEADING_IDX, buf);

	sprintf(buf, "CORE: %d K", heat);
	UI_TextSetText(pUI, HEAT_IDX, buf);

	sprintf(buf, "HULL: %d%%", (int)(hullHealth / (float)hullMax) * 100);
	UI_TextSetText(pUI, HULL_IDX, buf);

	sprintf(buf, "RAD: %d%%", (int)(coolingExtendPercent * 100));
	UI_TextSetText(pUI, RAD_EXTEND_IDX, buf);

	{
		int	i;
		for(i=0;i < IDX_COUNT;i++)
		{
			UI_ComputeVB(pUI, pGD, i);
		}
	}
}
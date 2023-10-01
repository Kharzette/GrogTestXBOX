#pragma once
#include	<utstring.h>

typedef struct	UI_t				UI;
typedef struct	Font_t				Font;
typedef struct	GraphicsDevice_t	GraphicsDevice;

#define	ORIGIN_CENTER	0
#define	ORIGIN_LEFT		1
#define	ORIGIN_RIGHT	2
#define	ORIGIN_TOP		3
#define	ORIGIN_BOTTOM	4

extern UI	*UI_Init(GraphicsDevice *pGD);
extern BOOL	UI_AddString(UI *pUI, GraphicsDevice *pGD,
				Font *pFont, LPDIRECT3DTEXTURE8 pFontTex,
				int maxChars, UT_string *pKey, UT_string *pString);
extern void	UI_Draw(UI *pUI, GraphicsDevice *pGD);

extern void	UI_TextSetColour(UI *pUI, UT_string *pKey, D3DXVECTOR4 col);
extern void	UI_TextSetPosition(UI *pUI, UT_string *pKey, D3DXVECTOR2 pos);
extern void	UI_TextSetScale(UI *pUI, UT_string *pKey, D3DXVECTOR2 scale);
extern void	UI_TextSetOrigin(UI *pUI, UT_string *pKey, BYTE xOrg, BYTE yOrg);
extern void	UI_TextSetFont(UI *pUI, UT_string *pKey, Font *pFont, LPDIRECT3DTEXTURE8 pFontTex);
extern void	UI_TextSetRect(UI *pUI, UT_string *pKey, RECT r);
extern void	UI_TextSetText(UI *pUI, UT_string *pKey, UT_string *pText);

//call this whenever something changes that affects the VB
extern void	UI_ComputeVB(UI *pUI, GraphicsDevice *pGD, UT_string *pKey);

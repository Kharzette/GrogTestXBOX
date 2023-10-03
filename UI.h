#pragma once

typedef struct	UI_t				UI;
typedef struct	Font_t				Font;
typedef struct	GraphicsDevice_t	GraphicsDevice;

#define	ORIGIN_CENTER	0
#define	ORIGIN_LEFT		1
#define	ORIGIN_RIGHT	2
#define	ORIGIN_TOP		3
#define	ORIGIN_BOTTOM	4

extern UI	*UI_Init(GraphicsDevice *pGD, int arraySize);
extern BOOL	UI_AddString(UI *pUI, GraphicsDevice *pGD,
				Font *pFont, LPDIRECT3DTEXTURE8 pFontTex,
				int maxChars, int idx, const char *pString);
extern void	UI_Draw(UI *pUI, GraphicsDevice *pGD);

extern void	UI_TextSetColour(UI *pUI, int idx, const D3DXVECTOR4 *col);
extern void	UI_TextSetPosition(UI *pUI, int idx, const D3DXVECTOR2 *pos);
extern void	UI_TextSetScale(UI *pUI, int idx, const D3DXVECTOR2 *scale);
extern void	UI_TextSetOrigin(UI *pUI, int idx, BYTE xOrg, BYTE yOrg);
extern void	UI_TextSetFont(UI *pUI, int idx, Font *pFont, LPDIRECT3DTEXTURE8 pFontTex);
extern void	UI_TextSetRect(UI *pUI, int idx, RECT r);
extern void	UI_TextSetText(UI *pUI, int idx, const char *pText);

//call this whenever something changes that affects the VB
extern void	UI_ComputeVB(UI *pUI, GraphicsDevice *pGD, int idx);

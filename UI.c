#include	<xtl.h>
#include	<assert.h>
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/UtilityLib/StringStuff.h"
#include	"GrogLibsXBOX/UtilityLib/DictionaryStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"
#include	"GrogLibsXBOX/MaterialLib/Font.h"
#include	"UI.h"


typedef struct	TextVert_t
{
	D3DXVECTOR4	Position;
}	TextVert;

typedef struct	TextData_t
{
	D3DXVECTOR4		mColor;
	D3DXVECTOR2		mPosition;
	D3DXVECTOR2		mScale;
	BYTE			mXOrg, mYOrg;	//origin at which corner of the rect?
	Font			*mpFont;		//font (don't free)
	BOOL			mbWordWrap;		//wrap long paragraphs?
	RECT			mRect;			//place for the text to go
	UT_string		*mpText;		//actual text

	LPDIRECT3DTEXTURE8		mpFontTex;	//font tex (don't free)
	IDirect3DVertexBuffer8	*mpVB;		//vbuffer for this gumptext
}	TextData;

typedef struct	UI_t
{
	//onscreen text pieces
	DictSZ	*mpText;	//string key to TextData

	//text shaders
	DWORD	mVSHandle, mPSHandle;
}	UI;


UI	*UI_Init(GraphicsDevice *pGD)
{
	DWORD	vertDecl[3];
	UI		*pRet		=malloc(sizeof(UI));

	DictSZ_New(&pRet->mpText);

	//vertex declaration, sorta like input layouts on 11
	vertDecl[0]	=D3DVSD_STREAM(0);
	vertDecl[1]	=D3DVSD_REG(0, D3DVSDT_FLOAT4);
	vertDecl[2]	=D3DVSD_END();

	pRet->mVSHandle	=LoadCompiledVShader(pGD, vertDecl, "D:\\Media\\ShaderLib\\Text.xvu");
	pRet->mPSHandle	=LoadCompiledPShader(pGD, "D:\\Media\\ShaderLib\\Text.xpu");

	return	pRet;
}

BOOL	UI_AddString(UI *pUI, GraphicsDevice *pGD,
			Font *pFont, LPDIRECT3DTEXTURE8 pFontTex,
			int maxChars, UT_string *pKey, UT_string *pString)
{
	TextData	*pTD;

	if(DictSZ_ContainsKey(pUI->mpText, pKey))
	{
		return	FALSE;
	}

	pTD	=malloc(sizeof(TextData));
	memset(pTD, 0, sizeof(TextData));

	pTD->mpFont		=pFont;
	pTD->mpFontTex	=pFontTex;

	pTD->mScale.x	=pTD->mScale.y	=1.0f;

	GD_CreateVertexBuffer(pGD, NULL, maxChars * 4, &pTD->mpVB);

	DictSZ_Add(&pUI->mpText, pKey, pTD);
}

static void	DrawCB(const UT_string *pKey, const void *pValue, void *pContext)
{
	const TextData	*pText	=(const TextData *)pValue;
	GraphicsDevice	*pGD	=(GraphicsDevice *)pContext;

	D3DXVECTOR4	posScale	={ pText->mPosition.x, pText->mPosition.y,
		pText->mScale.x, pText->mScale.y	};

	GD_SetTexture(pGD, 0, pText->mpFontTex);

	GD_SetPShaderConstant(pGD, 0, &pText->mColor, 1);
	GD_SetVShaderConstant(pGD, 0, &posScale, 1);

	GD_SetStreamSource(pGD, 0, pText->mpVB, sizeof(TextVert));
	GD_SetIndices(pGD, NULL, 0);
	GD_DrawVertices(pGD, D3DPT_QUADLIST, 0, utstring_len(pText->mpText) * 4);
}

void	UI_Draw(UI *pUI, GraphicsDevice *pGD)
{
	D3DXVECTOR4	handyNums	={	-0.5f, 1.0f, 0.0f, 2.0f	};

	//set shaders, eventually want to load all and use offsets
	GD_SetVertexShader(pGD, pUI->mVSHandle);
	GD_SetPixelShader(pGD, pUI->mPSHandle);

	GD_SetVShaderConstant(pGD, 1, &handyNums, 1);

	DictSZ_ForEach(pUI->mpText, DrawCB, pGD);
}

void	UI_TextSetColour(UI *pUI, UT_string *pKey, D3DXVECTOR4 col)
{
	TextData	*pText	=DictSZ_GetValue(pUI->mpText, pKey);

	pText->mColor	=col;
}

void	UI_TextSetPosition(UI *pUI, UT_string *pKey, D3DXVECTOR2 pos)
{
	TextData	*pText	=DictSZ_GetValue(pUI->mpText, pKey);

	pText->mPosition	=pos;
}

void	UI_TextSetScale(UI *pUI, UT_string *pKey, D3DXVECTOR2 scale)
{
	TextData	*pText	=DictSZ_GetValue(pUI->mpText, pKey);

	pText->mScale	=scale;
}

void	UI_TextSetOrigin(UI *pUI, UT_string *pKey, BYTE xOrg, BYTE yOrg)
{
	TextData	*pText	=DictSZ_GetValue(pUI->mpText, pKey);

	pText->mXOrg	=xOrg;
	pText->mYOrg	=yOrg;
}

void	UI_TextSetFont(UI *pUI, UT_string *pKey, Font *pFont, LPDIRECT3DTEXTURE8 pFontTex)
{
	TextData	*pText	=DictSZ_GetValue(pUI->mpText, pKey);

	pText->mpFont		=pFont;
	pText->mpFontTex	=pFontTex;
}

void	UI_TextSetRect(UI *pUI, UT_string *pKey, RECT r)
{
	TextData	*pText	=DictSZ_GetValue(pUI->mpText, pKey);

	pText->mRect	=r;
}

void	UI_TextSetText(UI *pUI, UT_string *pKey, UT_string *pText)
{
	TextData	*pTD	=DictSZ_GetValue(pUI->mpText, pKey);

	if(pTD->mpText != NULL)
	{
		utstring_free(pTD->mpText);
	}
	utstring_new(pTD->mpText);

	utstring_concat(pTD->mpText, pText);
}

//call this whenever something changes that affects the VB
void	UI_ComputeVB(UI *pUI, GraphicsDevice *pGD, UT_string *pKey)
{
	TextData	*pTD	=DictSZ_GetValue(pUI->mpText, pKey);
	
	int	curWidth, i, len	=utstring_len(pTD->mpText);

	//alloc verts
	TextVert	*pVerts	=malloc(sizeof(TextVert) * len * 4);

	//start with a basic top left origin one line string
	curWidth	=0;
	for(i=0;i < len;i++)
	{
		D3DXVECTOR2	uv;

		char	letter	=utstring_body(pTD->mpText)[i];

		int	nextWidth	=curWidth + Font_GetCharacterWidth(pTD->mpFont, letter);
		int	height		=Font_GetCharacterHeight(pTD->mpFont);

		uv	=Font_GetUV(pTD->mpFont, letter, 0);

		pVerts[i * 4].Position.x	=curWidth;
		pVerts[i * 4].Position.y	=0.0f;
		pVerts[i * 4].Position.z	=uv.x;
		pVerts[i * 4].Position.w	=uv.y;

		uv	=Font_GetUV(pTD->mpFont, letter, 3);

		pVerts[(i * 4) + 3].Position.x	=nextWidth;
		pVerts[(i * 4) + 3].Position.y	=0.0f;
		pVerts[(i * 4) + 3].Position.z	=uv.x;
		pVerts[(i * 4) + 3].Position.w	=uv.y;

		uv	=Font_GetUV(pTD->mpFont, letter, 2);

		pVerts[(i * 4) + 2].Position.x	=nextWidth;
		pVerts[(i * 4) + 2].Position.y	=height;
		pVerts[(i * 4) + 2].Position.z	=uv.x;
		pVerts[(i * 4) + 2].Position.w	=uv.y;

		uv	=Font_GetUV(pTD->mpFont, letter, 1);

		pVerts[(i * 4) + 1].Position.x	=curWidth;
		pVerts[(i * 4) + 1].Position.y	=height;
		pVerts[(i * 4) + 1].Position.z	=uv.x;
		pVerts[(i * 4) + 1].Position.w	=uv.y;

		curWidth	=nextWidth;
	}

	GD_SetVBData(pGD, pTD->mpVB, sizeof(TextVert) * len * 4, pVerts);

	free(pVerts);
}
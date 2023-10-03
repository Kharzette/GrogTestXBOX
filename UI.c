#include	<xtl.h>
#include	<assert.h>
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"
#include	"GrogLibsXBOX/MaterialLib/Font.h"
#include	"UI.h"

//see the vertex shader assembler reference page in the docs
//actually looks better without it!?!?
#define	PIXEL_OFFSET	0.0f
//#define	PIXEL_OFFSET	0.53125f

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
	char			*szText;		//actual text	

	LPDIRECT3DTEXTURE8		mpFontTex;	//font tex (don't free)
	IDirect3DVertexBuffer8	*mpVB;		//vbuffer for this gumptext
	int						mVBSize;	//Num Characters represented in the VB
}	TextData;

typedef struct	UI_t
{
	//onscreen text pieces array
	TextData	*mpText;
	int			mArraySize;

	//text shaders
	DWORD	mVSHandle, mPSHandle;
}	UI;


//fixed number of UI strings onscreen
UI	*UI_Init(GraphicsDevice *pGD, int arraySize)
{
	DWORD	vertDecl[3];
	UI		*pRet		=malloc(sizeof(UI));

	//alloc string storage
	pRet->mpText		=malloc(sizeof(TextData) * arraySize);
	pRet->mArraySize	=arraySize;

	//clear array
	memset(pRet->mpText, 0, sizeof(TextData) * arraySize);

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
			int maxChars, int idx, const char *pString)
{
	TextData	*pTD;

	if(idx < 0 || idx > pUI->mArraySize)
	{
		return	FALSE;
	}

	pTD	=&pUI->mpText[idx];

	pTD->mpFont		=pFont;
	pTD->mpFontTex	=pFontTex;

	//copy string
	assert(pTD->szText == NULL);

	//alloc chars for the maximum size plus null
	pTD->szText	=malloc(maxChars + 1);
	memset(pTD->szText, 0, maxChars + 1);

	strncpy(pTD->szText, pString, maxChars);

	//some defaults
	pTD->mScale.x	=pTD->mScale.y	=1.0f;
	pTD->mColor.x	=pTD->mColor.y	=pTD->mColor.z	=pTD->mColor.w	=1.0f;

	//keeping this at a string size
	pTD->mVBSize	=maxChars;

	//I love quads
	GD_CreateVertexBuffer(pGD, NULL, pTD->mVBSize * sizeof(TextVert) * 4, &pTD->mpVB);

	return	TRUE;
}

static void	DrawTD(const TextData *pText, GraphicsDevice *pGD)
{
	int			len;
	D3DXVECTOR4	posScale	={ pText->mPosition.x, pText->mPosition.y,
		pText->mScale.x, pText->mScale.y	};

	if(pText->mpVB == NULL)
	{
		return;
	}

	//make sure no unterminated craziness
	len	=strlen(pText->szText);
	if(len == 0 || len > pText->mVBSize)
	{
		//TODO: warn or something
		return;
	}

	GD_SetTexture(pGD, 0, pText->mpFontTex);

	GD_SetVShaderConstant(pGD, 0, &posScale, 1);
	GD_SetPShaderConstant(pGD, 0, &pText->mColor, 1);

	GD_SetStreamSource(pGD, 0, pText->mpVB, sizeof(TextVert));
	GD_SetIndices(pGD, NULL, 0);
	GD_DrawVertices(pGD, D3DPT_QUADLIST, 0, strlen(pText->szText) * 4);
}

void	UI_Draw(UI *pUI, GraphicsDevice *pGD)
{
	D3DXVECTOR4	handyNums	={	-0.5f, 1.0f, 0.0f, 2.0f	};

	//set shaders, eventually want to load all and use offsets
	GD_SetVertexShader(pGD, pUI->mVSHandle);
	GD_SetPixelShader(pGD, pUI->mPSHandle);

	GD_SetVShaderConstant(pGD, 1, &handyNums, 1);

	//trying some renderstate crap from a sample
	GD_SetRenderState(pGD, D3DRS_ALPHABLENDENABLE,	TRUE);
	GD_SetRenderState(pGD, D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
	GD_SetRenderState(pGD, D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA);
	GD_SetRenderState(pGD, D3DRS_ALPHATESTENABLE,	FALSE);
	GD_SetRenderState(pGD, D3DRS_ALPHAFUNC,			D3DCMP_ALWAYS);
	GD_SetRenderState(pGD, D3DRS_FILLMODE,			D3DFILL_SOLID);
	GD_SetRenderState(pGD, D3DRS_CULLMODE,			D3DCULL_NONE);
	GD_SetRenderState(pGD, D3DRS_ZENABLE,			FALSE);
	GD_SetRenderState(pGD, D3DRS_STENCILENABLE,		FALSE);
	GD_SetRenderState(pGD, D3DRS_EDGEANTIALIAS,		FALSE);

	GD_SetTextureStageState(pGD, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	GD_SetTextureStageState(pGD, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	GD_SetTextureStageState(pGD, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	GD_SetTextureStageState(pGD, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	GD_SetTextureStageState(pGD, 0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);

	//draw all strings
	{
		int	i;
		for(i=0;i < pUI->mArraySize;i++)
		{
			DrawTD(&pUI->mpText[i], pGD);
		}
	}

	GD_SetRenderState(pGD, D3DRS_CULLMODE, D3DCULL_CCW);
	GD_SetRenderState(pGD, D3DRS_ZENABLE, TRUE);
	GD_SetRenderState(pGD, D3DRS_ALPHABLENDENABLE, FALSE );
}

void	UI_TextSetColour(UI *pUI, int idx, const D3DXVECTOR4 *pCol)
{
	if(idx < 0 || idx > pUI->mArraySize)
	{
		return;
	}
	pUI->mpText[idx].mColor	=*pCol;
}

void	UI_TextSetPosition(UI *pUI, int idx, const D3DXVECTOR2 *pPos)
{
	if(idx < 0 || idx > pUI->mArraySize)
	{
		return;
	}
	pUI->mpText[idx].mPosition	=*pPos;
}

void	UI_TextSetScale(UI *pUI, int idx, const D3DXVECTOR2 *pScale)
{
	if(idx < 0 || idx > pUI->mArraySize)
	{
		return;
	}
	pUI->mpText[idx].mScale	=*pScale;
}

void	UI_TextSetOrigin(UI *pUI, int idx, BYTE xOrg, BYTE yOrg)
{
	if(idx < 0 || idx > pUI->mArraySize)
	{
		return;
	}
	pUI->mpText[idx].mXOrg	=xOrg;
	pUI->mpText[idx].mYOrg	=yOrg;
}

void	UI_TextSetFont(UI *pUI, int idx, Font *pFont, LPDIRECT3DTEXTURE8 pFontTex)
{
	if(idx < 0 || idx > pUI->mArraySize)
	{
		return;
	}
	pUI->mpText[idx].mpFont		=pFont;
	pUI->mpText[idx].mpFontTex	=pFontTex;
}

void	UI_TextSetRect(UI *pUI, int idx, RECT r)
{
	if(idx < 0 || idx > pUI->mArraySize)
	{
		return;
	}
	pUI->mpText[idx].mRect	=r;
}

void	UI_TextSetText(UI *pUI, int idx, const char *pText)
{
	if(idx < 0 || idx > pUI->mArraySize)
	{
		return;
	}
	//ensure no overflow
	strncpy(pUI->mpText[idx].szText, pText, pUI->mpText[idx].mVBSize);
}

//temporary buffer for vb copies
//the joys of single threading!
static TextVert	tempBuf[20 * 4];

//call this whenever something changes that affects the VB
//this is super lazy and probably super slow
void	UI_ComputeVB(UI *pUI, GraphicsDevice *pGD, int idx)
{
	TextVert	*pVerts;
	TextData	*pTD;
	int			curWidth, i, len;

	if(idx < 0 || idx > pUI->mArraySize)
	{
		//TODO: warn
		return;
	}
	pTD	=&pUI->mpText[idx];
	
	len	=strlen(pTD->szText);

	//sanity check
	if(len == 0 || len > pTD->mVBSize)
	{
		//TODO: warn
		return;
	}

	if(len < 20)
	{
		pVerts	=tempBuf;
	}
	else
	{
		//alloc temp verts
		pVerts	=malloc(sizeof(TextVert) * len * 4);
	}

	//start with a basic top left origin one line string
	curWidth	=0;
	for(i=0;i < len;i++)
	{
		D3DXVECTOR2	uv;

		char	letter	=pTD->szText[i];

		int	nextWidth	=curWidth + Font_GetCharacterWidth(pTD->mpFont, letter);
		int	height		=Font_GetCharacterHeight(pTD->mpFont);

		uv	=Font_GetUV(pTD->mpFont, letter, 0);

		pVerts[i * 4].Position.x	=PIXEL_OFFSET + (float)curWidth;
		pVerts[i * 4].Position.y	=PIXEL_OFFSET;
		pVerts[i * 4].Position.z	=uv.x;
		pVerts[i * 4].Position.w	=uv.y;

		uv	=Font_GetUV(pTD->mpFont, letter, 1);

		pVerts[(i * 4) + 1].Position.x	=PIXEL_OFFSET + (float)nextWidth;
		pVerts[(i * 4) + 1].Position.y	=PIXEL_OFFSET;
		pVerts[(i * 4) + 1].Position.z	=uv.x;
		pVerts[(i * 4) + 1].Position.w	=uv.y;

		uv	=Font_GetUV(pTD->mpFont, letter, 2);

		pVerts[(i * 4) + 2].Position.x	=PIXEL_OFFSET + (float)nextWidth;
		pVerts[(i * 4) + 2].Position.y	=PIXEL_OFFSET + (float)height;
		pVerts[(i * 4) + 2].Position.z	=uv.x;
		pVerts[(i * 4) + 2].Position.w	=uv.y;

		uv	=Font_GetUV(pTD->mpFont, letter, 3);

		pVerts[(i * 4) + 3].Position.x	=PIXEL_OFFSET + (float)curWidth;
		pVerts[(i * 4) + 3].Position.y	=PIXEL_OFFSET + (float)height;
		pVerts[(i * 4) + 3].Position.z	=uv.x;
		pVerts[(i * 4) + 3].Position.w	=uv.y;

		curWidth	=nextWidth;
	}

	GD_SetVBData(pGD, pTD->mpVB, sizeof(TextVert) * len * 4, pVerts);

	//free verts if needed for a big string
	if(len >= 20)
	{
		free(pVerts);
	}
}
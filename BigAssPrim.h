#pragma once
#include	<XTL.h>

typedef struct	BigAssPrim_t		BigAssPrim;
typedef struct	GraphicsDevice_t	GraphicsDevice;


BigAssPrim	*BAP_Init(GraphicsDevice *pGD);

void	BAP_Draw(const BigAssPrim *pBAP, GraphicsDevice *pGD);
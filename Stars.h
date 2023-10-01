#pragma once

typedef struct	StarsTag			Stars;
typedef struct	GraphicsDevice_t	GraphicsDevice;

extern Stars	*Stars_Generate(GraphicsDevice *pGD);
extern void		Stars_Draw(Stars *pStars, GraphicsDevice *pGD);

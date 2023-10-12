#pragma once

typedef struct XBCtag	XBC;

extern XBC	*XBC_Init(void);
extern void	XBC_UpdateInput(XBC *pXBC);
extern void	XBC_PrintInput(XBC	*pXBC);

extern void	XBC_GetAnalogLeft(const XBC *pXBC, SHORT *pX, SHORT *pY);
extern void	XBC_GetAnalogRight(const XBC *pXBC, SHORT *pX, SHORT *pY);
extern void	XBC_GetLeftTrigger(const XBC *pXBC, BYTE *val);
extern void	XBC_GetRightTrigger(const XBC *pXBC, BYTE *val);
extern BOOL	XBC_ButtonTapped(const XBC *pXBC, WORD btn);
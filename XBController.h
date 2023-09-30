//#include	<xtl.h>

typedef struct XBCtag	XBC;

extern XBC	*XBC_Init(void);
extern void	XBC_UpdateInput(XBC *pXBC);
extern void	XBC_PrintInput(XBC	*pXBC);

extern void	XBC_GetAnalogLeft(const XBC *pXBC, SHORT *pX, SHORT *pY);

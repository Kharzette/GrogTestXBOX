#pragma once

typedef struct	UI_t				UI;
typedef struct	Font_t				Font;
typedef struct	Ship_t				Ship;
typedef	struct	WayPoints_t			WayPoints;
typedef struct	GraphicsDevice_t	GraphicsDevice;


WayPoints	*WayPoints_Init(int arraySize);

void	WayPoints_Add(WayPoints *pWP, const char *pName,
						Vec3Int32 secPos, D3DXVECTOR3 inSectorPos);
void	WayPoints_DisposeByIndex(WayPoints *pWP, int idx);
void	WayPoints_ComputeHeadingToIndex(const WayPoints *pWP,
			const Ship *pShip, int idx,
			int *pHeading, int *pNadir, INT64 *pDist);

int	WayPoints_ValidIndex(const WayPoints *pWP, int idx);

const char	*WayPoints_GetName(const WayPoints *pWP, int idx);
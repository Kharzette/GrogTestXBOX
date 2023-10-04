#pragma once
#include	<XTL.h>

typedef struct	DroneCam_t				DroneCam;


DroneCam	*DroneCam_Init(void);

void		DroneCam_GetCameraMatrix(const DroneCam *pDC,
				const D3DXQUATERNION *pAttachedRot, D3DXMATRIX *pMat);

void		DroneCam_DistanceChange(DroneCam *pDC, float delta);
void		DroneCam_SetMinMaxDistance(DroneCam *pDC, float minDist, float maxDist);
void		DroneCam_Rotate(DroneCam *pDC, float deltaPitch, float deltaYaw, float deltaRoll);
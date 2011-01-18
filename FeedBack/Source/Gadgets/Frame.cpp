#include "Fuji.h"
#include "MFMaterial.h"
#include "MFPrimitive.h"

#include "Gadgets/Frame.h"

dBFrame::dBFrame()
{
	colours[0] = colours[1] = colours[2] = colours[3] = MFVector::white;
	pMat = NULL;
	borderWidth = 16.0f;
}

dBFrame::~dBFrame()
{
	if(pMat)
		MFMaterial_Destroy(pMat);
}

void dBFrame::Draw()
{
	if(!pMat)
	{
		MFPrimitive_DrawUntexturedQuad(rect.x-10, rect.y-10, rect.width+20, rect.height+20, colours[0]);
	}
	else
	{
		// draw background
		float x, y, xRemaining, yRemaining;

		// calculate number of tiles (including edges)
		int h = (int)MFCeil(rect.width / borderWidth);
		int w = (int)MFCeil(rect.height / borderWidth);
		int numTiles = (h + 2) * (w + 2);

		// begin immediate renderer
		MFMaterial_SetMaterial(pMat);
		MFPrimitive(PT_QuadList);
		MFBegin(numTiles*2);

		MFSetColour(colours[0]);

		// render tiled background
		yRemaining = rect.height;
		for(y = 0.0f; y < rect.height; y += borderWidth)
		{
			xRemaining = rect.width;
			for(x = 0.0f; x < rect.width; x += borderWidth)
			{
				float xuv = xRemaining < borderWidth ? 0.25f * (xRemaining / borderWidth) : 0.25f;
				float yuv = yRemaining < borderWidth ? 0.25f * (yRemaining / borderWidth) : 0.25f;

				MFSetTexCoord1(0.25f, 0.25f);
				MFSetPosition(rect.x + x, rect.y + y, 0);
				MFSetTexCoord1(0.25f + xuv, 0.25f + yuv);		
				MFSetPosition(rect.x + x + MFMin(borderWidth, xRemaining), rect.y + y + MFMin(borderWidth, yRemaining), 0);

				xRemaining -= borderWidth;
			}

			yRemaining -= borderWidth;
		}

		// draw frame
		// draw corners
		MFSetTexCoord1(0.0f, 0.0f);
		MFSetPosition(rect.x - borderWidth, rect.y - borderWidth, 0.0f);
		MFSetTexCoord1(0.25f, 0.25f);		
		MFSetPosition(rect.x, rect.y, 0.0f);
		MFSetTexCoord1(0.5f, 0.0f);
		MFSetPosition(rect.x + rect.width, rect.y - borderWidth, 0.0f);
		MFSetTexCoord1(0.75f, 0.25f);		
		MFSetPosition(rect.x + rect.width + borderWidth, rect.y, 0.0f);
		MFSetTexCoord1(0.0f, 0.50f);
		MFSetPosition(rect.x - borderWidth, rect.y + rect.height, 0.0f);
		MFSetTexCoord1(0.25f, 0.75f);		
		MFSetPosition(rect.x, rect.y + rect.height + borderWidth, 0.0f);
		MFSetTexCoord1(0.5f, 0.5f);
		MFSetPosition(rect.x + rect.width, rect.y + rect.height, 0.0f);
		MFSetTexCoord1(0.75f, 0.75f);		
		MFSetPosition(rect.x + rect.width + borderWidth, rect.y + rect.height + borderWidth, 0.0f);

		// draw vertical edges
		yRemaining = rect.height;
		for(y = 0.0f; y < rect.height; y += borderWidth)
		{
			float yuv = yRemaining < borderWidth ? 0.25f * (yRemaining / borderWidth) : 0.25f;

			MFSetTexCoord1(0.0f, 0.25f);
			MFSetPosition(rect.x - borderWidth, rect.y + y, 0.0f);
			MFSetTexCoord1(0.25f, 0.25f + yuv);		
			MFSetPosition(rect.x, rect.y + y + MFMin(borderWidth, yRemaining), 0.0f);

			MFSetTexCoord1(0.5f, 0.25f);
			MFSetPosition(rect.x + rect.width, rect.y + y, 0.0f);
			MFSetTexCoord1(0.75f, 0.25f + yuv);		
			MFSetPosition(rect.x + rect.width + borderWidth, rect.y + y + MFMin(borderWidth, yRemaining), 0.0f);

			yRemaining -= borderWidth;
		}

		// draw horizontal edges
		xRemaining = rect.width;
		for(x = 0.0f; x < rect.width; x += borderWidth)
		{
			float xuv = xRemaining < borderWidth ? 0.25f * (xRemaining / borderWidth) : 0.25f;

			MFSetTexCoord1(0.25f, 0.0f);
			MFSetPosition(rect.x + x, rect.y - borderWidth, 0.0f);
			MFSetTexCoord1(0.25f + xuv, 0.25f);		
			MFSetPosition(rect.x + x + MFMin(borderWidth, xRemaining), rect.y, 0.0f);

			MFSetTexCoord1(0.25f, 0.5f);
			MFSetPosition(rect.x + x, rect.y + rect.height, 0.0f);
			MFSetTexCoord1(0.25f + xuv, 0.75f);		
			MFSetPosition(rect.x + x + MFMin(borderWidth, xRemaining), rect.y + rect.height + borderWidth, 0.0f);

			xRemaining -= borderWidth;
		}

		MFEnd();
	}
}

void dBFrame::SetFrame(MFRect *pRect)
{
	rect = *pRect;
}

void dBFrame::SetPosition(const MFVector &pos)
{
	rect.x = pos.x;
	rect.y = pos.y;
}

void dBFrame::SetSize(float width, float height)
{
	rect.width = width;
	rect.height = height;
}

void dBFrame::SetBorderWidth(float _borderWidth)
{
	borderWidth = _borderWidth;
}

void dBFrame::SetMaterial(const char *pMaterial)
{
	if(pMaterial)
	{
		pMat = MFMaterial_Create(pMaterial);
		MFVector colour = MakeVector(0.2f, 0.5f, 1, 1);
		SetCornerColours(colour, colour, colour, colour);
//		int zread = MFMaterial_GetParameterIndexFromName(pMat, "zread");
//		size_t off = 0;
//		MFMaterial_SetParameter(pMat, zread, 0, &off);
	}
	else if(pMat)
	{
		MFMaterial_Destroy(pMat);
		pMat = NULL;
	}
}

void dBFrame::SetCornerColours(const MFVector &topLeft, const MFVector &topRight, const MFVector &bottomLeft, const MFVector &bottomRight)
{
	colours[0] = topLeft;
	colours[1] = topRight;
	colours[2] = bottomLeft;
	colours[3] = bottomRight;
}

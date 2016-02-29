#include "FeedBack.h"
#include "Fuji/MFFileSystem.h"
#include "Fuji/MFTexture.h"
#include "Fuji/MFMaterial.h"
#include "Fuji/MFModel.h"
#include "Fuji/DebugMenu.h"
#include "Track.h"

#include "Screens/Editor.h"

MenuItemBool bHalfFrets(true);
MenuItemBool bRenderNoteTimes(false);

MFVector Fretboard::gColours[5] =
{
	MakeVector(0,1,0,1),
	MakeVector(1,0,0,1),
	MakeVector(1,1,0,1),
	MakeVector(0,0,1,1),
	MakeVector(1,0.5f,0,1)
};

dBTrack::dBTrack()
{
	scrollSpeed = 12.f;
	viewPoint = 0;
}

dBTrack::~dBTrack()
{
}

void DrawRing(float x, float z, float size, bool dark = false)
{
	MFPrimitive(PT_TriStrip, 0);
	MFBegin(4);
	MFSetColourV(MFVector::white * (dark ? 0.6f : 1.0f));
	MFSetTexCoord1(0.0f, 0.0f);
	MFSetPosition(x, z, size*0.5f);
	MFSetTexCoord1(1.0f, 0.0f);
	MFSetPosition(x + size, z, size*0.5f);
	MFSetTexCoord1(0.0f, 1.0f);
	MFSetPosition(x, z, -size*0.5f);
	MFSetTexCoord1(1.0f, 1.0f);
	MFSetPosition(x + size, z, -size*0.5f);
	MFEnd();
}

Fretboard::Fretboard()
{
	pFretboard = NULL;

	// create materials
	MFTexture *pFretTex = MFTexture_Find("frets");
	if(!pFretTex)
		pFretTex = MFTexture_CreateBlank("frets", MFVector::one);
	pFrets = MFMaterial_Create("frets");
	MFTexture_Release(pFretTex);

	int zRead = MFMaterial_GetParameterIndexFromName(pFrets, "zread");
	int zWrite = MFMaterial_GetParameterIndexFromName(pFrets, "zwrite");
	int additive = MFMaterial_GetParameterIndexFromName(pFrets, "additive");

	pEdge = MFMaterial_Create("edge");
	pBar = MFMaterial_Create("bar");

	pRing = MFMaterial_Create("ring");

	pColourRing[0] = MFMaterial_Create("green");
	pColourRing[1] = MFMaterial_Create("red");
	pColourRing[2] = MFMaterial_Create("yellow");
	pColourRing[3] = MFMaterial_Create("blue");
	pColourRing[4] = MFMaterial_Create("orange");

	pButtonMat[0] = MFMaterial_Create("button-green");
	pButtonMat[1] = MFMaterial_Create("button-red");
	pButtonMat[2] = MFMaterial_Create("button-yellow");
	pButtonMat[3] = MFMaterial_Create("button-blue");
	pButtonMat[4] = MFMaterial_Create("button-orange");
	pButtonRing[0] = MFMaterial_Create("button-green-ring");
	pButtonRing[1] = MFMaterial_Create("button-red-ring");
	pButtonRing[2] = MFMaterial_Create("button-yellow-ring");
	pButtonRing[3] = MFMaterial_Create("button-blue-ring");
	pButtonRing[4] = MFMaterial_Create("button-orange-ring");

	MFParticleParameters params;
	params.colour = MakeVector(1, 0, 1, 1);
	params.force.Set(0.f, 0.f, 0.f);
	params.life = .3f;
	params.size = .15f;
	params.fadeDelay = 0.2f;
	params.rotationRate = 2.f;
	params.scaleRate = -.1f;
	params.maxActiveParticles = 1000;
	params.pMaterial = "Notes/sparkles";
	pParticles = MFParticleSystem_Create(&params);

	MFParticleEmitterParameters emitter;
	emitter.position.SetTranslation(MakeVector(0, 0, 0));
	emitter.startVector.Set(0, 1.f, 0);
	emitter.pParticleSystem = pParticles;
	emitter.type = MFET_Disc;
	emitter.behaviour = MFEB_Direction;
	emitter.radius = 0.25f;
	emitter.velocity = 3.f;
	emitter.velocityScatter = 1.f;
	emitter.directionScatter = .3f;
	emitter.emitRate = 100.f;
	pEmitter = MFParticleSystem_CreateEmitter(&emitter);

	MFMaterial_SetParameterI(pFrets, zRead, 0, 0);
	MFMaterial_SetParameterI(pFrets, zWrite, 0, 0);
	MFMaterial_SetParameterI(pEdge, zRead, 0, 0);
	MFMaterial_SetParameterI(pEdge, zWrite, 0, 0);
	MFMaterial_SetParameterI(pBar, zRead, 0, 0);
	MFMaterial_SetParameterI(pBar, zWrite, 0, 0);
	MFMaterial_SetParameterI(pRing, zWrite, 0, 0);
	for(int a=0; a<5; ++a)
		MFMaterial_SetParameterI(pColourRing[a], additive, 0, 1);

	// load models
	pButton = MFModel_Create("Notes/button");
}

Fretboard::~Fretboard()
{
	// clean up models
	MFModel_Release(pButton);

	// clean up materials
	MFMaterial_Release(pEdge);
	MFMaterial_Release(pBar);

	MFMaterial_Release(pFrets);
	MFMaterial_Release(pRing);

	for(int a=0; a<5; ++a)
	{
		MFMaterial_Release(pColourRing[a]);
		MFMaterial_Release(pButtonMat[a]);
		MFMaterial_Release(pButtonRing[a]);
	}

	MFMaterial_Release(pFretboard);
}

void Fretboard::Draw(float time, dBChart *pSong, int track)
{
	MFCALLSTACKc;

	MFView_Push();

	MFRect rect;
	MFView_GetViewport(&rect);

	float aspect = rect.width / rect.height;
	aspect = MFClamp(0.82f, aspect, 2.0f);
	MFView_SetAspectRatio(aspect);

	if(viewPoint == 0)
	{
		// incoming
		MFView_ConfigureProjection(MFDEGREES(65.0f)/aspect, 1.0f, 100.0f);

		// Setup the Camera in 3D space.
		MFMatrix cameraMatrix;
		MFVector start = MakeVector( 0, 8, 3 );
		MFVector dir = MakeVector( 0, 5, -8 );
		dir = (dir-start) * (1.0f/1.777777777f);
		cameraMatrix.LookAt(start + dir*aspect, MakeVector(0.0f, 0.0f, 5.0f));
		MFView_SetCameraMatrix(cameraMatrix);
	}
	else if(viewPoint == 1)
	{
		// overhead
		MFView_ConfigureProjection(MFDEGREES(45.0f), 1.0f, 100.0f);
/*
		float aspect = MFDisplay_GetNativeAspectRatio();
		MFView_SetAspectRatio(aspect);

		MFRect projRect;
		projRect.y = 15;
		projRect.height = -30;
		projRect.x = -projRect.y * aspect;
		projRect.width = -projRect.height * aspect;
		MFView_SetOrtho(&projRect);
*/

		// Setup the Camera in 3D space.
		MFMatrix cameraMatrix;
		cameraMatrix.LookAt(MakeVector(0, 30, 10), MakeVector(0, 0, 10), MakeVector(0, 0, 1));
		MFView_SetCameraMatrix(cameraMatrix);
	}
	else if(viewPoint == 2)
	{
		// overhead
		MFView_ConfigureProjection(MFDEGREES(45.0f), 1.0f, 100.0f);
/*
		float aspect = MFDisplay_GetNativeAspectRatio();
		MFView_SetAspectRatio(aspect);

		MFRect projRect;
		projRect.y = 15;
		projRect.height = -30;
		projRect.x = -projRect.y * aspect;
		projRect.width = -projRect.height * aspect;
		MFView_SetOrtho(&projRect);
*/

		// Setup the Camera in 3D space.
		MFMatrix cameraMatrix;
		cameraMatrix.LookAt(MakeVector(0, 20, 13), MakeVector(0, 0, 13), MakeVector(-1, 0, 0));
		MFView_SetCameraMatrix(cameraMatrix);
	}

	MFView_SetProjection();

	MFMaterial *pFB = pSong->pFretboard ? pSong->pFretboard : pFretboard;
	MFMaterial_SetMaterial(pFB);
	MFPrimitive(PT_TriStrip, 0);

	int start = -4;
	int end = 60;
	int fadeStart = end - 10;

	float fretboardRepeat = 15.0f;
	float fretboardWidth = 7.0f;

	float columnWidth = fretboardWidth / 5.0f;
	float ringBorder = 0.1f;

	// draw the fretboard...
	MFBegin(((end-start) / 4) * 2 + 2);
	MFSetColourV(MFVector::white);

	float halfFB = fretboardWidth*0.5f;

	float offset = time*scrollSpeed;
	float topTime = time + end/scrollSpeed;
	float bottomTime = time + start/scrollSpeed;

	int a;
	float textureOffset = fmodf(offset, fretboardRepeat);
	for(a=start; a<end; a+=4)
	{
		float z = (float)a;
		MFSetTexCoord1(1.0f, 1.0f - (z+textureOffset) / fretboardRepeat);
		MFSetPosition(halfFB, 0.0f, z);
		MFSetTexCoord1(0.0f, 1.0f - (z+textureOffset) / fretboardRepeat);
		MFSetPosition(-halfFB, 0.0f, z);
	}

	float z = (float)a;
	MFSetTexCoord1(1.0f, 1.0f - (z+textureOffset) / fretboardRepeat);
	MFSetPosition(halfFB, 0.0f, z);
	MFSetTexCoord1(0.0f, 1.0f - (z+textureOffset) / fretboardRepeat);
	MFSetPosition(-halfFB, 0.0f, z);

	MFEnd();

	// draw the selection region
	MFMaterial_SetMaterial(pFrets);
	MFPrimitive(PT_TriStrip, 0);

	if(gEditor.selectStart != gEditor.selectEnd)
	{
		float selectStartTime = GETSECONDS(pSong->CalculateTimeOfTick(gEditor.selectStart));
		float selectEndTime = GETSECONDS(pSong->CalculateTimeOfTick(gEditor.selectEnd));

		if(selectStartTime < topTime && selectEndTime > bottomTime)
		{
			selectStartTime = (MFMax(bottomTime, selectStartTime) - time) * scrollSpeed;
			selectEndTime = (MFMin(topTime, selectEndTime) - time) * scrollSpeed;

			MFBegin(4);
			MFSetColour(1.0f, 0.0f, 0.0f, 0.5f);
			MFSetPosition(-halfFB, 0.0f, selectEndTime);
			MFSetPosition(halfFB, 0.0f, selectEndTime);
			MFSetPosition(-halfFB, 0.0f, selectStartTime);
			MFSetPosition(halfFB, 0.0f, selectStartTime);
			MFEnd();
		}
	}

	// draw the fretboard edges and bar lines
	const float barWidth = 0.2f;

	MFMaterial_SetMaterial(pBar);
	MFPrimitive(PT_TriStrip, 0);

	MFBegin(4);
	MFSetColour(0.0f, 0.0f, 0.0f, 0.8f);
	MFSetTexCoord1(0,0);
	MFSetPosition(-halfFB, 0.0f, barWidth);
	MFSetTexCoord1(1,0);
	MFSetPosition(halfFB, 0.0f, barWidth);
	MFSetTexCoord1(0,1);
	MFSetPosition(-halfFB, 0.0f, -barWidth);
	MFSetTexCoord1(1,1);
	MFSetPosition(halfFB, 0.0f, -barWidth);
	MFEnd();

	MFMaterial_SetMaterial(pEdge);
	MFPrimitive(PT_TriStrip, 0);
	MFBegin(34);

	MFSetColour(0.0f, 0.0f, 0.0f, 0.3f);
	for(int col=1; col<5; col++)
	{
		if(col > 1)
			MFSetPosition(-halfFB + columnWidth*(float)col - 0.02f, 0.0f, (float)end);

		MFSetTexCoord1(0,0);
		MFSetPosition(-halfFB + columnWidth*(float)col - 0.02f, 0.0f, (float)end);
		MFSetTexCoord1(1,0);
		MFSetPosition(-halfFB + columnWidth*(float)col + 0.02f, 0.0f, (float)end);
		MFSetTexCoord1(0,1);
		MFSetPosition(-halfFB + columnWidth*(float)col - 0.02f, 0.0f, (float)start);
		MFSetTexCoord1(1,1);
		MFSetPosition(-halfFB + columnWidth*(float)col + 0.02f, 0.0f, (float)start);

		MFSetPosition(-halfFB + columnWidth*(float)col + 0.02f, 0.0f, (float)start);
	}
	MFSetColourV(MFVector::white);
	MFSetPosition(-halfFB - 0.1f, 0.0f, (float)end);

	MFSetTexCoord1(0,0);
	MFSetPosition(-halfFB - 0.1f, 0.0f, (float)end);
	MFSetTexCoord1(1,0);
	MFSetPosition(-halfFB + 0.1f, 0.0f, (float)end);
	MFSetTexCoord1(0,1);
	MFSetPosition(-halfFB - 0.1f, 0.0f, (float)start);
	MFSetTexCoord1(1,1);
	MFSetPosition(-halfFB + 0.1f, 0.0f, (float)start);

	MFSetPosition(-halfFB + 0.1f, 0.0f, (float)start);
	MFSetPosition(halfFB - 0.1f, 0.0f, (float)end);

	MFSetTexCoord1(0,0);
	MFSetPosition(halfFB - 0.1f, 0.0f, (float)end);
	MFSetTexCoord1(1,0);
	MFSetPosition(halfFB + 0.1f, 0.0f, (float)end);
	MFSetTexCoord1(0,1);
	MFSetPosition(halfFB - 0.1f, 0.0f, (float)start);
	MFSetTexCoord1(1,1);
	MFSetPosition(halfFB + 0.1f, 0.0f, (float)start);

	MFEnd();

	// draw the frets....
	MFMaterial_SetMaterial(pBar);
	MFPrimitive(PT_TriStrip, 0);

	int bottomTick = pSong->CalculateTickAtTime((int64)(bottomTime*1000000.0f));
	int res = pSong->GetRes();
	int ticks = bHalfFrets ? res/2 : res;
	int fretBeat = bottomTick + ticks - 1;
	fretBeat -= fretBeat % ticks;
	float fretTime = GETSECONDS(pSong->CalculateTimeOfTick(fretBeat));

	while(fretTime < topTime)
	{
		bool halfBeat = (fretBeat % res) != 0;
		bool bar = false;

		if(!halfBeat)
		{
			GHEvent *pLastTS = pSong->sync.GetMostRecentEvent(GHE_TimeSignature, fretBeat);

			if(pLastTS)
				bar = ((fretBeat - pLastTS->tick) % (pLastTS->parameter*res)) == 0;
			else if(fretBeat == 0)
				bar = true;
		}

		float bw = bar ? barWidth : barWidth*0.5f;
		MFBegin(4);

		float position = (fretTime - time) * scrollSpeed;

		if(!halfBeat)
			MFSetColourV(MFVector::white);
		else
			MFSetColourV(MakeVector(1,1,1,0.3f));
		MFSetTexCoord1(0,0);
		MFSetPosition(-halfFB, 0.0f, position + bw);
		MFSetTexCoord1(1,0);
		MFSetPosition(halfFB, 0.0f, position + bw);
		MFSetTexCoord1(0,1);
		MFSetPosition(-halfFB, 0.0f, position + -bw);
		MFSetTexCoord1(1,1);
		MFSetPosition(halfFB, 0.0f, position + -bw);

		MFEnd();

		fretBeat += ticks;
		fretTime = GETSECONDS(pSong->CalculateTimeOfTick(fretBeat));
	}

	// draw the notes...
	GHEventManager &noteStream = pSong->notes[track];
	GHEvent *pEv = noteStream.First();

	int64 topTimeus = (int64)(topTime*1000000.0f);
	while(pEv && pEv->time < topTimeus)
	{
		if((pEv->event == GHE_Note || pEv->event == GHE_Special) && pEv->tick + pEv->parameter >= bottomTick)
		{
			float evTime = GETSECONDS(pEv->time);

			// TODO: we need to calculate the end of the hold...
			float noteEnd = evTime;
			if(pEv->parameter)
				noteEnd = GETSECONDS(pSong->CalculateTimeOfTick(pEv->tick + pEv->parameter));

			if(pEv->event == GHE_Note && pEv->played != 1)
			{
				// draw a note
				int key = pEv->key;
				bool tap = false;

				// check if there is a previous note, and it is in range
				if(pEv->Prev() && pEv->Prev()->tick < pEv->tick && pEv->Prev()->tick > pEv->tick - (res/2) && pEv->Prev()->key != pEv->key
					&& (!pEv->Next() || !(pEv->Next()->tick == pEv->tick))
					&& !pEv->Prev()->parameter && !(pEv->Prev()->Prev() && pEv->Prev()->Prev()->tick == pEv->Prev()->tick))
				{
					tap = true;
				}

				int noteX = gConfig.controls.leftyFlip[0] ? 4-key : key;

				float position = (GETSECONDS(pEv->time) - time)*scrollSpeed;
				float xoffset = -halfFB + columnWidth*0.5f + (float)noteX*columnWidth;

				if(pEv->parameter)
				{
					MFMaterial_SetMaterial(pFrets);

					float whammyTop = (noteEnd - time)*scrollSpeed;

					MFPrimitive(PT_TriStrip, 0);

					// TODO: we could consider not drawing this part of the hold line.. seems reasonable that it terminates at the line...
					if(gEditor.state == GHPS_Stopped)
					{
						if(position < 0.0f)
						{
							MFBegin(4);
							MFSetColourV(gColours[key]);
							MFSetPosition(xoffset - 0.2f, 0.0f, MFMin(whammyTop, 0.0f));
							MFSetPosition(xoffset + 0.2f, 0.0f, MFMin(whammyTop, 0.0f));
							MFSetPosition(xoffset - 0.2f, 0.0f, position);
							MFSetPosition(xoffset + 0.2f, 0.0f, position);
							MFEnd();
						}
					}

					if(whammyTop > 0.0f)
					{
						// this half could have waves cruising down it if we wanted to support the whammy...
						MFBegin(4);
						MFSetColourV(gColours[key]);
						MFSetPosition(xoffset - 0.2f, 0.0f, MFMin(whammyTop, (float)end));
						MFSetPosition(xoffset + 0.2f, 0.0f, MFMin(whammyTop, (float)end));
						MFSetPosition(xoffset - 0.2f, 0.0f, MFMax(position, 0.0f));
						MFSetPosition(xoffset + 0.2f, 0.0f, MFMax(position, 0.0f));
						MFEnd();
					}
				}

				if(evTime >= bottomTime)
				{
					MFMatrix mat;
					mat.SetScale(MakeVector(0.5f/20, 0.5f/20, 0.5f/20));
					mat.Translate(MakeVector(xoffset, 0.03f, position));
					MFModel_SetWorldMatrix(pButton, mat);

					MFStateBlock *pSB = MFStateBlock_CreateTemporary(64);
					MFStateBlock_SetVector(pSB, MFSCV_DiffuseColour, pEv->played == -1 ? MakeVector(0.3f, 0.3f, 0.3f, 1.0f) : MFVector::white);
//					MFStateBlock_SetVector(pSB, MFSCV_DiffuseColour, position < 0.0f ? MakeVector(0.3f, 0.3f, 0.3f, 1.0f) : MFVector::white);

//					MFRenderer_SetRenderStateOverride(MFRS_MaterialOverride, (uint32&)(tap ? pButtonMat[key] : pButtonRing[key]));
					MFRenderer_AddModel(pButton, pSB, MFView_GetViewState());

					// render the note time
					if(bRenderNoteTimes)
					{
						MFView_Push();
						MFView_SetOrtho(&rect);

						MFVector pos;
						MFView_TransformPoint3DTo2D(MakeVector(xoffset, 0.0f, position), &pos);
						pos.x += 16.0f;
						pos.y -= 26.0f;

						int minutes = (int)(pEv->time / 60000000);
						int seconds = (int)((pEv->time % 60000000) / 1000000);
						int milliseconds = (int)((pEv->time % 1000000) / 1000);
						MFFont_DrawTextf(pText, pos, 20.0f, MFVector::yellow, "%s: %d:%02d.%d\nTick: %g", MFTranslation_GetString(pStrings, TRACK_TIME), minutes, seconds, milliseconds, (float)pEv->tick/res);

						MFView_Pop();
					}
				}
			}

			if(pEv->event == GHE_Special)
			{
//				static MFVector specialColours[3] = { MakeVector(1,0,0,1), MakeVector(1,1,0,1), MakeVector(0,0,1,1) };
//				static float specialX[3] = { halfFB + 0.2f, halfFB + 1, -halfFB - 1 };
//				static float specialWidth[3] = { 0.8f, 0.8f, 0.8f };
				static MFVector specialColours[3] = { MakeVector(1,0,0,0.5f), MakeVector(1,1,0,0.5f), MakeVector(0,0,1,0.5f) };
				static float specialX[3] = { -halfFB, halfFB - 0.8f, -halfFB };
				static float specialWidth[3] = { 0.8f, 0.8f, fretboardWidth };

				float bottom = (evTime - time)*scrollSpeed;
				float top = (noteEnd - time)*scrollSpeed;

				int key = pEv->key;

				MFMaterial_SetMaterial(pFrets);

				MFPrimitive(PT_TriStrip, 0);
				MFBegin(4);
				MFSetColourV(specialColours[key]);
				MFSetPosition(specialX[key], 0.0f, MFMin(top, (float)end));
				MFSetPosition(specialX[key]+specialWidth[key], 0.0f, MFMin(top, (float)end));
				MFSetPosition(specialX[key], 0.0f, MFMax(bottom, (float)start));
				MFSetPosition(specialX[key]+specialWidth[key], 0.0f, MFMax(bottom, (float)start));
				MFEnd();
			}
		}

		pEv = pEv->Next();
	}

//	MFRenderer_SetRenderStateOverride(MFRS_MaterialOverride, NULL);

	// draw circles at the bottom..
	MFMaterial_SetMaterial(pRing);
	for(int a=0; a<5; a++)
	{
		DrawRing(-halfFB + (float)a*columnWidth + columnWidth*ringBorder, 0.0f, columnWidth*(1.0f-ringBorder*2));
	}

	for(int a=0; a<5; a++)
	{
		dBControlType keys_righty[] = { dBCtrl_Edit_Note0, dBCtrl_Edit_Note1, dBCtrl_Edit_Note2, dBCtrl_Edit_Note3, dBCtrl_Edit_Note4 };
		dBControlType keys_lefty[] = { dBCtrl_Edit_Note4, dBCtrl_Edit_Note3, dBCtrl_Edit_Note2, dBCtrl_Edit_Note1, dBCtrl_Edit_Note0 };
		dBControlType *keys = gConfig.controls.leftyFlip[0] ? keys_lefty : keys_righty;

		int ringPos = gConfig.controls.leftyFlip[0] ? 4-a : a;

		MFMaterial_SetMaterial(pColourRing[a]);
		DrawRing(-halfFB + (float)ringPos*columnWidth, 0.0f, columnWidth, !TestControl(keys[a], GHCT_Hold));
	}

	// render trigger particles
	MFParticleSystem_Draw(pParticles);

	// render text and stuff
	MFView_SetOrtho(&rect);

	pEv = pSong->sync.GetNextEvent(bottomTick);

	while(pEv && pEv->time < topTimeus)
	{
		float evTime = GETSECONDS(pEv->time);

		if(evTime > bottomTime)
		{
			if(pEv->event == GHE_BPM)
			{
				float position = (evTime - time) * scrollSpeed;

				MFVector pos;
				MFView_TransformPoint3DTo2D(MakeVector(halfFB + 0.2f, 0.0f, position), &pos);
				pos.y -= 12.0f;
				MFFont_DrawTextf(pText, pos, 24.0f, MakeVector(0,0.5f,0,1), "%s: %g", MFTranslation_GetString(pStrings, TRACK_BPM), (float)pEv->parameter * 0.001f);
			}
			if(pEv->event == GHE_Anchor)
			{
				int minutes = (int)(pEv->time / 60000000);
				int seconds = (int)((pEv->time%60000000)/1000000);
				int milliseconds = (int)((pEv->time%1000000)/1000);

				float position = (evTime - time) * scrollSpeed;

				MFVector pos;
				MFView_TransformPoint3DTo2D(MakeVector(halfFB + 0.2f, 0.0f, position), &pos);
				pos.y -= 12.0f;
				MFFont_DrawTextf(pText, pos, 24.0f, MakeVector(0,0.5f,0,1), "A: %02d:%02d.%03d\n   %s: %g", minutes, seconds, milliseconds, MFTranslation_GetString(pStrings, TRACK_BPM), (float)pEv->parameter * 0.001f);
			}
			else if(pEv->event == GHE_TimeSignature)
			{
				float position = (evTime - time) * scrollSpeed;

				MFVector pos;
				MFView_TransformPoint3DTo2D(MakeVector(-halfFB - 0.2f, 0.0f, position), &pos);
				const char *pString = MFStr("TS: %d/4", pEv->parameter);
				pos.x -= MFFont_GetStringWidth(pText, pString, 24.0f);
				pos.y -= 12.0f;
				MFFont_DrawTextf(pText, pos, 24.0f, MFVector::yellow, pString);
			}
		}

		pEv = pEv->Next();
	}

	// render events
	pEv = pSong->events.GetNextEvent(bottomTick);

	int lastChecked = -1;
	float yEventOffset = -12.0f;
	while(pEv && pEv->time < topTimeus)
	{
		float evTime = GETSECONDS(pEv->time);

		if(evTime > bottomTime)
		{
			if(pEv->event == GHE_Event)
			{
				if(lastChecked != pEv->tick)
				{
					yEventOffset = -12.0f;
					lastChecked = pEv->tick;

					if(pSong->sync.FindEvent(GHE_TimeSignature, pEv->tick, 0))
					{
						yEventOffset -= 24.0f;
					}
				}

				float position = (evTime - time) * scrollSpeed;

				MFVector pos;
				MFView_TransformPoint3DTo2D(MakeVector(-halfFB - 0.2f, 0.0f, position), &pos);

				if(!MFString_CompareN(pEv->GetString(), "section ", 8))
				{
					// draw a line across?

					pos.x -= MFFont_GetStringWidth(pText, &pEv->GetString()[8], 24.0f);
					pos.y += yEventOffset;
					MFFont_DrawTextf(pText, pos, 24.0f, MFVector::blue, &pEv->GetString()[8]);
				}
				else
				{
					pos.x -= MFFont_GetStringWidth(pText, pEv->GetString(), 24.0f);
					pos.y += yEventOffset;
					MFFont_DrawTextf(pText, pos, 24.0f, MFVector::white, pEv->GetString());
				}

				yEventOffset -= 24.0f;
			}
		}

		pEv = pEv->Next();
	}

	// render track events
	pEv = pSong->notes[track].GetNextEvent(bottomTick);

	lastChecked = -1;
	yEventOffset = -12.0f;
	while(pEv && pEv->time < topTimeus)
	{
		float evTime = GETSECONDS(pEv->time);

		if(evTime > bottomTime)
		{
			if(pEv->event == GHE_Event)
			{
				if(lastChecked != pEv->tick)
				{
					yEventOffset = -12.0f;
					lastChecked = pEv->tick;

					if(pSong->sync.FindEvent(GHE_TimeSignature, pEv->tick, 0))
					{
						yEventOffset -= 24.0f;
					}

					GHEvent *pOther = pSong->events.FindEvent(GHE_Event, pEv->tick, 0);
					while(pOther && pOther->tick == pEv->tick)
					{
						yEventOffset -= 24.0f;
						pOther = pOther->Next();
					}
				}

				float position = (evTime - time) * scrollSpeed;

				MFVector pos;
				MFView_TransformPoint3DTo2D(MakeVector(-halfFB - 0.2f, 0.0f, position), &pos);

				pos.x -= MFFont_GetStringWidth(pText, pEv->GetString(), 24.0f);
				pos.y += yEventOffset;
				MFFont_DrawTextf(pText, pos, 24.0f, MakeVector(0.6f, 0.8f, 1.0f, 1.0f), pEv->GetString());

				yEventOffset -= 24.0f;
			}
		}

		pEv = pEv->Next();
	}

	MFView_Pop();
}

void Fretboard::LoadFretboard(const char *pImage)
{
	if(pFretboard)
		MFMaterial_Release(pFretboard);

	pFretboard = MFMaterial_Create(MFStr("Fretboards/%s", pImage));

	if(pFretboard)
	{
		int zRead = MFMaterial_GetParameterIndexFromName(pFretboard, "zread");
		int zWrite = MFMaterial_GetParameterIndexFromName(pFretboard, "zwrite");
		int off = 0;

		MFMaterial_SetParameterI(pFretboard, zRead, 0, 0);
		MFMaterial_SetParameterI(pFretboard, zWrite, 0, 0);
	}
}

void Fretboard::HitNote(int note)
{
	float fretboardWidth = 7.0f;
	float columnWidth = fretboardWidth / 5.0f;
	float halfFB = fretboardWidth*0.5f;

	MFMatrix mat;
	mat.SetTranslation(MakeVector(-halfFB + (float)note*columnWidth + columnWidth*0.5f, 0.f, 0.f));

	MFParticleSystem_SetWorldMatrix(pEmitter, mat);
	MFParticleSystem_BurstEmit(pEmitter, 100);
}

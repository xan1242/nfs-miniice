#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include <stdlib.h>
#include <iup.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "iup.lib")

#include "includes\IniReader.h"
#include "includes\injector\injector.hpp"
#include "CAnimScene_Carbon.h"

void(*CameraAISetAction)(int EVIEW_ID, char* Action) = (void(*)(int, char*))0x0048D620;

#define THEANIMCHOOSEROVERRIDEADDRESS 0xA4D2FC
#define CANIMSCENEISPAUSED_ADDRESS 0x009C6000

#define LOSTFOCUSADDRESS 0xAB0B3C

void* LastCAnimScene, *PlayHookCAnimScene;

#define GETANIMSCENEENTRY 0x0045F579
unsigned int GetCAnimSceneExit = 0x0045F583;
void __declspec(naked) GetCAnimScene()
{
	//printf("Creating a CAnimScene!\n");
	_asm
	{
		call malloc
		mov LastCAnimScene, eax
		add esp, 4
		test eax, eax
		jmp GetCAnimSceneExit
	}
}

#define DESTROYANIMSCENEENTRY 0x0045E67F
void __declspec(naked) DestroyCAnimScene()
{
	_asm
	{
		push esi
		call free
		xor eax, eax
		mov LastCAnimScene, eax
		add esp, 4
		mov eax, esi
		pop esi
		retn 4
	}
}

int bDarkMode = 1;
int bRemoveSleep = 0;

char NISSceneName[256];
char NISSceneDescription[256];
unsigned int CurrentCameraTrackNum = 0;

float CurrentTimelineSliderValue = 0;

float TimeTotalLength, TimeElapsed;
int NISSceneType;

bool bTerminateMainThread = 0;
bool bSetupSlider = 0;
bool bUpdateTimeline = 0;
bool bIsPaused = 0;
bool bStartPaused = 0;
//bool bCAnimSceneExists = 0;


Ihandle *dlg, *label, *vbox, *hbox1, *hbox2, *vbox1, *vbox2, *vbox_names, *vbox_toggles, *hbox_status, *hbox_timeline, *dlg2, *dlg3;
Ihandle *playbutton, *restartbutton, *timelabel, *cameratrackbox, *overridenisbox, *overridenislabel, *cameratracklabel, *spacer, *spacer2, *vspacer, *statusbar_separator, *timeline, *playbacklabel;
Ihandle *scenelabel, *descriptionlabel, *currentcameratracklabel, *scenetypelabel, *startpausedtoggle, *vspacer2;

bool CAnimScene_Play_Hook()
{
	_asm mov PlayHookCAnimScene, ecx
	if (bStartPaused)
		return true;
	return CAnimScene_UnPause(PlayHookCAnimScene);
}

void GetNISSceneName()
{
	if (LastCAnimScene)
		AnimDirectory_GetNameOfSceneHash(*(void**)THEANIMDIRECTORY_ADDRESS, CAnimScene_GetAnimID(LastCAnimScene), NISSceneName);
}

char* GetNISSceneDescription()
{
	if (LastCAnimScene)
		// my favourite line so far
		return *(char**)((int)*(void**)((int)((*(CAnimScene*)LastCAnimScene)).CAnimSceneDataPointer + 0x10) + 0x18);
	return 0;
}


int OverrideNIS_Textbox_Callback(Ihandle *ih, int c, char *new_value)
{
	strncpy((char*)THEANIMCHOOSEROVERRIDEADDRESS, new_value, 63);
	return 0;
}

int isNumeric(const char * s)
{
	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;
	char * p;
	strtod(s, &p);
	return *p == '\0';
}

int CameraTrack_Textbox_Callback(Ihandle *ih, int c, char *new_value)
{
	int CameraTrackNumber;

	if (LastCAnimScene)
	{
		if (isNumeric(new_value))
		{
			sscanf(new_value, "%d", &CameraTrackNumber);
			(*(CAnimScene*)LastCAnimScene).CameraTrackNumber = CameraTrackNumber;
		}
	}
	return 0;
}

int RestartButton_Callback(Ihandle *ih, int c, char *new_value)
{
	if (LastCAnimScene)
		CameraAISetAction(1, "CDActionDrive"); // HACK HACK HACK
	return 0;
}

int PlayButton_Callback(Ihandle *ih, int c, char *new_value)
{
	if (LastCAnimScene)
		if (bIsPaused)
			CAnimScene_UnPause(LastCAnimScene);
			//((*(CAnimScene*)LastCAnimScene)).PlaybackMode = 2;
		else
			CAnimScene_Pause(LastCAnimScene);
			//((*(CAnimScene*)LastCAnimScene)).PlaybackMode = 1;

	return 0;
}

int StartPausedToggle_Callback(Ihandle* ih, int state)
{
	bStartPaused = state;
	return 0;
}

void __stdcall Timeline_GameSync_Callback() // the timeline movement NEEDS to be in sync with the game, otherwise crashes are going to happen!
{
	if (bUpdateTimeline && LastCAnimScene)
	{
		CAnimScene_SetTime(LastCAnimScene, CurrentTimelineSliderValue);
		bUpdateTimeline = 0;
	}
}

void __stdcall GetNISData() // synced data collection to avoid crashes
{
	if (LastCAnimScene)
	{
		CurrentCameraTrackNum = ((*(CAnimScene*)LastCAnimScene)).CameraTrackNumber;
		GetNISSceneName();
		strcpy(NISSceneDescription, GetNISSceneDescription());
		NISSceneType = CAnimScene_GetSceneType(LastCAnimScene);
		TimeTotalLength = ((*(CAnimScene*)LastCAnimScene)).TimeTotalLength;
		TimeElapsed = ((*(CAnimScene*)LastCAnimScene)).TimeElapsed;
		if (((*(CAnimScene*)LastCAnimScene)).PlaybackMode == 1)
			bIsPaused = true;
		else
			bIsPaused = false;
	}
}

void __stdcall GameLoopIntegrator()
{
	if (!bTerminateMainThread)
	{
		GetNISData();
		Timeline_GameSync_Callback();
	}
}

int Timeline_Callback(Ihandle *ih)
{

	if (LastCAnimScene)
	{
	//	printf("Setting timeline value to: %s\n", IupGetAttribute(ih, "VALUE"));
		sscanf(IupGetAttribute(ih, "VALUE"), "%f", &CurrentTimelineSliderValue);
		bUpdateTimeline = 1;
	}
	return 0;
}

void IUP_MySetup()
{
	if (bDarkMode)															// Dark mode using Visual Studio's dark color scheme.
	{
		IupSetGlobal("DLGBGCOLOR", "#2d2d30");
		IupSetGlobal("DLGFGCOLOR", "#ffffff");
		IupSetGlobal("TXTBGCOLOR", "#3f3f46");
		IupSetGlobal("TXTFGCOLOR", "#ffffff");
		IupSetGlobal("MENUBGCOLOR", "#1b1b1c");
		IupSetGlobal("MENUFGCOLOR", "#ffffff");
	}

	label = IupLabel("MiniICE not initialized.");		// Setup main label strings.
	playbacklabel = IupLabel("Waiting...");
	//statusbar_separator = IupSeparator();
	overridenislabel = IupLabel("Override NIS:");
	cameratracklabel = IupLabel("Camera Track Number:");
	scenelabel = IupLabel("Scene: ");
	currentcameratracklabel = IupLabel("CameraTrack: ");
	descriptionlabel = IupLabel("Description: ");
	scenetypelabel = IupLabel("SceneType: ");
	spacer = IupFill();
	spacer2 = IupFill();
	vspacer = IupFill();
	vspacer2 = IupFill();

	playbutton = IupFlatButton("Play / Pause");
	restartbutton = IupFlatButton("Restart");
	overridenisbox = IupText(NULL);
	cameratrackbox = IupText(NULL);

	timeline = IupVal("HORIZONTAL");
	timelabel = IupLabel("Time:");

	startpausedtoggle = IupToggle("Start paused:", NULL);

	vbox1 = IupVbox(overridenislabel, cameratracklabel, NULL);
	vbox2 = IupVbox(overridenisbox, cameratrackbox, NULL);
	vbox_names = IupVbox(scenelabel, currentcameratracklabel, descriptionlabel, scenetypelabel, NULL);
	vbox_toggles = IupVbox(startpausedtoggle, NULL);

	IupSetAttribute(vbox1, "GAP", "12");
	//IupSetAttribute(vbox1, "MARGIN", "4x2");

	hbox_status = IupHbox(label, spacer2, playbacklabel, NULL);
	IupSetAttribute(hbox_status, "GAP", "4");

	hbox1 = IupHbox(playbutton, spacer, restartbutton, NULL);
	hbox2 = IupHbox(vbox1, vbox2, NULL);
	//hbox_timeline = IupHbox(NULL);
	//IupSetAttribute(hbox1, "MARGIN", "4x2");


	vbox = IupVbox(timeline, timelabel, hbox1, hbox2, vbox_toggles, vspacer, vbox_names, vspacer2, hbox_status, NULL);  // Set dialog box elements here.
	IupSetAttribute(vbox, "MARGIN", "4x2");

	//IupSetAttribute(vbox, "GAP", "4");

	dlg = IupDialog(vbox);

	IupSetAttribute(dlg, "TITLE", "MiniICE (CB)");
	IupSetAttribute(dlg, "ICON", "IDI_ICON1");
	IupSetAttribute(dlg, "MAXBOX", "NO");
	//IupSetAttribute(dlg, "RESIZE", "NO");
	IupSetAttribute(dlg, "MINSIZE", "353x214");								// Dialog box size.
																			//IupSetAttribute(dlg, "MAXSIZE", "260x196");

	IupSetAttribute(timeline, "EXPAND", "HORIZONTAL");
	IupSetCallback(timeline, "VALUECHANGED_CB", (Icallback)Timeline_Callback);

	//IupSetAttribute(playbutton, "ALIGNMENT", "ALEFT:ALEFT");
	//IupSetAttribute(restartbutton, "ALIGNMENT", "ARIGHT:ARIGHT");

	IupSetAttribute(overridenisbox, "BGCOLOR", "255 255 255");
	IupSetAttribute(overridenisbox, "SIZE", "136x12");
	IupSetCallback(overridenisbox, "ACTION", (Icallback)OverrideNIS_Textbox_Callback);

	IupSetAttribute(cameratrackbox, "BGCOLOR", "255 255 255");
	IupSetAttribute(cameratrackbox, "SIZE", "32x12");
	IupSetCallback(cameratrackbox, "ACTION", (Icallback)CameraTrack_Textbox_Callback);

	IupSetAttribute(playbutton, "PADDING", "4x4");
	IupSetAttribute(restartbutton, "PADDING", "4x4");
	IupSetAttribute(playbutton, "BORDER", "YES");
	IupSetAttribute(restartbutton, "BORDER", "YES");

	IupSetCallback(playbutton, "FLAT_ACTION", (Icallback)PlayButton_Callback);
	IupSetCallback(restartbutton, "FLAT_ACTION", (Icallback)RestartButton_Callback);

	IupSetAttribute(startpausedtoggle, "RIGHTBUTTON", "YES");
	IupSetCallback(startpausedtoggle, "ACTION", (Icallback)StartPausedToggle_Callback);

	IupSetAttribute(label, "ALIGNMENT", "ALEFT:ABOTTOM");


	if (bDarkMode)															// Dark mode using Visual Studio's dark color scheme.
	{
		IupSetAttribute(overridenisbox, "BGCOLOR", "#3f3f46");
		IupSetAttribute(cameratrackbox, "BGCOLOR", "#3f3f46");

		IupSetAttribute(playbutton, "BORDERCOLOR", "#3399ff");
		IupSetAttribute(restartbutton, "BORDERCOLOR", "#3399ff");
		IupSetAttribute(playbutton, "BORDERPSCOLOR", "#3399ff");
		IupSetAttribute(restartbutton, "BORDERPSCOLOR", "#3399ff");
		IupSetAttribute(playbutton, "BORDERHLCOLOR", "#3399ff");
		IupSetAttribute(restartbutton, "BORDERHLCOLOR", "#3399ff");

		IupSetAttribute(playbutton, "HLCOLOR", "#3e3e40");
		IupSetAttribute(restartbutton, "HLCOLOR", "#3e3e40");
		IupSetAttribute(playbutton, "PSCOLOR", "#3399ff");
		IupSetAttribute(restartbutton, "PSCOLOR", "#3399ff");

	}
}

DWORD WINAPI IUPThread()												// The GUI thread
{
	IupOpen(NULL, NULL);													// Init IUP lib
	//IupControlsOpen();														// Init IUP controls lib

	IUP_MySetup();

	IupShowXY(dlg, IUP_LEFT, IUP_TOP);

	IupRefresh(dlg);

	IupMainLoop();

	IupClose();

	bTerminateMainThread = true;

	return EXIT_SUCCESS;
}

DWORD WINAPI MainThread(LPVOID)
{
	while (!bTerminateMainThread)
	{
		Sleep(1);

		if (LastCAnimScene) // needs optimization to reduce wasted cycles
		{
			IupSetAttribute(label, "TITLE", "MiniICE running!");

			IupSetfAttribute(scenelabel, "TITLE", "Scene: %s", NISSceneName);

			IupSetfAttribute(currentcameratracklabel, "TITLE", "CameraTrack: %d", CurrentCameraTrackNum);
			
			IupSetfAttribute(descriptionlabel, "TITLE", "Description: %s", NISSceneDescription);

			IupSetfAttribute(scenetypelabel, "TITLE", "SceneType: %d", NISSceneType);

			IupSetfAttribute(timeline, "MAX", "%.2f", TimeTotalLength);

			if (!bIsPaused)
			{
				IupSetAttribute(playbacklabel, "TITLE", "Playing");
				IupSetfAttribute(timeline, "VALUE", "%.2f", TimeElapsed);
			}
			else
				IupSetAttribute(playbacklabel, "TITLE", "Paused");
			IupSetfAttribute(timelabel, "TITLE", "Time: %.2f | Total: %.2f", TimeElapsed, TimeTotalLength);
		}
		else
		{
			IupSetAttribute(timeline, "VALUE", "0");
			IupSetAttribute(playbacklabel, "TITLE", "Stopped");

			TimeElapsed = 0;
			TimeTotalLength = 0;
			switch (*(int*)THEGAMEFLOWMANAGER_ADDRESS)
			{
			case 1:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (game loading)");
				break;
			case 2:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (loading FrontEnd)");
				break;
			case 3:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (in FrontEnd)");
				break;
			case 4:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (freeing FrontEnd)");
				break;
			case 5:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (loading world)");
				break;
			case 6:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (ingame)");
				break;
			case 7:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (unloading world)");
				break;
			case 8:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (freeing resources)");
				break;
			case 9:
				IupSetfAttribute(label, "TITLE", "MiniICE standing by (reloading FrontEnd)");
				break;

			}
		}

		IupRefresh(dlg);														// Added to avoid glitches.
	}

	return 0;
}

int Init()
{
	CIniReader inireader("");

	bDarkMode = inireader.ReadInteger("MiniICE", "DarkMode", 1);
	bRemoveSleep = inireader.ReadInteger("MiniICE", "RemoveSleep", 0);

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&MainThread, NULL, 0, NULL);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&IUPThread, NULL, 0, NULL);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		injector::MakeJMP(GETANIMSCENEENTRY, GetCAnimScene, true);
		injector::MakeJMP(DESTROYANIMSCENEENTRY, DestroyCAnimScene, true);

		// disable auto unpausing
		injector::MakeJMP(0x00494401, 0x4944A6, true);

		// disable game pausing in background
		injector::MakeJMP(0x00711F08, 0x711F12, true);

		injector::MakeCALL(0x006B7BC2, GameLoopIntegrator, true);

		injector::MakeCALL(0x0076997D, CAnimScene_Play_Hook, true);
		injector::MakeCALL(0x0045FA02, CAnimScene_Play_Hook, true);
		injector::MakeCALL(0x0045E254, CAnimScene_Play_Hook, true);

		Init();
	}
	return TRUE;
}
int main()		// Included due to IUP library. Never called.
{
	return 0;
}


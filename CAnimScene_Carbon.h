#pragma once
#define THEGAMEFLOWMANAGER_ADDRESS 0xA99BBC
#define THEANIMDIRECTORY_ADDRESS 0xA83FDC
struct Matrix4
{
	float x[4];
	float y[4];
	float z[4];
	float w[4];
};

struct CAnimScene
{
	void* CAnimSceneVTable; // +0
	unsigned int unk1; // +4
	unsigned int unk62; // +8
	unsigned int unk2; // init = 0 // +C
	void* CAnimSceneDataPointer; // +10
	void* SomePointerToThisItself; // = &this value here // +14
	void* PointerToTheValueAbove; // +18
	unsigned int PlaybackMode; // +1C // 1 = pause, 2 = play, other values unk
	float TimeElapsed; // 20
	unsigned int unk5; // 24
	float TimeTotalLength; // 28
	void* SomePointerToThisItself2; // = &this value here // 2C
	void* PointerToTheValueAbove2; // 30
	unsigned int unk7; // 4 byte value, inits only first byte // 34
	unsigned int CameraTrackNumber; // 38
	bool IsControllingCamera; // 3C
	unsigned char unk8[3];
	float SceneRotationMatrix; // 40 // matrix starts here?
	unsigned int unk10; // 44
	unsigned int unk11; // 48
	unsigned int unk12; // 4C
	unsigned int unk13; // 50
	float unk14; // 54
	unsigned int unk15; // 58
	unsigned int unk16; // 5C
	unsigned int unk17; // 60
	unsigned int unk18; // 64
	float unk19; // 68
	unsigned int unk20; // 6C
	unsigned int unk21; // 70
	unsigned int unk22; // 74
	unsigned int unk23; // 78
	float unk24; // 7C
	float unk25; // 80
	float unk26; // 84
	float unk27; // 88
	float unk28; // 8C
	float unk29; // 90
	float unk30; // 94
	float unk31; // 98
	float unk32; // 9C
	float unk33; // A0
	float unk34; // A4
	float unk35; // A8
	float unk36; // AC
	float unk37; // B0
	float unk38; // B4
	float unk39; // B8
	float unk40; // BC
	Matrix4 SceneTransformMatrix;
	/*float SceneTransformMatrix; // C0 // matrix starts here?
	float unk42; // C4
	float unk43; // C8
	float unk44; // CC
	float unk45; // D0
	float unk46; // D4
	float unk47; // D8
	float unk48; // DC
	float unk49; // E0
	float unk50; // E4
	float unk51; // E8
	float unk52; // EC
	float unk53; // F0
	float unk54; // F4
	float unk55; // F8
	float unk56; // FC*/
	/*unsigned int unk57; // 100
	unsigned int unk58; // 104
	unsigned int unk59; // 108
	unsigned int unk60;*/ // 10C
						//	unsigned int unk61; // 110
};

enum CAnimScene_ePlayStatus
{
	Stopped = 0,
	Paused = 1,
	Playing = 2,
	MaxPlayStatus = 3,
};



bool(__thiscall *CAnimScene_Pause)(void* CAnimScene) = (bool(__thiscall*)(void*))0x0045D980;
bool(__thiscall *CAnimScene_UnPause)(void* CAnimScene) = (bool(__thiscall*)(void*))0x0045D9A0;
bool(__thiscall *CAnimScene_IsPaused)(void* CAnimScene) = (bool(__thiscall*)(void*))0x0044F4D0;
bool(__thiscall *CAnimScene_Play)(void* CAnimScene) = (bool(__thiscall*)(void*))0x0045D900;
void(__thiscall *CAnimScene_SetTime)(void* CAnimScene, float time) = (void(__thiscall*)(void*, float))0x0045F200;
float(__thiscall *CAnimScene_GetTimeElapsed)(void* CAnimScene) = (float(__thiscall*)(void*))0x0045D740;
float(__thiscall *CAnimScene_GetTimeTotalLength)(void* CAnimScene) = (float(__thiscall*)(void*))0x0045D750;
unsigned int(__thiscall *CAnimScene_GetAnimID)(void* CAnimScene) = (unsigned int(__thiscall*)(void*))0x0044F410;
int(__thiscall *CAnimScene_GetSceneType)(void* CAnimScene) = (int(__thiscall*)(void*))0x0044F420;
void(__thiscall *AnimDirectory_GetNameOfSceneHash)(void* AnimDirectory, unsigned int scene_hash, char* buffer) = (void(__thiscall*)(void*, unsigned int, char*))0x0044C150;
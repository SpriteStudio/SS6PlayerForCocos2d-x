/**
*  SS5PlayerPlatform.h
*/

#ifndef SS5PlayerPlatform_h
#define SS5PlayerPlatform_h
#include "SS6Player.h"

namespace ss
{
	struct State;
	struct UserData;
	class Player;
	class CustomSprite;

	extern void SSPlatformInit(void);
	extern void SSPlatformRelese(void);
	extern void SSSetPlusDirection(int direction, int window_w, int window_h);
	extern void SSGetPlusDirection(int &direction, int &window_w, int &window_h);
    extern void SSSetPlusDirection(bool flag);
	extern unsigned char* SSFileOpen(const char* pszFileName, const char* pszMode, unsigned long * pSize);
	extern long SSTextureLoad(const char* pszFileName, int  wrapmode, int filtermode);
	extern bool SSTextureRelese(long handle);
	extern bool isAbsolutePath(const std::string& strPath);
	extern void SSRenderSetup(void);
	extern void SSDrawSprite(CustomSprite *sprite, State *overwrite_state = NULL);
	extern bool SSGetTextureSize(long handle, int &w, int &h);
	extern void SSonUserData(Player *player, UserData *userData);
	extern void SSPlayEnd(Player *player);
	extern void clearMask();
	extern void enableMask(bool flag);
	extern void execMask(CustomSprite *sprite);



};	// namespace ss

#endif

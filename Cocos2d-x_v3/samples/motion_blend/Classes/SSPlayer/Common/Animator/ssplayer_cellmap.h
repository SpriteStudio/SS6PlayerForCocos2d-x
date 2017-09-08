#ifndef __SSPLAYER_CELLMAP__
#define __SSPLAYER_CELLMAP__

#include "cocos2d.h"

namespace ss
{

/**
* SsCell
*/
struct SsCell
{
	float		pivot_X;		//原点補正
	float		pivot_Y;		//原点補正
	cocos2d::Texture2D* texture;
	cocos2d::Rect rect;
	std::string texname;
	int			cellIndex;
	std::string cellName;

	SsCell():
		  pivot_X(0)
		, pivot_Y(0)
		, texture(0)
		, cellIndex(-1)
	{}
};

///パーツが使用するセルの情報
struct SsCellValue
{
	SsRenderBlendType::_enum    blendType;
	SsCell						refCell;



	SsCellValue() :  
		blendType(SsRenderBlendType::_enum::Add)
		{}
};

};
#endif

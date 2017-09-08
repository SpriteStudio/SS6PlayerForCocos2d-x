//
//  SS5Player.cpp
//

#include "SS6Player.h"
#include "SS6PlayerData.h"
#include <string>


namespace ss
{

/**
 * definition
 */

static const ss_u32 DATA_ID = 0x42505353;
static const ss_u32 DATA_VERSION = 5;

/**
 * utilites
 */

static void splitPath(std::string& directoty, std::string& filename, const std::string& path)
{
    std::string f = path;
    std::string d = "";

    size_t pos = path.find_last_of("/");
	if (pos == std::string::npos) pos = path.find_last_of("\\");	// for win

    if (pos != std::string::npos)
    {
        d = path.substr(0, pos+1);
        f = path.substr(pos+1);
    }

	directoty = d;
	filename = f;
}


//乱数シードに利用するユニークIDを作成します。
//この値は全てのSS5プレイヤー共通で使用します
int seedMakeID = 123456;
//エフェクトに与えるシードを取得する関数
unsigned int getRandomSeed()
{
	seedMakeID++;	//ユニークIDを更新します。
	//時間＋ユニークIDにする事で毎回シードが変わるようにします。
	unsigned int rc = (unsigned int)time(0) + (seedMakeID);

	return(rc);
}



/**
 * ToPointer
 */
class ToPointer
{
public:
	explicit ToPointer(const void* base)
		: _base(static_cast<const char*>(base)) {}
	
	const void* operator()(ss_offset offset) const
	{
		return (_base + offset);
	}

private:
	const char*	_base;
};


/**
 * DataArrayReader
 */
class DataArrayReader
{
public:
	DataArrayReader(const ss_u16* dataPtr)
		: _dataPtr(dataPtr)
	{}

	ss_u16 readU16() { return *_dataPtr++; }
	ss_s16 readS16() { return static_cast<ss_s16>(*_dataPtr++); }

	unsigned int readU32()
	{
		unsigned int l = readU16();
		unsigned int u = readU16();
		return static_cast<unsigned int>((u << 16) | l);
	}

	int readS32()
	{
		return static_cast<int>(readU32());
	}

	float readFloat()
	{
		union {
			float			f;
			unsigned int	i;
		} c;
		c.i = readU32();
		return c.f;
	}
	
	void readColor(cocos2d::Color4B& color)
	{
		unsigned int raw = readU32();
		color.a = static_cast<GLubyte>(raw >> 24);
		color.r = static_cast<GLubyte>(raw >> 16);
		color.g = static_cast<GLubyte>(raw >> 8);
		color.b = static_cast<GLubyte>(raw);
	}
	
	ss_offset readOffset()
	{
		return static_cast<ss_offset>(readS32());
	}

private:
	const ss_u16*	_dataPtr;
};


/**
 * CellRef
 */
struct CellRef
{
	const Cell* cell;
	cocos2d::Texture2D* texture;
	cocos2d::Rect rect;
	std::string texname;
};


/**
 * CellCache
 */
class CellCache
{
public:

	CellCache()
	{
	}
	~CellCache()
	{
		releseReference();
	}

	static CellCache* create(const ProjectData* data, const std::string& imageBaseDir)
	{
		CellCache* obj = new CellCache();
		if (obj)
		{
			obj->init(data, imageBaseDir);
//			obj->autorelease();
		}
		return obj;
	}

	CellRef* getReference(int index)
	{
		if (index < 0 || index >= (int)_refs.size())
		{
			CCLOGERROR("Index out of range > %d", index);
			CC_ASSERT(0);
		}
		CellRef* ref = _refs.at(index);
		return ref;
	}

	//指定した名前のセルの参照テクスチャを変更する
	bool setCellRefTexture(const ProjectData* data, const char* cellName, cocos2d::Texture2D* texture)
	{
		bool rc = false;

		ToPointer ptr(data);
		const Cell* cells = static_cast<const Cell*>(ptr(data->cells));

		//名前からインデックスの取得
		int cellindex = -1;
		for (int i = 0; i < data->numCells; i++)
		{
			const Cell* cell = &cells[i];
			const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
			const char* name = static_cast<const char*>(ptr(cellMap->name));
			if (strcmp(cellName, name) == 0)
			{
				CellRef* ref = getReference(i);
				ref->texture = texture;
				rc = true;
			}
		}

		return( rc );
	}

	//指定したデータのテクスチャを破棄する
	bool releseTexture(const ProjectData* data)
	{
		bool rc = false;

		ToPointer ptr(data);
		const Cell* cells = static_cast<const Cell*>(ptr(data->cells));
		for (int i = 0; i < data->numCells; i++)
		{
			const Cell* cell = &cells[i];
			const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
			{
				CellRef* ref = _refs.at(i);
				if (ref->texture)
				{
					cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();
					texCache->removeTexture(ref->texture);
					ref->texture = nullptr;
					rc = true;
				}
			}
		}
		return(rc);
	}

	//指定したセルマップのテクスチャを取得
	cocos2d::Texture2D* getTexture(const ProjectData* data, const char* cellName)
	{
		cocos2d::Texture2D* tex = nullptr;

		ToPointer ptr(data);
		const Cell* cells = static_cast<const Cell*>(ptr(data->cells));

		//名前からインデックスの取得
		int cellindex = -1;
		for (int i = 0; i < data->numCells; i++)
		{
			const Cell* cell = &cells[i];
			const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
			const char* name = static_cast<const char*>(ptr(cellMap->name));
			if (strcmp(cellName, name) == 0)
			{
				CellRef* ref = _refs.at(i);
				//テクスチャキャッシュ内を検索する
				cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();
				tex = texCache->getTextureForKey(ref->texname);
				break;
			}
		}

		return (tex);
	}

protected:
	void init(const ProjectData* data, const std::string& imageBaseDir)
	{
		CCASSERT(data != nullptr, "Invalid data");
		
		_textures.clear();
		_refs.clear();
		_texname.clear();
		
		ToPointer ptr(data);
		const Cell* cells = static_cast<const Cell*>(ptr(data->cells));

		for (int i = 0; i < data->numCells; i++)
		{
			const Cell* cell = &cells[i];
			const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
			
			if (cellMap->index >= (int)_textures.size())
			{
				const char* imagePath = static_cast<const char*>(ptr(cellMap->imagePath));
				addTexture(imagePath, imageBaseDir, (SsTexWrapMode::_enum)cellMap->wrapmode, (SsTexFilterMode::_enum)cellMap->filtermode);
			}
			
			CellRef* ref = new CellRef();
			ref->cell = cell;
			ref->texture = _textures.at(cellMap->index);
			ref->texname = _texname.at(cellMap->index);
			ref->rect = cocos2d::Rect(cell->x, cell->y, cell->width, cell->height);
			_refs.push_back(ref);
		}
	}
	//キャッシュの削除
	void releseReference(void)
	{
		for (int i = 0; i < (int)_refs.size(); i++)
		{
			CellRef* ref = _refs.at(i);
			if (ref->texture)
			{
				cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();
				texCache->removeTexture(ref->texture);
				ref->texture = nullptr;
			}
			delete ref;
		}
		_refs.clear();
	}

	void addTexture(const std::string& imagePath, const std::string& imageBaseDir, SsTexWrapMode::_enum  wrapmode, SsTexFilterMode::_enum filtermode)
	{
		std::string path = "";
		
		if (cocos2d::FileUtils::getInstance()->isAbsolutePath(imagePath))
		{
			// 絶対パスのときはそのまま扱う
			path = imagePath;
		}
		else
		{
			// 相対パスのときはimageBaseDirを付与する
			path.append(imageBaseDir);
			size_t pathLen = path.length();
			if (pathLen && path.at(pathLen-1) != '/' && path.at(pathLen-1) != '\\')
			{
				path.append("/");
			}
			path.append(imagePath);
		}
		
		cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();
		cocos2d::Texture2D* tex = texCache->addImage(path);

		cocos2d::Texture2D::TexParams texParams;
		switch (wrapmode)
		{
		case SsTexWrapMode::clamp:	//クランプ
			texParams.wrapS = GL_CLAMP_TO_EDGE;
			texParams.wrapT = GL_CLAMP_TO_EDGE;
			break;
		case SsTexWrapMode::repeat:	//リピート
			texParams.wrapS = GL_REPEAT;
			texParams.wrapT = GL_REPEAT;
			break;
		case SsTexWrapMode::mirror:	//ミラー
			texParams.wrapS = GL_MIRRORED_REPEAT;
			texParams.wrapT = GL_MIRRORED_REPEAT;
			break;
		}
		switch (filtermode)
		{
		case SsTexFilterMode::nearlest:	//ニアレストネイバー
			texParams.minFilter = GL_NEAREST;
			texParams.magFilter = GL_NEAREST;
			break;
		case SsTexFilterMode::linear:	//リニア、バイリニア
			texParams.minFilter = GL_LINEAR;
			texParams.magFilter = GL_LINEAR;
			break;
		}
		tex->setTexParameters(texParams);

		if (tex == nullptr)
		{
			std::string msg = "Can't load image > " + path;
			CCASSERT(tex != nullptr, msg.c_str());
		}
		CCLOG("load: %s", path.c_str());
		_textures.push_back(tex);
		_texname.push_back(path);
	}


protected:
	std::vector<std::string>			_texname;
	std::vector<cocos2d::Texture2D*>	_textures;
	std::vector<CellRef*>				_refs;
};


/**
* EffectCache
*/
class EffectCache
{
public:
	EffectCache()
	{
	}
	~EffectCache()
	{
		releseReference();
	}

	static EffectCache* create(const ProjectData* data, const std::string& imageBaseDir, CellCache* cellCache)
	{
		EffectCache* obj = new EffectCache();
		if (obj)
		{
			obj->init(data, imageBaseDir, cellCache);
//			obj->autorelease();
		}
		return obj;
	}

	/**
	* エフェクトファイル名を指定してEffectRefを得る
	*/
	SsEffectModel* getReference(const std::string& name)
	{
		SsEffectModel* ref = _dic.at(name);
		return ref;
	}

	void dump()
	{
		std::map<std::string, SsEffectModel*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			CCLOG("%s", (*it).second);
			++it;
		}
	}
protected:
	void init(const ProjectData* data, const std::string& imageBaseDir, CellCache* cellCache)
	{
		CCASSERT(data != nullptr, "Invalid data");

		ToPointer ptr(data);

		//ssbpからエフェクトファイル配列を取得
		const EffectFile* effectFileArray = static_cast<const EffectFile*>(ptr(data->effectFileList));

		for (int listindex = 0; listindex < data->numEffectFileList; listindex++)
		{
			//エフェクトファイル配列からエフェクトファイルを取得
			const EffectFile* effectFile = &effectFileArray[listindex];

			//保持用のエフェクトファイル情報を作成
			SsEffectModel *effectmodel = new SsEffectModel();
			std::string effectFileName = static_cast<const char*>(ptr(effectFile->name));

			//エフェクトファイルからエフェクトノード配列を取得
			const EffectNode* effectNodeArray = static_cast<const EffectNode*>(ptr(effectFile->effectNode));
			for (int nodeindex = 0; nodeindex < effectFile->numNodeList; nodeindex++)
			{
				const EffectNode* effectNode = &effectNodeArray[nodeindex];		//エフェクトノード配列からエフェクトノードを取得

				SsEffectNode *node = new SsEffectNode();
				node->arrayIndex = effectNode->arrayIndex;
				node->parentIndex = effectNode->parentIndex;
				node->type = (SsEffectNodeType::_enum)effectNode->type;
				node->visible = true;

				SsEffectBehavior behavior;
				//セル情報を作成
				behavior.CellIndex = effectNode->cellIndex;
				CellRef* cellRef = behavior.CellIndex >= 0 ? cellCache->getReference(behavior.CellIndex) : nullptr;
				if (cellRef)
				{
					behavior.refCell.pivot_X = cellRef->cell->pivot_X;
					behavior.refCell.pivot_Y = cellRef->cell->pivot_Y;
					behavior.refCell.texture = cellRef->texture;
					behavior.refCell.texname = cellRef->texname;
					behavior.refCell.rect = cellRef->rect;
					behavior.refCell.cellIndex = behavior.CellIndex;
					std::string name = static_cast<const char*>(ptr(cellRef->cell->name));
					behavior.refCell.cellName = name;

				}
				//				behavior.CellName;
				//				behavior.CellMapName;
				behavior.blendType = (SsRenderBlendType::_enum)effectNode->blendType;

				//エフェクトノードからビヘイビア配列を取得
				const ss_offset* behaviorArray = static_cast<const ss_offset*>(ptr(effectNode->Behavior));
				for (int behaviorindex = 0; behaviorindex < effectNode->numBehavior; behaviorindex++)
				{
					//ビヘイビア配列からビヘイビアを取得
					const ss_u16* behavior_adr = static_cast<const ss_u16*>(ptr(behaviorArray[behaviorindex]));
					DataArrayReader reader(behavior_adr);

					//パラメータを作ってpush_backで登録していく
					int type = reader.readS32();
					switch (type)
					{
					case SsEffectFunctionType::Basic:
					{
						//基本情報
						EffectParticleElementBasic readparam;
						readparam.priority = reader.readU32();			//表示優先度
						readparam.maximumParticle = reader.readU32();		//最大パーティクル数
						readparam.attimeCreate = reader.readU32();		//一度に作成するパーティクル数
						readparam.interval = reader.readU32();			//生成間隔
						readparam.lifetime = reader.readU32();			//エミッター生存時間
						readparam.speedMinValue = reader.readFloat();		//初速最小
						readparam.speedMaxValue = reader.readFloat();		//初速最大
						readparam.lifespanMinValue = reader.readU32();	//パーティクル生存時間最小
						readparam.lifespanMaxValue = reader.readU32();	//パーティクル生存時間最大
						readparam.angle = reader.readFloat();				//射出方向
						readparam.angleVariance = reader.readFloat();		//射出方向範囲

						ParticleElementBasic *effectParam = new ParticleElementBasic();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->priority = readparam.priority;							//表示優先度
						effectParam->maximumParticle = readparam.maximumParticle;			//最大パーティクル数
						effectParam->attimeCreate = readparam.attimeCreate;					//一度に作成するパーティクル数
						effectParam->interval = readparam.interval;							//生成間隔
						effectParam->lifetime = readparam.lifetime;							//エミッター生存時間
						effectParam->speed.setMinMax(readparam.speedMinValue, readparam.speedMaxValue);				//初速
						effectParam->lifespan.setMinMax(readparam.lifespanMinValue, readparam.lifespanMaxValue);	//パーティクル生存時間
						effectParam->angle = readparam.angle;								//射出方向
						effectParam->angleVariance = readparam.angleVariance;				//射出方向範囲

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::RndSeedChange:
					{
						//シード上書き
						EffectParticleElementRndSeedChange readparam;
						readparam.Seed = reader.readU32();				//上書きするシード値

						ParticleElementRndSeedChange *effectParam = new ParticleElementRndSeedChange();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Seed = readparam.Seed;							//上書きするシード値

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Delay:
					{
						//発生：タイミング
						EffectParticleElementDelay readparam;
						readparam.DelayTime = reader.readU32();			//遅延時間

						ParticleElementDelay *effectParam = new ParticleElementDelay();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->DelayTime = readparam.DelayTime;			//遅延時間

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Gravity:
					{
						//重力を加える
						EffectParticleElementGravity readparam;
						readparam.Gravity_x = reader.readFloat();			//X方向の重力
						readparam.Gravity_y = reader.readFloat();			//Y方向の重力

						ParticleElementGravity *effectParam = new ParticleElementGravity();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Gravity.x = readparam.Gravity_x;			//X方向の重力
						effectParam->Gravity.y = readparam.Gravity_y;			//Y方向の重力

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Position:
					{
						//座標：生成時
						EffectParticleElementPosition readparam;
						readparam.OffsetXMinValue = reader.readFloat();	//X座標に加算最小
						readparam.OffsetXMaxValue = reader.readFloat();	//X座標に加算最大
						readparam.OffsetYMinValue = reader.readFloat();	//X座標に加算最小
						readparam.OffsetYMaxValue = reader.readFloat();	//X座標に加算最大

						ParticleElementPosition *effectParam = new ParticleElementPosition();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->OffsetX.setMinMax(readparam.OffsetXMinValue, readparam.OffsetXMaxValue); 	//X座標に加算最小
						effectParam->OffsetY.setMinMax(readparam.OffsetYMinValue, readparam.OffsetYMaxValue);	//X座標に加算最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Rotation:
					{
						//Z回転を追加
						EffectParticleElementRotation readparam;
						readparam.RotationMinValue = reader.readFloat();		//角度初期値最小
						readparam.RotationMaxValue = reader.readFloat();		//角度初期値最大
						readparam.RotationAddMinValue = reader.readFloat();	//角度初期加算値最小
						readparam.RotationAddMaxValue = reader.readFloat();	//角度初期加算値最大

						ParticleElementRotation *effectParam = new ParticleElementRotation();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Rotation.setMinMax(readparam.RotationMinValue, readparam.RotationMaxValue);		//角度初期値最小
						effectParam->RotationAdd.setMinMax(readparam.RotationAddMinValue, readparam.RotationAddMaxValue);	//角度初期加算値最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransRotation:
					{
						//Z回転速度変更
						EffectParticleElementRotationTrans readparam;
						readparam.RotationFactor = reader.readFloat();		//角度目標加算値
						readparam.EndLifeTimePer = reader.readFloat();		//到達時間

						ParticleElementRotationTrans *effectParam = new ParticleElementRotationTrans();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->RotationFactor = readparam.RotationFactor;		//角度目標加算値
						effectParam->EndLifeTimePer = readparam.EndLifeTimePer;		//到達時間

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransSpeed:
					{
						//速度：変化
						EffectParticleElementTransSpeed readparam;
						readparam.SpeedMinValue = reader.readFloat();			//速度目標値最小
						readparam.SpeedMaxValue = reader.readFloat();			//速度目標値最大

						ParticleElementTransSpeed *effectParam = new ParticleElementTransSpeed();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Speed.setMinMax(readparam.SpeedMinValue, readparam.SpeedMaxValue);			//速度目標値最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TangentialAcceleration:
					{
						//接線加速度
						EffectParticleElementTangentialAcceleration readparam;
						readparam.AccelerationMinValue = reader.readFloat();	//設定加速度最小
						readparam.AccelerationMaxValue = reader.readFloat();	//設定加速度最大

						ParticleElementTangentialAcceleration *effectParam = new ParticleElementTangentialAcceleration();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Acceleration.setMinMax(readparam.AccelerationMinValue, readparam.AccelerationMaxValue);	//設定加速度最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::InitColor:
					{
						//カラーRGBA：生成時
						EffectParticleElementInitColor readparam;
						readparam.ColorMinValue = reader.readU32();			//設定カラー最小
						readparam.ColorMaxValue = reader.readU32();			//設定カラー最大

						ParticleElementInitColor *effectParam = new ParticleElementInitColor();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						int a = (readparam.ColorMinValue & 0xFF000000) >> 24;
						int r = (readparam.ColorMinValue & 0x00FF0000) >> 16;
						int g = (readparam.ColorMinValue & 0x0000FF00) >> 8;
						int b = (readparam.ColorMinValue & 0x000000FF) >> 0;
						SsU8Color mincol(r, g, b, a);
						a = (readparam.ColorMaxValue & 0xFF000000) >> 24;
						r = (readparam.ColorMaxValue & 0x00FF0000) >> 16;
						g = (readparam.ColorMaxValue & 0x0000FF00) >> 8;
						b = (readparam.ColorMaxValue & 0x000000FF) >> 0;
						SsU8Color maxcol(r, g, b, a);
						effectParam->Color.setMinMax(mincol, maxcol);			//設定カラー最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransColor:
					{
						//カラーRGB：変化
						EffectParticleElementTransColor readparam;
						readparam.ColorMinValue = reader.readU32();			//設定カラー最小
						readparam.ColorMaxValue = reader.readU32();			//設定カラー最大

						ParticleElementTransColor *effectParam = new ParticleElementTransColor();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						int a = (readparam.ColorMinValue & 0xFF000000) >> 24;
						int r = (readparam.ColorMinValue & 0x00FF0000) >> 16;
						int g = (readparam.ColorMinValue & 0x0000FF00) >> 8;
						int b = (readparam.ColorMinValue & 0x000000FF) >> 0;
						SsU8Color mincol(r, g, b, a);
						a = (readparam.ColorMaxValue & 0xFF000000) >> 24;
						r = (readparam.ColorMaxValue & 0x00FF0000) >> 16;
						g = (readparam.ColorMaxValue & 0x0000FF00) >> 8;
						b = (readparam.ColorMaxValue & 0x000000FF) >> 0;
						SsU8Color maxcol(r, g, b, a);
						effectParam->Color.setMinMax(mincol, maxcol);			//設定カラー最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::AlphaFade:
					{
						//フェード
						EffectParticleElementAlphaFade readparam;
						readparam.disprangeMinValue = reader.readFloat();		//表示区間開始
						readparam.disprangeMaxValue = reader.readFloat();		//表示区間終了

						ParticleElementAlphaFade *effectParam = new ParticleElementAlphaFade();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->disprange.setMinMax(readparam.disprangeMinValue, readparam.disprangeMaxValue);		//表示区間開始

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Size:
					{
						//スケール：生成時
						EffectParticleElementSize readparam;
						readparam.SizeXMinValue = reader.readFloat();			//幅倍率最小
						readparam.SizeXMaxValue = reader.readFloat();			//幅倍率最大
						readparam.SizeYMinValue = reader.readFloat();			//高さ倍率最小
						readparam.SizeYMaxValue = reader.readFloat();			//高さ倍率最大
						readparam.ScaleFactorMinValue = reader.readFloat();		//倍率最小
						readparam.ScaleFactorMaxValue = reader.readFloat();		//倍率最大

						ParticleElementSize *effectParam = new ParticleElementSize();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->SizeX.setMinMax(readparam.SizeXMinValue, readparam.SizeXMaxValue);			//幅倍率最小
						effectParam->SizeY.setMinMax(readparam.SizeYMinValue, readparam.SizeYMaxValue);			//高さ倍率最小
						effectParam->ScaleFactor.setMinMax(readparam.ScaleFactorMinValue, readparam.ScaleFactorMaxValue);		//倍率最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransSize:
					{
						//スケール：変化
						EffectParticleElementTransSize readparam;
						readparam.SizeXMinValue = reader.readFloat();			//幅倍率最小
						readparam.SizeXMaxValue = reader.readFloat();			//幅倍率最大
						readparam.SizeYMinValue = reader.readFloat();			//高さ倍率最小
						readparam.SizeYMaxValue = reader.readFloat();			//高さ倍率最大
						readparam.ScaleFactorMinValue = reader.readFloat();		//倍率最小
						readparam.ScaleFactorMaxValue = reader.readFloat();		//倍率最大

						ParticleElementTransSize *effectParam = new ParticleElementTransSize();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->SizeX.setMinMax(readparam.SizeXMinValue, readparam.SizeXMaxValue);			//幅倍率最小
						effectParam->SizeY.setMinMax(readparam.SizeYMinValue, readparam.SizeYMaxValue);			//高さ倍率最小
						effectParam->ScaleFactor.setMinMax(readparam.ScaleFactorMinValue, readparam.ScaleFactorMaxValue);		//倍率最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::PointGravity:
					{
						//重力点の追加
						EffectParticlePointGravity readparam;
						readparam.Position_x = reader.readFloat();				//重力点X
						readparam.Position_y = reader.readFloat();				//重力点Y
						readparam.Power = reader.readFloat();					//パワー

						ParticlePointGravity *effectParam = new ParticlePointGravity();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Position.x = readparam.Position_x;				//重力点X
						effectParam->Position.y = readparam.Position_y;				//重力点Y
						effectParam->Power = readparam.Power;					//パワー

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TurnToDirectionEnabled:
					{
						//進行方向に向ける
						EffectParticleTurnToDirectionEnabled readparam;
						readparam.Rotation = reader.readFloat();					//フラグ

						ParticleTurnToDirectionEnabled *effectParam = new ParticleTurnToDirectionEnabled();
						effectParam->Rotation = readparam.Rotation;
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::InfiniteEmitEnabled:
					{
						EffectParticleInfiniteEmitEnabled readparam;
						readparam.flag = reader.readS32();					//フラグ

						ParticleInfiniteEmitEnabled *effectParam = new ParticleInfiniteEmitEnabled();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					default:
						break;
					}
				}
				node->behavior = behavior;
				effectmodel->nodeList.push_back(node);
				if (nodeindex == 0)
				{
				}
			}
			//ツリーの構築
			if (effectmodel->nodeList.size() > 0)
			{
				effectmodel->root = effectmodel->nodeList[0];	//rootノードを追加
				for (size_t i = 1; i < effectmodel->nodeList.size(); i++)
				{
					int pi = effectmodel->nodeList[i]->parentIndex;
					if (pi >= 0)
					{
						effectmodel->nodeList[pi]->addChildEnd(effectmodel->nodeList[i]);
					}
				}
			}
			effectmodel->lockRandSeed = effectFile->lockRandSeed; 	 // ランダムシード固定値
			effectmodel->isLockRandSeed = effectFile->isLockRandSeed;  // ランダムシードを固定するか否か
			effectmodel->fps = effectFile->fps;             //
			effectmodel->effectName = effectFileName;
			effectmodel->layoutScaleX = effectFile->layoutScaleX;	//レイアウトスケールX
			effectmodel->layoutScaleY = effectFile->layoutScaleY;	//レイアウトスケールY


			CCLOG("effect key: %s", effectFileName.c_str());
			_dic.insert(std::map<std::string, SsEffectModel*>::value_type(effectFileName, effectmodel));
		}
	}
	//エフェクトファイル情報の削除
	void releseReference(void)
	{
		std::map<std::string, SsEffectModel*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			SsEffectModel* effectmodel = it->second;

			if (effectmodel)
			{
				for (int nodeindex = 0; nodeindex < (int)effectmodel->nodeList.size(); nodeindex++)
				{
					SsEffectNode* node = effectmodel->nodeList.at(nodeindex);
					for (int behaviorindex = 0; behaviorindex < (int)node->behavior.plist.size(); behaviorindex++)
					{
						SsEffectElementBase* eb = node->behavior.plist.at(behaviorindex);
						delete eb;
					}
					node->behavior.plist.clear();
				}
				if (effectmodel->nodeList.size() > 0)
				{
					SsEffectNode* node = effectmodel->nodeList.at(0);
					delete node;
					effectmodel->nodeList.clear();
				}
				effectmodel->root = 0;

			}
			delete effectmodel;
			it++;
		}
		_dic.clear();
	}
protected:
	std::map<std::string, SsEffectModel*>		_dic;
};


/**
 * AnimeRef
 */
struct AnimeRef
{
	std::string				packName;
	std::string				animeName;
	const AnimationData*	animationData;
	const AnimePackData*	animePackData;
};


/**
 * AnimeCache
 */
class AnimeCache
{
public:
	AnimeCache()
	{
	}
	~AnimeCache()
	{
		releseReference();
	}

	static AnimeCache* create(const ProjectData* data)
	{
		AnimeCache* obj = new AnimeCache();
		if (obj)
		{
			obj->init(data);
//			obj->autorelease();
		}
		return obj;
	}

	/**
	 * packNameとanimeNameを指定してAnimeRefを得る
	 */
	AnimeRef* getReference(const std::string& packName, const std::string& animeName)
	{
		std::string key = toPackAnimeKey(packName, animeName);
		AnimeRef* ref = _dic.at(key);
		return ref;
	}

	/**
	 * animeNameのみ指定してAnimeRefを得る
	 */
	AnimeRef* getReference(const std::string& animeName)
	{
		AnimeRef* ref = _dic.at(animeName);
		return ref;
	}
	
	void dump()
	{
		std::map<std::string, AnimeRef*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			CCLOG("%s", (*it).second);
			++it;
		}
	}

protected:
	void init(const ProjectData* data)
	{
		CCASSERT(data != nullptr, "Invalid data");
		
		ToPointer ptr(data);
		const AnimePackData* animePacks = static_cast<const AnimePackData*>(ptr(data->animePacks));

		for (int packIndex = 0; packIndex < data->numAnimePacks; packIndex++)
		{
			const AnimePackData* pack = &animePacks[packIndex];
			const AnimationData* animations = static_cast<const AnimationData*>(ptr(pack->animations));
			const char* packName = static_cast<const char*>(ptr(pack->name));
			
			for (int animeIndex = 0; animeIndex < pack->numAnimations; animeIndex++)
			{
				const AnimationData* anime = &animations[animeIndex];
				const char* animeName = static_cast<const char*>(ptr(anime->name));
				
				AnimeRef* ref = new AnimeRef();
				ref->packName = packName;
				ref->animeName = animeName;
				ref->animationData = anime;
				ref->animePackData = pack;

				// packName + animeNameでの登録
				std::string key = toPackAnimeKey(packName, animeName);
				CCLOG("anime key: %s", key.c_str());
				_dic.insert(std::map<std::string, AnimeRef*>::value_type(key, ref));

				// animeNameのみでの登録
//				_dic.insert(std::map<std::string, AnimeRef*>::value_type(animeName, ref));
			}
		}
	}

	static std::string toPackAnimeKey(const std::string& packName, const std::string& animeName)
	{
		return cocos2d::StringUtils::format("%s/%s", packName.c_str(), animeName.c_str());
	}

	//キャッシュの削除
	void releseReference(void)
	{
		std::map<std::string, AnimeRef*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			AnimeRef* ref = it->second;
			if (ref)
			{
				delete ref;
				it->second = 0;
			}
			it++;
		}
		_dic.clear();
	}

protected:
	std::map<std::string, AnimeRef*>	_dic;
};





/**
 * ResourceSet
 */
struct ResourceSet
{
	const ProjectData* data;
	bool isDataAutoRelease;
	EffectCache* effectCache;
	CellCache* cellCache;
	AnimeCache* animeCache;

	virtual ~ResourceSet()
	{
		if (isDataAutoRelease)
		{
			delete data;
			data = NULL;
		}
		if (animeCache)
		{
			delete animeCache;
			animeCache = NULL;
		}
		if (cellCache)
		{
			delete cellCache;
			cellCache = NULL;
		}
		if (effectCache)
		{
			delete effectCache;
			effectCache = NULL;
		}
	}
};


/**
 * ResourceManager
 */

static ResourceManager* defaultInstance = nullptr;
const std::string ResourceManager::s_null;

ResourceManager* ResourceManager::getInstance()
{
	if (!defaultInstance)
	{
		defaultInstance = ResourceManager::create();
		defaultInstance->retain();
	}
	return defaultInstance;
}

ResourceManager::ResourceManager(void)
{
}

ResourceManager::~ResourceManager()
{
	removeAllData();
}

ResourceManager* ResourceManager::create()
{
	ResourceManager* obj = new ResourceManager();
	if (obj)
	{
		obj->autorelease();
	}
	return obj;
}

ResourceSet* ResourceManager::getData(const std::string& dataKey)
{
	ResourceSet* rs = NULL;
	if (_dataDic.find(dataKey) != _dataDic.end())
	{
		rs = _dataDic.at(dataKey);
	}
	CCAssert(rs != NULL, "Invalid data");
	return rs;
}

std::string ResourceManager::addData(const std::string& dataKey, const ProjectData* data, const std::string& imageBaseDir)
{
    CCASSERT(data != nullptr, "Invalid data");
	CCASSERT(data->dataId == DATA_ID, "Not data id matched");
	CCASSERT(data->version == DATA_VERSION, "Version number of data does not match");
	
	// imageBaseDirの指定がないときコンバート時に指定されたパスを使用する
	std::string baseDir = imageBaseDir;
	if (imageBaseDir == s_null && data->imageBaseDir)
	{
		ToPointer ptr(data);
		const char* dir = static_cast<const char*>(ptr(data->imageBaseDir));
		baseDir = dir;
	}

	//アニメはエフェクトを参照し、エフェクトはセルを参照するのでこの順番で生成する必要がある
	CellCache* cellCache = CellCache::create(data, baseDir);

	EffectCache* effectCache = EffectCache::create(data, baseDir, cellCache);	//

	AnimeCache* animeCache = AnimeCache::create(data);


	ResourceSet* rs = new ResourceSet();
	rs->data = data;
	rs->isDataAutoRelease = false;
	rs->cellCache = cellCache;
	rs->effectCache = effectCache;
	rs->animeCache = animeCache;
	_dataDic.insert(std::map<std::string, ResourceSet*>::value_type(dataKey, rs));

	return dataKey;
}

std::string ResourceManager::addDataWithKey(const std::string& dataKey, const std::string& ssbpFilepath, const std::string& imageBaseDir)
{
	std::string fullpath = cocos2d::FileUtils::getInstance()->fullPathForFilename(ssbpFilepath);

	ssize_t nSize = 0;
	void* loadData = cocos2d::FileUtils::getInstance()->getFileData(fullpath, "rb", &nSize);
	if (loadData == nullptr)
	{
		std::string msg = "Can't load project data > " + fullpath;
		CCASSERT(loadData != nullptr, msg.c_str());
	}
	
	const ProjectData* data = static_cast<const ProjectData*>(loadData);
	CCASSERT(data->dataId == DATA_ID, "Not data id matched");
	CCASSERT(data->version == DATA_VERSION, "Version number of data does not match");
	
	std::string baseDir = imageBaseDir;
	if (imageBaseDir == s_null)
	{
		// imageBaseDirの指定がないとき
		if (data->imageBaseDir)
		{
			// コンバート時に指定されたパスを使用する
			ToPointer ptr(data);
			const char* dir = static_cast<const char*>(ptr(data->imageBaseDir));
			baseDir = dir;
		}
		else
		{
			// プロジェクトファイルと同じディレクトリを指定する
			std::string directory;
			std::string filename;
			splitPath(directory, filename, ssbpFilepath);
			baseDir = directory;
		}
		//CCLOG("imageBaseDir: %s", baseDir.c_str());
	}

	addData(dataKey, data, baseDir);
	
	// リソースが破棄されるとき一緒にロードしたデータも破棄する
	ResourceSet* rs = getData(dataKey);
	CCASSERT(rs != nullptr, "");
	rs->isDataAutoRelease = true;
	
	return dataKey;
}

std::string ResourceManager::addData(const std::string& ssbpFilepath, const std::string& imageBaseDir)
{
	// ファイル名を取り出す
	std::string directory;
    std::string filename;
	splitPath(directory, filename, ssbpFilepath);
	
	// 拡張子を取る
	std::string dataKey = filename;
	size_t pos = filename.find_last_of(".");
    if (pos != std::string::npos)
    {
        dataKey = filename.substr(0, pos);
    }
	
	//登録されている名前か判定する
	std::map<std::string, ResourceSet*>::iterator it = _dataDic.find(dataKey);
	if (it != _dataDic.end())
	{
		//登録されている場合は処理を行わない
		std::string str = "";
		return str;
	}


	return addDataWithKey(dataKey, ssbpFilepath, imageBaseDir);
}

//バイナリデータの解放
void ResourceManager::removeData(const std::string& ssbpName)
{
	ResourceSet* rs = getData(ssbpName);

	//バイナリデータの削除
	delete rs;
	_dataDic.erase(ssbpName);
}

void ResourceManager::removeAllData()
{
	//全リソースの解放
	while (!_dataDic.empty())
	{
		std::map<std::string, ResourceSet*>::iterator it = _dataDic.begin();
		std::string ssbpName = it->first;
		removeData(ssbpName);
	}
	_dataDic.clear();
}

//データ名、セル名を指定して、セルで使用しているテクスチャを変更する
bool ResourceManager::changeTexture(char* ssbpName, char* ssceName, cocos2d::Texture2D* texture)
{
	bool rc = false;

	ResourceSet* rs = getData(ssbpName);
	rc = rs->cellCache->setCellRefTexture(rs->data, ssceName, texture);

	return( rc );
}

//セルとして読み込んだテクスチャを取得する
cocos2d::Texture2D* ResourceManager::getTexture(char* ssbpName, char* ssceName)
{
	cocos2d::Texture2D* tex = nullptr;

	ResourceSet* rs = getData(ssbpName);
	tex = rs->cellCache->getTexture(rs->data, ssceName);

	return(tex);
}

//アニメーションの開始フレーム数を取得する
int ResourceManager::getStartFrame(std::string ssbpName, std::string animeName)
{
	int rc = -1;

	ResourceSet* rs = getData(ssbpName);
	AnimeRef* animeRef = rs->animeCache->getReference(animeName);
	if (animeRef == NULL)
	{
		std::string msg = cocos2d::StringUtils::format("Not found animation > anime=%s", animeName.c_str());
		CCASSERT(animeRef != NULL, msg.c_str());
	}
	rc = animeRef->animationData->startFrames;

	return(rc);
}

//アニメーションの終了フレーム数を取得する
int ResourceManager::getEndFrame(std::string ssbpName, std::string animeName)
{
	int rc = -1;

	ResourceSet* rs = getData(ssbpName);
	AnimeRef* animeRef = rs->animeCache->getReference(animeName);
	if (animeRef == NULL)
	{
		std::string msg = cocos2d::StringUtils::format("Not found animation > anime=%s", animeName.c_str());
		CCASSERT(animeRef != NULL, msg.c_str());
	}
	rc = animeRef->animationData->endFrames;

	return(rc);
}

//アニメーションの総フレーム数を取得する
int ResourceManager::getTotalFrame(std::string ssbpName, std::string animeName)
{
	int rc = -1;

	ResourceSet* rs = getData(ssbpName);
	AnimeRef* animeRef = rs->animeCache->getReference(animeName);
	if (animeRef == NULL)
	{
		std::string msg = cocos2d::StringUtils::format("Not found animation > anime=%s", animeName.c_str());
		CCASSERT(animeRef != NULL, msg.c_str());
	}
	rc = animeRef->animationData->totalFrames;

	return(rc);
}

//ssbpファイルが登録されているかを調べる
bool ResourceManager::isDataKeyExists(const std::string& dataKey) {
	// 登録されている名前か判定する
	std::map<std::string, ResourceSet*>::iterator it = _dataDic.find(dataKey);
	if (it != _dataDic.end()) {
		//登録されている
		return true;
	}

	return false;
}


/**
* SSPManager
*/
static SSPManager* defaultSSPManegerInstance = nullptr;
SSPManager* SSPManager::getInstance()
{
	if (!defaultSSPManegerInstance)
	{
		defaultSSPManegerInstance = SSPManager::create();
		defaultSSPManegerInstance->retain();
	}
	return defaultSSPManegerInstance;
}

SSPManager::SSPManager(void)
{
	//エフェクトバッファの作成
	_effectSpriteCount = 0;
	_effectSprite.clear();		//デストラクタのみ行う
	_isUpdate = true;
	_useOffscreenRendering = false;
}

SSPManager::~SSPManager()
{
	releseEffectBuffer();
}

SSPManager* SSPManager::create()
{
	SSPManager* obj = new SSPManager();
	if (obj)
	{
		obj->autorelease();
	}
	return obj;
}

void SSPManager::createEffectBuffer(int buffSize)
{
	//エフェクトバッファの解放
	releseEffectBuffer();
		
	//エフェクト用パーツ生成
	if (_effectSprite.size() == 0)
	{
		for (auto i = 0; i < buffSize; i++)
		{
			CustomSprite* sprite = CustomSprite::create();
			sprite->_parent = nullptr;
			sprite->setVisible(false);

			_effectSprite.pushBack(sprite);
		}
	}
}

void SSPManager::releseEffectBuffer()
{
	//エフェクトバッファの解放
	int i;
	for (i = 0; i < _effectSprite.size(); i++)	//
	{
		CustomSprite *sp = _effectSprite.at(i);
		sp->removeFromParentAndCleanup(true);
	}
	_effectSpriteCount = 0;
	_effectSprite.clear();
}

//各シーンのアップデートで必ず呼び出してください
void SSPManager::update()
{
	// エフェクトのアップデート
	if (_isUpdate == true)
	{
		//スプライトをすべて非表示にする
		int i = 0;
		for (i = 0; i < _effectSpriteCount; i++)	//前回更新した分だけ初期化する
		{
			CustomSprite *sp = _effectSprite.at(i);
			sp->setVisible(false);
			sp->removeFromParentAndCleanup(false);
		}
		_effectSpriteCount = 0;
	}
	_isUpdate = false;
}

//この関数はプレイヤー内部で使用します。ゲームから直接呼び出しません。
CustomSprite* SSPManager::getEffectBuffer()
{
	CustomSprite* sp = 0;
	if ((_effectSprite.size() > 0) && (_effectSpriteCount < _effectSprite.size()))
	{
		sp = _effectSprite.at(_effectSpriteCount);
		_effectSpriteCount++;
	}

	return(sp);
}

//この関数はプレイヤー内部で使用します。ゲームから直接呼び出しません。
void SSPManager::setUpdateFlag()
{
	if (_useOffscreenRendering == false)
	{
		_isUpdate = true;
	}
}
void SSPManager::setUseOffscreenRendering( bool use)
{
	_useOffscreenRendering = use;
}

/**
 * Player
 */

static const std::string s_nullString;

Player::Player(void)
	: _resman(nullptr)
	, _currentRs(nullptr)
	, _currentAnimeRef(nullptr)

	, _frameSkipEnabled(true)
	, _playingFrame(0.0f)
	, _step(1.0f)
	, _loop(0)
	, _loopCount(0)
	, _isPlaying(false)
	, _isPausing(false)
	, _prevDrawFrameNo(-1)
	, _InstanceAlpha(255)
	, _InstanceRotX(0.0f)
	, _InstanceRotY(0.0f)
	, _InstanceRotZ(0.0f)
	, _isContentScaleFactorAuto(true)
	, _col_r(255)
	, _col_g(255)
	, _col_b(255)
	, _instanceOverWrite(false)
	, _offScreentexture(nullptr)
	, _userDataCallback(nullptr)
	, _playEndCallback(nullptr)
	, _offScreenWidth(0)
	, _offScreenHeight(0)
	, _offScreenPivotX(0.5f)
	, _offScreenPivotY(0.5f)
	, _motionBlendPlayer(NULL)
	, _blendTime(0.0f)
	, _blendTimeMax(0.0f)
	, _startFrameOverWrite(-1)		//開始フレームの上書き設定
	, _endFrameOverWrite(-1)		//終了フレームの上書き設定
	, _seedOffset(0)
	, _maskFuncFlag(true)
	, _maskParentSetting(true)
	, _parentMatUse(false)					//プレイヤーが持つ継承されたマトリクスがあるか？
{
	int i;
	for (i = 0; i < PART_VISIBLE_MAX; i++)
	{
		_partVisible[i] = true;
		_partIndex[i] = -1;
		_cellChange[i] = -1;
	}
	_instanseParam.clear();
	_parentMat = cocos2d::Mat4::IDENTITY;
}

Player::~Player()
{
	this->unscheduleUpdate();
	releaseParts();
	releaseData();
	releaseResourceManager();
}

Player* Player::create(ResourceManager* resman)
{
	Player* obj = new Player();
	if (obj && obj->init())
	{
		obj->setResourceManager(resman);
		obj->setSSPManager();
		obj->autorelease();
		obj->scheduleUpdate();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}

bool Player::init()
{
    if (!cocos2d::Sprite::init())
    {
        return false;
    }
	return true;
}

void Player::releaseResourceManager()
{
	CC_SAFE_RELEASE_NULL(_resman);
}

void Player::setResourceManager(ResourceManager* resman)
{
	if (_resman) releaseResourceManager();
	
	if (!resman)
	{
		// nullのときはデフォルトを使用する
		resman = ResourceManager::getInstance();
	}
	
	CC_SAFE_RETAIN(resman);
	_resman = resman;
}

void Player::setSSPManager()
{
	SSPManager* sspman = SSPManager::getInstance();
	_sspman = sspman;
}

int Player::getStartFrame() const
{
	if (_currentAnimeRef)
	{
		return(_currentAnimeRef->animationData->startFrames);
	}
	else
	{
		return(0);
	}

}
int Player::getEndFrame() const
{
	if (_currentAnimeRef)
	{
		return(_currentAnimeRef->animationData->endFrames);
	}
	else
	{
		return(0);
	}

}
int Player::getTotalFrame() const
{
	if (_currentAnimeRef)
	{
		return(_currentAnimeRef->animationData->totalFrames);
	}
	else
	{
		return(0);
	}

}

int Player::getFrameNo() const
{
	return static_cast<int>(_playingFrame);
}

void Player::setFrameNo(int frameNo)
{
	if (_currentAnimeRef)
	{
		_playingFrame = (float)frameNo;
		if (_playingFrame < _currentAnimeRef->animationData->startFrames)
		{
			_playingFrame = _currentAnimeRef->animationData->startFrames;
		}
		if (_playingFrame > _currentAnimeRef->animationData->endFrames)
		{
			_playingFrame = _currentAnimeRef->animationData->endFrames;
		}
	}
}

float Player::getStep() const
{
	return _step;
}

void Player::setStep(float step)
{
	_step = step;
}

int Player::getLoop() const
{
	return _loop;
}

void Player::setLoop(int loop)
{
	if (loop < 0) return;
	_loop = loop;
}

int Player::getLoopCount() const
{
	return _loopCount;
}

void Player::clearLoopCount()
{
	_loopCount = 0;
}

void Player::setFrameSkipEnabled(bool enabled)
{
	_frameSkipEnabled = enabled;
	_playingFrame = (int)_playingFrame;
}

bool Player::isFrameSkipEnabled() const
{
	return _frameSkipEnabled;
}

void Player::setUserDataCallback(const UserDataCallback& callback)
{
	_userDataCallback = callback;
}

void Player::setPlayEndCallback(const PlayEndCallback& callback)
{
	_playEndCallback = callback;
}


void Player::setData(const std::string& dataKey)
{
	ResourceSet* rs = _resman->getData(dataKey);
	_currentdataKey = dataKey;
	if (rs == nullptr)
	{
		std::string msg = cocos2d::StringUtils::format("Not found data > %s", dataKey.c_str());
		CCASSERT(rs != nullptr, msg.c_str());
	}
	
	if (_currentRs != rs)
	{
//		releaseData();
//		rs->retain();
		_currentRs = rs;
	}
}

void Player::releaseData()
{
	releaseAnime();
//	CC_SAFE_RELEASE_NULL(_currentRs);
}


void Player::releaseAnime()
{
	releaseParts();
//	CC_SAFE_RELEASE_NULL(_currentAnimeRef);
}

void Player::play(const std::string& ssaeName, const std::string& motionName, int loop, int startFrameNo)
{
	auto animeName = cocos2d::StringUtils::format("%s/%s", ssaeName.c_str(), motionName.c_str());
	play(animeName, loop,startFrameNo);
}

void Player::play(const std::string& animeName, int loop, int startFrameNo)
{
	CCASSERT(_currentRs != nullptr, "Not select data");

	AnimeRef* animeRef = _currentRs->animeCache->getReference(animeName);
	if (animeRef == nullptr)
	{
		auto msg = cocos2d::StringUtils::format("Not found animation > anime=%s", animeName.c_str());
		CCASSERT(animeRef != nullptr, msg.c_str());
	}
	_currentAnimename = animeName;

	play(animeRef, loop, startFrameNo);
}

void Player::play(AnimeRef* animeRef, int loop, int startFrameNo)
{
	if (_currentAnimeRef != animeRef)
	{
//		CC_SAFE_RELEASE_NULL(_currentAnimeRef);
//		animeRef->retain();

		_currentAnimeRef = animeRef;
		
		allocParts(animeRef->animePackData->numParts, false);
		setPartsParentage();
	}
	_playingFrame = static_cast<float>(startFrameNo);
	_step = 1.0f;
	_loop = loop;
	_loopCount = 0;
	_isPlaying = true;
	_isPausing = false;
	_prevDrawFrameNo = -1;
	_isPlayFirstUserdataChack = true;
	_isPlayFirstUpdate = true;
	_animefps = _currentAnimeRef->animationData->fps;
	setStartFrame(-1);
	setEndFrame(-1);

	setFrame((int)_playingFrame);
}

//モーションブレンドしつつ再生
void Player::motionBlendPlay(const std::string& animeName, int loop, int startFrameNo, float blendTime)
{
	if (_currentAnimename != "")
	{
		//現在のアニメーションをブレンド用プレイヤーで再生
		if (_motionBlendPlayer == NULL)
		{
			_motionBlendPlayer = ss::Player::create();
			addChild(_motionBlendPlayer);
		}
		int loopnum = _loop;
		if (_loop > 0)
		{
			loopnum = _loop - _loopCount;
		}
		_motionBlendPlayer->setData(_currentdataKey);        // ssbpファイル名（拡張子不要）
		_motionBlendPlayer->play(_currentAnimename, loopnum, getFrameNo());
		_motionBlendPlayer->setStep(_step);
		_motionBlendPlayer->setVisible(false);

		if (_loop > 0)
		{
			if (_loop == _loopCount)	//アニメは最後まで終了している
			{
				_motionBlendPlayer->animePause();
			}
		}
		_blendTime = 0;
		_blendTimeMax = blendTime;

	}
	play(animeName, loop, startFrameNo);

}

void Player::animePause()
{
	_isPausing = true;
}

void Player::animeResume()
{
	_isPausing = false;
}

void Player::stop()
{
	_isPlaying = false;
}

const std::string& Player::getPlayPackName() const
{
	return _currentAnimeRef != nullptr ? _currentAnimeRef->packName : s_nullString;
}

const std::string& Player::getPlayAnimeName() const
{
	return _currentAnimeRef != nullptr ? _currentAnimeRef->animeName : s_nullString;
}


void Player::update(float dt)
{
	//SSPlayerの定時処理
	_sspman->update();

	updateFrame(dt);
}

void Player::updateFrame(float dt)
{
	if (!_currentAnimeRef) return;
	if (!_currentRs->data) return;

	int startFrame = _currentAnimeRef->animationData->startFrames;
	int endFrame = _currentAnimeRef->animationData->endFrames;
	if (_startFrameOverWrite != -1)
	{
		startFrame = _startFrameOverWrite;
	}
	if (_endFrameOverWrite != -1)
	{
		endFrame = _endFrameOverWrite;
	}
	CCASSERT(startFrame < endFrame, "Playframe is out of range.");

	bool playEnd = false;
	bool toNextFrame = _isPlaying && !_isPausing;
	if (toNextFrame && (_loop == 0 || _loopCount < _loop))
	{
		// フレームを進める.
		// forward frame.
		const int numFrames = endFrame;

		float fdt = _frameSkipEnabled ? dt : cocos2d::Director::getInstance()->getAnimationInterval();
		float s = fdt / (1.0f / _currentAnimeRef->animationData->fps);
		
		//if (!m_frameSkipEnabled) CCLOG("%f", s);
		
		float next = _playingFrame + (s * _step);

		int nextFrameNo = static_cast<int>(next);
		float nextFrameDecimal = next - static_cast<float>(nextFrameNo);
		int currentFrameNo = static_cast<int>(_playingFrame);
		
		//playを行って最初のupdateでは現在のフレームのユーザーデータを確認する
		if (_isPlayFirstUserdataChack == true )
		{
			checkUserData(currentFrameNo);
			_isPlayFirstUserdataChack = false;
		}

		if (_step >= 0)
		{
			// 順再生時.
			// normal plays.
			for (int c = nextFrameNo - currentFrameNo; c; c--)
			{
				int incFrameNo = currentFrameNo + 1;
				if (incFrameNo > numFrames)
				{
					// アニメが一巡
					// turned animation.
					_loopCount += 1;
					if (_loop && _loopCount >= _loop)
					{
						// 再生終了.
						// play end.
						playEnd = true;
						break;
					}
					
					incFrameNo = startFrame;
					_seedOffset++;	//シードオフセットを加算
				}
				currentFrameNo = incFrameNo;

				// このフレームのユーザーデータをチェック
				// check the user data of this frame.
				checkUserData(currentFrameNo);
			}
		}
		else
		{
			// 逆再生時.
			// reverse play.
			for (int c = currentFrameNo - nextFrameNo; c; c--)
			{
				int decFrameNo = currentFrameNo - 1;
				if (decFrameNo < startFrame)
				{
					// アニメが一巡
					// turned animation.
					_loopCount += 1;
					if (_loop && _loopCount >= _loop)
					{
						// 再生終了.
						// play end.
						playEnd = true;
						break;
					}
				
					decFrameNo = numFrames;
					_seedOffset++;	//シードオフセットを加算
				}
				currentFrameNo = decFrameNo;

				// このフレームのユーザーデータをチェック
				// check the user data of this frame.
				checkUserData(currentFrameNo);
			}
		}
		
		_playingFrame = static_cast<float>(currentFrameNo) + nextFrameDecimal;
	}
	else
	{
		//アニメを手動で更新する場合
		checkUserData(getFrameNo());
	}

	setFrame(getFrameNo(), dt);

	//モーションブレンド用アップデート
	if (_motionBlendPlayer)
	{
//		_motionBlendPlayer->update(dt);
		_blendTime = _blendTime + dt;
		if (_blendTime >= _blendTimeMax)
		{
			_blendTime = _blendTimeMax;
			//プレイヤーを削除する
//			delete (_motionBlendPlayer);
//			_motionBlendPlayer = NULL;
			removeChild(_motionBlendPlayer, true);
			_motionBlendPlayer = NULL;
		}
	}

	if (playEnd)
	{
		stop();
	
		// 再生終了コールバックの呼び出し
		if (_playEndCallback)
		{
			_playEndCallback(this);
		}
	}
}




void Player::allocParts(int numParts, bool useCustomShaderProgram)
{
	int partnum = _parts.size();
	if (partnum < numParts)
	{
		// パーツ数だけCustomSpriteを作成する
		// create CustomSprite objects.
		float globalZOrder = getGlobalZOrder();
		for (auto i = partnum; i < numParts; i++)
		{
			CustomSprite* sprite =  CustomSprite::create();
			if (globalZOrder != 0.0f)
			{
				sprite->setGlobalZOrder(globalZOrder);
			}
			
			_parts.pushBack(sprite);
			addChild(sprite);
		}
	}
	else
	{
		// 多い分は解放する
		for (auto i = partnum - 1; i >= numParts; i--)
		{
			CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(i));
			removeChild(sprite, true);
			_parts.eraseObject(sprite);
		}
	
		// パラメータ初期化
		for (int i = 0; i < numParts; i++)
		{
			CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(i));
			sprite->initState();
		}
	}

	// 全て一旦非表示にする
	for (auto child : getChildren())
	{
		child->setVisible(false);
	}
}

void Player::releaseParts()
{
	// パーツの子CustomSpriteを全て削除
	// remove children CCSprite objects.
	removeAllChildrenWithCleanup(true);
	_parts.clear();
}

void Player::setPartsParentage()
{
	if (!_currentAnimeRef) return;

	ToPointer ptr(_currentRs->data);
	const AnimePackData* packData = _currentAnimeRef->animePackData;
	const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

	//親子関係を設定
	for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
	{
		const PartData* partData = &parts[partIndex];
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
		
		sprite->_partData = *partData;

		if (partIndex > 0)
		{
			CustomSprite* parent = static_cast<CustomSprite*>(_parts.at(partData->parentIndex));
			sprite->_parent = parent;
		}
		else
		{
			sprite->_parent = nullptr;
		}

		//インスタンスパーツの生成

		if (sprite->_ssplayer)
		{
			sprite->_ssplayer->removeFromParentAndCleanup(true);	//子供のパーツを削除
			sprite->_ssplayer = 0;
		}

		std::string refanimeName = static_cast<const char*>(ptr(partData->refname));
		sprite->_maskInfluence = partData->maskInfluence && _maskParentSetting;	//インスタンス時の親パーツを加味したマスク対象
		if (refanimeName != "")
		{
			//インスタンスパーツが設定されている
			sprite->_ssplayer = ss::Player::create();
			sprite->_ssplayer->setMaskFuncFlag(false);
			sprite->_ssplayer->setMaskParentSetting(partData->maskInfluence);

			sprite->_ssplayer->setData(_currentdataKey);
			sprite->_ssplayer->play(refanimeName);				 // アニメーション名を指定(ssae名/アニメーション名も可能、詳しくは後述)
			sprite->_ssplayer->animePause();
			sprite->addChild(sprite->_ssplayer);

		}

		//エフェクトパーツの生成
		if (sprite->refEffect)
		{
			delete sprite->refEffect;
			sprite->refEffect = 0;
		}

		std::string refeffectName = static_cast<const char*>(ptr(partData->effectfilename));
		if (refeffectName != "")
		{
			SsEffectModel* effectmodel = _currentRs->effectCache->getReference(refeffectName);
			if (effectmodel)
			{

				//エフェクトクラスにパラメータを設定する
				SsEffectRenderV2* er = new SsEffectRenderV2();
				sprite->refEffect = er;
				sprite->refEffect->setParentAnimeState(&sprite->partState);
				sprite->refEffect->setEffectData(effectmodel);
				sprite->refEffect->setSSPManeger(_sspman);

				sprite->refEffect->setSeed(getRandomSeed());
				sprite->refEffect->reload();
				sprite->refEffect->stop();
				sprite->refEffect->setLoop(false);
			}
		}
	}
}

void Player::setGlobalZOrder(float globalZOrder)
{
	if (_globalZOrder != globalZOrder)
	{
		cocos2d::Sprite::setGlobalZOrder(globalZOrder);

		for (auto child : getChildren())
		{
			child->setGlobalZOrder(globalZOrder);
		}
	}
}

//再生しているアニメーションに含まれるパーツ数を取得
int Player::getPartsCount(void)
{
	ToPointer ptr(_currentRs->data);
	const AnimePackData* packData = _currentAnimeRef->animePackData;
	return packData->numParts;
}

//indexからパーツ名を取得
const char* Player::getPartName(int partId) const
{
	ToPointer ptr(_currentRs->data);

	const AnimePackData* packData = _currentAnimeRef->animePackData;
	CCAssert(partId >= 0 && partId < packData->numParts, "partId is out of range.");

	const PartData* partData = static_cast<const PartData*>(ptr(packData->parts));
	const char* name = static_cast<const char*>(ptr(partData[partId].name));
	return name;
}

//パーツ名からindexを取得
int Player::indexOfPart(const char* partName) const
{
	const AnimePackData* packData = _currentAnimeRef->animePackData;
	for (int i = 0; i < packData->numParts; i++)
	{
		const char* name = getPartName(i);
		if (strcmp(partName, name) == 0)
		{
			return i;
		}
	}
	return -1;
}

/*
 パーツ名から指定フレームのパーツステータスを取得します。
 必要に応じて　ResluteState　を編集しデータを取得してください。
*/
bool Player::getPartState(ResluteState& result, const char* name, int frameNo)
{
	bool rc = false;
	if (_currentAnimeRef)
	{
		{
			//カレントフレームのパーツステータスを取得する
			if (frameNo == -1)
			{
				//フレームの指定が省略された場合は現在のフレームを使用する
				frameNo = getFrameNo();
			}

			if (frameNo != getFrameNo())
			{
				//取得する再生フレームのデータが違う場合プレイヤーを更新する
				//パーツステータスの更新
				setFrame(frameNo);
			}

			ToPointer ptr(_currentRs->data);

			const AnimePackData* packData = _currentAnimeRef->animePackData;
			const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

			for (int index = 0; index < packData->numParts; index++)
			{
				int partIndex = _partIndex[index];

				const PartData* partData = &parts[partIndex];
				const char* partName = static_cast<const char*>(ptr(partData->name));
				if (strcmp(partName, name) == 0)
				{
					//必要に応じて取得するパラメータを追加してください。
					//当たり判定などのパーツに付属するフラグを取得する場合は　partData　のメンバを参照してください。
					//親から継承したスケールを反映させる場合はxスケールは_mat.m[0]、yスケールは_mat.m[5]をかけて使用してください。
					CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));

					//プレイヤーの位置を取得
					cocos2d::Vec2 pos = getPosition();
					//プレイヤーのスケール値を取得
					float scaleX = getScaleX() * sprite->_mat.m[0];
					float scaleY = getScaleY() * sprite->_mat.m[5];

					//パーツアトリビュート
//					sprite->_state;												//SpriteStudio上のアトリビュートの値は_stateから取得してください
					result.flags = sprite->_state.flags;						// このフレームで更新が行われるステータスのフラグ
					result.cellIndex = sprite->_state.cellIndex;				// パーツに割り当てられたセルの番号
					result.x = ( sprite->_mat.m[12] * scaleX ) + pos.x;			//画面上のX座標を取得
					result.y = ( sprite->_mat.m[13] * scaleY ) + pos.y;			//画面上のY座標を取得
					result.z = sprite->_state.z;								// Z座標アトリビュートを取得
					result.pivotX = sprite->_state.pivotX;						// 原点Xオフセット＋セルに設定された原点オフセットX
					result.pivotY = sprite->_state.pivotY;						// 原点Yオフセット＋セルに設定された原点オフセットY
					result.rotationX = sprite->_state.rotationX;				// アトリビュート：X回転
					result.rotationY = sprite->_state.rotationY;				// アトリビュート：Y回転
					result.rotationZ = sprite->_state.rotationZ;				// アトリビュート：Z回転
					result.scaleX = sprite->_state.scaleX;						// アトリビュート：Xスケール
					result.scaleY = sprite->_state.scaleY;						// アトリビュート：Yスケール
					result.localscaleX = sprite->_state.localscaleX;			// Xローカルスケール
					result.localscaleY = sprite->_state.localscaleY;			// Yローカルスケール
					result.opacity = sprite->_state.opacity;					// 不透明度（0～255）（親子関係計算済）
					result.localopacity = sprite->_state.localopacity;			// ローカル不透明度（0～255）
					result.size_X = sprite->_state.size_X;						// アトリビュート：Xサイズ
					result.size_Y = sprite->_state.size_Y;						// アトリビュート：Yサイズ
					result.scaledsize_X = sprite->_state.size_X * scaleX;		/// 画面上のXサイズ（親子関係計算済）
					result.scaledsize_Y = sprite->_state.size_Y * scaleY;		/// 画面上のYサイズ（親子関係計算済）
					result.uv_move_X = sprite->_state.uv_move_X;				// アトリビュート：UV X移動
					result.uv_move_Y = sprite->_state.uv_move_Y;				// アトリビュート：UV Y移動
					result.uv_rotation = sprite->_state.uv_rotation;			// アトリビュート：UV 回転
					result.uv_scale_X = sprite->_state.uv_scale_X;				// アトリビュート：UV Xスケール
					result.uv_scale_Y = sprite->_state.uv_scale_Y;				// アトリビュート：UV Yスケール
					result.boundingRadius = sprite->_state.boundingRadius;		// アトリビュート：当たり半径
					result.priority = sprite->_state.priority;					// アトリビュート：優先度
					result.partsColorFunc = sprite->_state.partsColorFunc;		// アトリビュート：カラーブレンドのブレンド方法
					result.partsColorType = sprite->_state.partsColorType;		// アトリビュート：カラーブレンドの単色か頂点カラーか。
					result.flipX = sprite->_state.flipX;						// 横反転（親子関係計算済）
					result.flipY = sprite->_state.flipY;						// 縦反転（親子関係計算済）
					result.isVisibled = sprite->_state.isVisibled;				// 非表示（親子関係計算済）

					//パーツ設定
					result.part_type = partData->type;							//パーツ種別
					result.part_boundsType = partData->boundsType;				//当たり判定種類
					result.part_alphaBlendType = partData->alphaBlendType;		// BlendType
					//ラベルカラー
					std::string colorName = static_cast<const char*>(ptr(partData->colorLabel));
					if (colorName == COLORLABELSTR_NONE)
					{
						result.part_labelcolor = COLORLABEL_NONE;
					}
					if (colorName == COLORLABELSTR_RED)
					{
						result.part_labelcolor = COLORLABEL_RED;
					}
					if (colorName == COLORLABELSTR_ORANGE)
					{
						result.part_labelcolor = COLORLABEL_ORANGE;
					}
					if (colorName == COLORLABELSTR_YELLOW)
					{
						result.part_labelcolor = COLORLABEL_YELLOW;
					}
					if (colorName == COLORLABELSTR_GREEN)
					{
						result.part_labelcolor = COLORLABEL_GREEN;
					}
					if (colorName == COLORLABELSTR_BLUE)
					{
						result.part_labelcolor = COLORLABEL_BLUE;
					}
					if (colorName == COLORLABELSTR_VIOLET)
					{
						result.part_labelcolor = COLORLABEL_VIOLET;
					}
					if (colorName == COLORLABELSTR_GRAY)
					{
						result.part_labelcolor = COLORLABEL_GRAY;
					}

					rc = true;
					break;
				}
			}
			//パーツステータスを表示するフレームの内容で更新
			if (frameNo != getFrameNo())
			{
				//取得する再生フレームのデータが違う場合プレイヤーの状態をもとに戻す
				//パーツステータスの更新
				setFrame(getFrameNo());
			}
		}
	}
	return (rc);
}

//ラベル名からラベルの設定されているフレームを取得
//ラベルが存在しない場合は戻り値が-1となります。
//ラベル名が全角でついていると取得に失敗します。
int Player::getLabelToFrame(char* findLabelName)
{
	int rc = -1;

	ToPointer ptr(_currentRs->data);

	const AnimePackData* packData = _currentAnimeRef->animePackData;
	const AnimationData* animeData = _currentAnimeRef->animationData;

	if (!animeData->labelData) return -1;
	const ss_offset* labelDataIndex = static_cast<const ss_offset*>(ptr(animeData->labelData));


	int idx = 0;
	for (idx = 0; idx < animeData->labelNum; idx++ )
	{
		if (!labelDataIndex[idx]) return -1;
		const ss_u16* labelDataArray = static_cast<const ss_u16*>(ptr(labelDataIndex[idx]));

		DataArrayReader reader(labelDataArray);

		LabelData ldata;
		ss_offset offset = reader.readOffset();
		const char* str = static_cast<const char*>(ptr(offset));
		int labelFrame = reader.readU16();
		ldata.str = str;
		ldata.frameNo = labelFrame;

		if (ldata.str.compare(findLabelName) == 0 )
		{
			//同じ名前のラベルが見つかった
			return (ldata.frameNo);
		}
	}

	return (rc);
}

//パーツ名からパーツの表示、非表示を設定します
void Player::setPartVisible( std::string partsname, bool flg)
{
	bool rc = false;
	if (_currentAnimeRef)
	{
		ToPointer ptr(_currentRs->data);

		const AnimePackData* packData = _currentAnimeRef->animePackData;
		const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

		for (int index = 0; index < packData->numParts; index++)
		{
			int partIndex = _partIndex[index];

			const PartData* partData = &parts[partIndex];
			const char* partName = static_cast<const char*>(ptr(partData->name));
			if (strcmp(partName, partsname.c_str()) == 0)
			{
				_partVisible[index] = flg;
				break;
			}
		}
	}
}

//パーツに割り当たるセルを変更します
void Player::setPartCell(std::string partsname, std::string sscename, std::string cellname)
{
	bool rc = false;
	if (_currentAnimeRef)
	{
		ToPointer ptr(_currentRs->data);

		int changeCellIndex = -1;
		if ((sscename != "") && (cellname != ""))
		{
			//セルマップIDを取得する
			const Cell* cells = static_cast<const Cell*>(ptr(_currentRs->data->cells));

			//名前からインデックスの取得
			int cellindex = -1;
			for (int i = 0; i < _currentRs->data->numCells; i++)
			{
				const Cell* cell = &cells[i];
				const char* name1 = static_cast<const char*>(ptr(cell->name));
				const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
				const char* name2 = static_cast<const char*>(ptr(cellMap->name));
				if (strcmp(cellname.c_str(), name1) == 0)
				{
					if (strcmp(sscename.c_str(), name2) == 0)
					{
						changeCellIndex = i;
						break;
					}
				}
			}
		}

		const AnimePackData* packData = _currentAnimeRef->animePackData;
		const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

		for (int index = 0; index < packData->numParts; index++)
		{
			int partIndex = _partIndex[index];

			const PartData* partData = &parts[partIndex];
			const char* partName = static_cast<const char*>(ptr(partData->name));
			if (strcmp(partName, partsname.c_str()) == 0)
			{
				//セル番号を設定
				_cellChange[index] = changeCellIndex;	//上書き解除
				break;
			}
		}
	}
}

// setContentScaleFactorの数値に合わせて内部のUV補正を有効にするか設定します。
// 専用解像度のテクスチャを用意する場合はfalseにしてください。
void Player::setContentScaleEneble(bool eneble)
{
	_isContentScaleFactorAuto = eneble;
}

// インスタンスパーツが再生するアニメを変更します。
bool Player::changeInstanceAnime(std::string partsname, std::string animename, bool overWrite, Instance keyParam)
{
	//名前からパーツを取得
	bool rc = false;
	if (_currentAnimeRef)
	{
		ToPointer ptr(_currentRs->data);

		const AnimePackData* packData = _currentAnimeRef->animePackData;
		const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

		for (int index = 0; index < packData->numParts; index++)
		{
			int partIndex = _partIndex[index];

			const PartData* partData = &parts[partIndex];
			const char* partName = static_cast<const char*>(ptr(partData->name));
			if (strcmp(partName, partsname.c_str()) == 0)
			{
				CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
				if ( sprite->_ssplayer )
				{
					//パーツがインスタンスパーツの場合は再生するアニメを設定する
					//アニメが入れ子にならないようにチェックする
					if (_currentAnimename != animename )
					{
						sprite->_ssplayer->play(animename);
						sprite->_ssplayer->setInstanceParam(overWrite, keyParam);	//インスタンスパラメータの設定
						sprite->_ssplayer->animeResume();		//アニメ切り替え時にがたつく問題の対応
						sprite->_liveFrame = 0;					//独立動作の場合再生位置をリセット
						rc = true;
					}
				}

				break;
			}
		}
	}

	return ( rc );
}

//インスタンスパラメータを設定します
void Player::setInstanceParam(bool overWrite, Instance keyParam)
{
	_instanceOverWrite = overWrite;		//インスタンス情報を上書きするか？
	_instanseParam = keyParam;			//インスタンスパラメータ

}
//インスタンスパラメータを取得します
void Player::getInstanceParam(bool *overWrite, Instance *keyParam)
{
	*overWrite = _instanceOverWrite;		//インスタンス情報を上書きするか？
	*keyParam = _instanseParam;			//インスタンスパラメータ
}


//アニメーションの色成分を変更します
void Player::setColor(int r, int g, int b)
{
	_col_r = r;
	_col_g = g;
	_col_b = b;
}

//オフスクリーンレンダリングを有効にします。
void Player::offScreenRenderingEnable(bool enable, float width, float height, float pivotX, float pivotY)
{
	if (_currentAnimeRef)
	{
		if (_offScreentexture)
		{
			_offScreentexture->removeFromParentAndCleanup(true);
			_offScreentexture = nullptr;
			_offScreenWidth = 0.0f;
			_offScreenHeight = 0.0f;
			_offScreenPivotX = 0.0f;
			_offScreenPivotY = 0.0f;
		}
		if (enable == true)
		{
			//オフスクリーンレンダリングテクスチャを作成
			if (width == 0.0f)
			{
				width = _currentAnimeRef->animationData->canvasSizeW;
			}
			if (height == 0.0f)
			{
				height = _currentAnimeRef->animationData->canvasSizeH;
			}
			if (pivotX == -1000.0f)
			{
				pivotX = _currentAnimeRef->animationData->canvasPvotX;
			}
			if (pivotY == -1000.0f)
			{
				pivotY = _currentAnimeRef->animationData->canvasPvotY;
			}
			_offScreenWidth = width;
			_offScreenHeight = height;
			_offScreenPivotX = pivotX;
			_offScreenPivotY = pivotY;
			_offScreentexture = SSRenderTexture::create(width, height);
			cocos2d::Texture2D::TexParams texParams;
			texParams.wrapS = GL_CLAMP_TO_EDGE;
			texParams.wrapT = GL_CLAMP_TO_EDGE;
			texParams.minFilter = GL_NEAREST;
			texParams.magFilter = GL_NEAREST;
			_offScreentexture->getSprite()->getTexture()->setTexParameters(texParams);

			addChild(_offScreentexture);
			_offScreentexture->setVisible(true);
		}
	}
}

//アニメーションのループ範囲を設定します
void Player::setStartFrame(int frame)
{
	_startFrameOverWrite = frame;	//開始フレームの上書き設定
									//現在フレームより後の場合は先頭フレームに設定する
	if (getFrameNo() < frame)
	{
		setFrameNo(frame);
	}
}
void Player::setEndFrame(int frame)
{
	_endFrameOverWrite = frame;		//終了フレームの上書き設定
}
//アニメーションのループ範囲をラベル名で設定します
void Player::setStartFrameToLabelName(char *findLabelName)
{
	int frame = getLabelToFrame(findLabelName);
	setStartFrame(frame);
}
void Player::setEndFrameToLabelName(char *findLabelName)
{
	int frame = getLabelToFrame(findLabelName);
	if (frame != -1)
	{
		frame += 1;
	}
	setEndFrame(frame);
}

void Player::setParentMatrix(cocos2d::Mat4 mat, bool use)
{
	_parentMat =  mat;						//
	_parentMatUse = use;					//プレイヤーが持つ継承されたマトリクスがあるか？
}

//スプライト情報の取得
CustomSprite* Player::getSpriteData(int partIndex)
{
	CustomSprite* sprite = NULL;
	if (_parts.size() < partIndex)
	{
	}
	else
	{
		sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
	}
	return(sprite);
}

void Player::setFrame(int frameNo, float dt)
{
	if (!_currentAnimeRef) return;
	if (!_currentRs->data) return;

	bool forceUpdate = false;
	{
		// フリップに変化があったときは必ず描画を更新する
		CustomSprite* root = static_cast<CustomSprite*>(_parts.at(0));
		float scaleX = isFlippedX() ? -1.0f : 1.0f;
		float scaleY = isFlippedY() ? -1.0f : 1.0f;
		root->setStateValue(root->_state.scaleX, scaleX);
		root->setStateValue(root->_state.scaleY, scaleY);
		forceUpdate = root->_isStateChanged;
	}
	
	// 前回の描画フレームと同じときはスキップ
	//インスタンスアニメがあるので毎フレーム更新するためコメントに変更
//	if (!forceUpdate && frameNo == _prevDrawFrameNo) return;
	_maskIndexList.clear();

	ToPointer ptr(_currentRs->data);

	const AnimePackData* packData = _currentAnimeRef->animePackData;
	//プレイヤーが再生できるパーツの最大数を超えたアニメーションを再生している。
	//SS6Player.hに記載されている定数 #define PART_VISIBLE_MAX (xxx) の数値を編集してください。
	CCASSERT(packData->numParts < PART_VISIBLE_MAX, "Change #define PART_VISIBLE_MAX");

	const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

	const AnimationData* animeData = _currentAnimeRef->animationData;
	const ss_offset* frameDataIndex = static_cast<const ss_offset*>(ptr(animeData->frameData));
	
	const ss_u16* frameDataArray = static_cast<const ss_u16*>(ptr(frameDataIndex[frameNo]));
	DataArrayReader reader(frameDataArray);

	const AnimationInitialData* initialDataList = static_cast<const AnimationInitialData*>(ptr(animeData->defaultData));

	State state;
	cocos2d::V3F_C4B_T2F_Quad tempQuad;

	for (int index = 0; index < packData->numParts; index++)
	{
		int partIndex = reader.readS16();
		const PartData* partData = &parts[partIndex];
		const AnimationInitialData* init = &initialDataList[partIndex];

		// optional parameters
		int flags = reader.readU32();
		int cellIndex = flags & PART_FLAG_CELL_INDEX ? reader.readS16() : init->cellIndex;
		float x = flags & PART_FLAG_POSITION_X ? reader.readFloat() : init->positionX;
		float y = flags & PART_FLAG_POSITION_Y ? reader.readFloat() : init->positionY;
		float z = flags & PART_FLAG_POSITION_Z ? reader.readFloat() : init->positionZ;
		float pivotX = flags & PART_FLAG_PIVOT_X ? reader.readFloat() : init->pivotX;
		float pivotY = flags & PART_FLAG_PIVOT_Y ? -reader.readFloat() : -init->pivotY;		//cocosでは上下が逆なので反転する
		float rotationX = flags & PART_FLAG_ROTATIONX ? -reader.readFloat() : -init->rotationX;
		float rotationY = flags & PART_FLAG_ROTATIONY ? -reader.readFloat() : -init->rotationY;
		float rotationZ = flags & PART_FLAG_ROTATIONZ ? -reader.readFloat() : -init->rotationZ;
		float scaleX = flags & PART_FLAG_SCALE_X ? reader.readFloat() : init->scaleX;
		float scaleY = flags & PART_FLAG_SCALE_Y ? reader.readFloat() : init->scaleY;
		float localscaleX = flags & PART_FLAG_LOCALSCALE_X ? reader.readFloat() : init->localscaleX;
		float localscaleY = flags & PART_FLAG_LOCALSCALE_Y ? reader.readFloat() : init->localscaleY;
		int opacity = flags & PART_FLAG_OPACITY ? reader.readU16() : init->opacity;
		int localopacity = flags & PART_FLAG_LOCALOPACITY ? reader.readU16() : init->localopacity;
		float size_X = flags & PART_FLAG_SIZE_X ? reader.readFloat() : init->size_X;
		float size_Y = flags & PART_FLAG_SIZE_Y ? reader.readFloat() : init->size_Y;
		float uv_move_X = flags & PART_FLAG_U_MOVE ? reader.readFloat() : init->uv_move_X;
		float uv_move_Y = flags & PART_FLAG_V_MOVE ? reader.readFloat() : init->uv_move_Y;
		float uv_rotation = flags & PART_FLAG_UV_ROTATION ? reader.readFloat() : init->uv_rotation;
		float uv_scale_X = flags & PART_FLAG_U_SCALE ? reader.readFloat() : init->uv_scale_X;
		float uv_scale_Y = flags & PART_FLAG_V_SCALE ? reader.readFloat() : init->uv_scale_Y;
		float boundingRadius = flags & PART_FLAG_BOUNDINGRADIUS ? reader.readFloat() : init->boundingRadius;
		float masklimen = flags & PART_FLAG_MASK ? reader.readU16() : init->masklimen;
		float priority = flags & PART_FLAG_PRIORITY ? reader.readU16() : init->priority;

		//インスタンスアトリビュート
		int		instanceValue_curKeyframe = flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_curKeyframe;
		int		instanceValue_startFrame = flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_startFrame;
		int		instanceValue_endFrame = flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_endFrame;
		int		instanceValue_loopNum = flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_loopNum;
		float	instanceValue_speed = flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readFloat() : init->instanceValue_speed;
		int		instanceValue_loopflag = flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_loopflag;
		//エフェクトアトリビュート
		int		effectValue_curKeyframe = flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readS32() : init->effectValue_curKeyframe;
		int		effectValue_startTime = flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readS32() : init->effectValue_startTime;
		float	effectValue_speed = flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readFloat() : init->effectValue_speed;
		int		effectValue_loopflag = flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readS32() : init->effectValue_loopflag;

		bool flipX = (bool)(flags & PART_FLAG_FLIP_H);
		bool flipY = (bool)(flags & PART_FLAG_FLIP_V);
		bool isVisibled = !(flags & PART_FLAG_INVISIBLE);

		if (_partVisible[index] == false)
		{
			//ユーザーが任意に非表示としたパーツは非表示に設定
			isVisibled = false;
		}

		if (_cellChange[index] != -1)
		{
			//ユーザーがセルを上書きした
			cellIndex = _cellChange[index];
		}

		_partIndex[index] = partIndex;

		//オフスクリーンレンダリング時はrootパーツの位置を画面の中央に移動させる
		if (_offScreentexture)
		{
			if (partIndex == 0)
			{
				x += _offScreenWidth * (_offScreenPivotX + 0.5f);
				y += _offScreenHeight * (_offScreenPivotY + 0.5f);
			}
		}


		//インスタンスパーツのパラメータを加える
		//不透明度はすでにコンバータで親の透明度が計算されているため
		//全パーツにインスタンスの透明度を加える必要がある
		opacity = (opacity * _InstanceAlpha) / 255;

		//セルの原点設定を反映させる
		CellRef* cellRef = cellIndex >= 0 ? _currentRs->cellCache->getReference(cellIndex) : nullptr;
		if (cellRef)
		{
			float cpx = 0;
			float cpy = 0;

			cpx = cellRef->cell->pivot_X;
			if (flipX) cpx = -cpx;	// 水平フリップによって原点を入れ替える
			cpy = cellRef->cell->pivot_Y;
			if (flipY) cpy = -cpy;	// 垂直フリップによって原点を入れ替える

			pivotX += cpx;
			pivotY += cpy;

		}
		pivotX += 0.5f;
		pivotY += 0.5f;

		//モーションブレンド
		if (_motionBlendPlayer)
		{
			CustomSprite* blendSprite = _motionBlendPlayer->getSpriteData(partIndex);
			if (blendSprite)
			{
				float percent = _blendTime / _blendTimeMax;
				x = parcentVal(x, blendSprite->_orgState.x, percent);
				y = parcentVal(y, blendSprite->_orgState.y, percent);
				scaleX = parcentVal(scaleX, blendSprite->_orgState.scaleX, percent);
				scaleY = parcentVal(scaleY, blendSprite->_orgState.scaleY, percent);
				rotationX = parcentValRot(rotationX, blendSprite->_orgState.rotationX, percent);
				rotationY = parcentValRot(rotationY, blendSprite->_orgState.rotationY, percent);
				rotationZ = parcentValRot(rotationZ, blendSprite->_orgState.rotationZ, percent);
			}
		}

		//ステータス保存
		state.name = static_cast<const char*>(ptr(partData->name));
		state.flags = flags;
		state.cellIndex = cellIndex;
		state.x = x;
		state.y = y;
		state.z = z;
		state.pivotX = pivotX;
		state.pivotY = pivotY;
		state.rotationX = rotationX;
		state.rotationY = rotationY;
		state.rotationZ = rotationZ;
		state.scaleX = scaleX;
		state.scaleY = scaleY;
		state.localscaleX = localscaleX;
		state.localscaleY = localscaleY;
		state.opacity = opacity;
		state.localopacity = localopacity;
		state.size_X = size_X;
		state.size_Y = size_Y;
		state.uv_move_X = uv_move_X;
		state.uv_move_Y = uv_move_Y;
		state.uv_rotation = uv_rotation;
		state.uv_scale_X = uv_scale_X;
		state.uv_scale_Y = uv_scale_Y;
		state.boundingRadius = boundingRadius;
		state.masklimen = masklimen;
		state.priority = priority;
		state.isVisibled = isVisibled;
		state.flipX = flipX;
		state.flipY = flipY;
		state.instancerotationX = _InstanceRotX;
		state.instancerotationY = _InstanceRotY;
		state.instancerotationZ = _InstanceRotZ;

		state.instanceValue_curKeyframe = instanceValue_curKeyframe;
		state.instanceValue_startFrame = instanceValue_startFrame;
		state.instanceValue_endFrame = instanceValue_endFrame;
		state.instanceValue_loopNum = instanceValue_loopNum;
		state.instanceValue_speed = instanceValue_speed;
		state.instanceValue_loopflag = instanceValue_loopflag;
		state.effectValue_curKeyframe = effectValue_curKeyframe;
		state.effectValue_startTime = effectValue_startTime;
		state.effectValue_speed = effectValue_speed;
		state.effectValue_loopflag = effectValue_loopflag;

		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));

		//表示設定
		if (opacity == 0)
		{
			//不透明度0の時は非表示にする
			isVisibled = false;
		}
		sprite->setLocalZOrder(index);

		sprite->setPosition(cocos2d::Point(x, y));
		//		sprite->setRotation(rotationZ);				// for Cocos2d-x ver 3.6
		// for Cocos2d-x ver 3.7
		cocos2d::Vec3 rot(rotationX, rotationY, rotationZ);
		sprite->setRotation3D(rot);
		// --

		bool setBlendEnabled = true;
		if (cellRef)
		{
			if (cellRef->texture)
			{
				sprite->setTexture(cellRef->texture);
				sprite->setTextureRect(cellRef->rect);

				if (setBlendEnabled)
				{
					// ブレンド方法を設定
					// 標準状態でMIXブレンド相当になります
					// BlendFuncの値を変更することでブレンド方法を切り替えます
					cocos2d::BlendFunc blendFunc = sprite->getBlendFunc();

					if (flags & PART_FLAG_PARTS_COLOR)
					{
						if (!cellRef->texture->hasPremultipliedAlpha())
						{
							blendFunc.src = GL_SRC_ALPHA;
							blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
						}
						else
						{
							blendFunc.src = CC_BLEND_SRC;
							blendFunc.dst = CC_BLEND_DST;
						}

						// カスタムシェーダを使用する場合
						blendFunc.src = GL_SRC_ALPHA;

						// 乗算ブレンド
						if (partData->alphaBlendType == BLEND_MUL) {
							blendFunc.src = GL_DST_COLOR;
							blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
						}
						// 加算ブレンド
						if (partData->alphaBlendType == BLEND_ADD) {
							blendFunc.dst = GL_ONE;
						}
						// 減算ブレンド
						if (partData->alphaBlendType == BLEND_SUB) {
							blendFunc.src = GL_ZERO;
							blendFunc.dst = GL_ONE_MINUS_SRC_COLOR;
						}
					}
					else
					{
						// 通常ブレンド
						if (partData->alphaBlendType == BLEND_MIX)
						{
							if (!cellRef->texture->hasPremultipliedAlpha())
							{
								blendFunc.src = GL_SRC_ALPHA;
								blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
							}
							else
							{
								blendFunc.src = GL_ONE;
								blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
							}
						}
						// 加算ブレンド
						if (partData->alphaBlendType == BLEND_ADD) {
							if (!cellRef->texture->hasPremultipliedAlpha())
							{
								blendFunc.src = GL_SRC_ALPHA;
								blendFunc.dst = GL_ONE;
							}
							else
							{
								blendFunc.src = GL_ONE;
								blendFunc.dst = GL_ONE;
							}
						}
						// 乗算ブレンド
						if (partData->alphaBlendType == BLEND_MUL) {
							if (!cellRef->texture->hasPremultipliedAlpha())
							{
								blendFunc.src = GL_ZERO;
								blendFunc.dst = GL_SRC_COLOR;
							}
							else
							{
								blendFunc.src = GL_DST_COLOR;
								blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
							}
						}
						// 減算ブレンド
						if (partData->alphaBlendType == BLEND_SUB) {
							if (!cellRef->texture->hasPremultipliedAlpha())
							{
								blendFunc.src = GL_ZERO;
								blendFunc.dst = GL_ONE_MINUS_SRC_COLOR;
							}
							else
							{
								blendFunc.src = GL_ONE_MINUS_SRC_ALPHA;
								blendFunc.dst = GL_ONE_MINUS_SRC_COLOR;
							}
						}
						/*
						//除外
						if (partData->alphaBlendType == BLEND_) {
						blendFunc.src = GL_ONE_MINUS_DST_COLOR;
						blendFunc.dst = GL_ONE_MINUS_SRC_COLOR;
						}
						//スクリーン
						if (partData->alphaBlendType == BLEND_) {
						blendFunc.src = GL_ONE_MINUS_DST_COLOR;
						blendFunc.dst = GL_ONE;
						}
						*/
					}

					sprite->setBlendFunc(blendFunc);
				}
			}
			else
			{
				sprite->setTexture(nullptr);
				sprite->setTextureRect(cocos2d::Rect());
				//セルが無く通常パーツ、マスク、NULLパーツの時は非表示にする
				if ((partData->type == PARTTYPE_NORMAL) || (partData->type == PARTTYPE_MASK) || (partData->type == PARTTYPE_NULL))
				{
					isVisibled = false;
				}
			}
		}
		else
		{
			sprite->setTexture(nullptr);
			sprite->setTextureRect(cocos2d::Rect());
			//セルが無く通常パーツ、ヌルパーツの時は非表示にする
			if ((partData->type == PARTTYPE_NORMAL) || (partData->type == PARTTYPE_NULL))
			{
				isVisibled = false;
			}
		}
		sprite->setVisible(isVisibled);

		sprite->setAnchorPoint(cocos2d::Point(pivotX, 1.0f - pivotY));	//cocosは下が-なので座標を反転させる
		sprite->setFlippedX(flags & PART_FLAG_FLIP_H);
		sprite->setFlippedY(flags & PART_FLAG_FLIP_V);
		sprite->setOpacity(opacity);

		//頂点データの取得
		cocos2d::V3F_C4B_T2F_Quad& quad = sprite->getAttributeRef();
		if (_isContentScaleFactorAuto == true)
		{
			//ContentScaleFactor対応
			float cScale = cocos2d::Director::getInstance()->getContentScaleFactor();
			quad.tl.texCoords.u /= cScale;
			quad.tr.texCoords.u /= cScale;
			quad.bl.texCoords.u /= cScale;
			quad.br.texCoords.u /= cScale;
			quad.tl.texCoords.v /= cScale;
			quad.tr.texCoords.v /= cScale;
			quad.bl.texCoords.v /= cScale;
			quad.br.texCoords.v /= cScale;
		}

		//サイズ設定
		if (flags & PART_FLAG_SIZE_X)
		{
			float w = 0;
			float center = 0;
			w = (quad.tr.vertices.x - quad.tl.vertices.x) / 2.0f;
			if (w != 0.0f)
			{
				center = quad.tl.vertices.x + w;
				float scale = (size_X / 2.0f) / w;

				quad.bl.vertices.x = center - (w * scale);
				quad.br.vertices.x = center + (w * scale);
				quad.tl.vertices.x = center - (w * scale);
				quad.tr.vertices.x = center + (w * scale);
			}
		}
		if (flags & PART_FLAG_SIZE_Y)
		{
			float h = 0;
			float center = 0;
			h = (quad.bl.vertices.y - quad.tl.vertices.y) / 2.0f;
			if (h != 0.0f)
			{
				center = quad.tl.vertices.y + h;
				float scale = (size_Y / 2.0f) / h;

				quad.bl.vertices.y = center - (h * scale);
				quad.br.vertices.y = center - (h * scale);
				quad.tl.vertices.y = center + (h * scale);
				quad.tr.vertices.y = center + (h * scale);
			}
		}
		sprite->setScale(scaleX, scaleY);	//スケール設定


		// 頂点変形のオフセット値を反映
		if (flags & PART_FLAG_VERTEX_TRANSFORM)
		{
			int vt_flags = reader.readU16();
			if (vt_flags & VERTEX_FLAG_LT)
			{
				quad.tl.vertices.x += reader.readS16();
				quad.tl.vertices.y += reader.readS16();
			}
			if (vt_flags & VERTEX_FLAG_RT)
			{
				quad.tr.vertices.x += reader.readS16();
				quad.tr.vertices.y += reader.readS16();
			}
			if (vt_flags & VERTEX_FLAG_LB)
			{
				quad.bl.vertices.x += reader.readS16();
				quad.bl.vertices.y += reader.readS16();
			}
			if (vt_flags & VERTEX_FLAG_RB)
			{
				quad.br.vertices.x += reader.readS16();
				quad.br.vertices.y += reader.readS16();
			}
		}


		//頂点情報の取得
		GLubyte alpha = (GLubyte)opacity;
		cocos2d::Color4B color4( 0xff, 0xff, 0xff, opacity);

		sprite->sethasPremultipliedAlpha(0);	//
		if (cellRef)
		{
			if (cellRef->texture)
			{
				if (cellRef->texture->hasPremultipliedAlpha())
				{
					//テクスチャのカラー値にアルファがかかっている場合は、アルファ値をカラー値に反映させる
					color4.r = color4.r * alpha * _col_r / 255 / 255;
					color4.g = color4.g * alpha * _col_g / 255 / 255;
					color4.b = color4.b * alpha * _col_b / 255 / 255;
					// 加算ブレンド
					if (partData->alphaBlendType == BLEND_ADD)
					{
						color4.a = 255;	//加算の場合はアルファの計算を行わない。(カラー値にアルファ分が計算されているため)
					}
					sprite->sethasPremultipliedAlpha(1);
				}
				else
				{
					//テクスチャのカラー値を変更する
					color4.r = color4.r * _col_r / 255;
					color4.g = color4.g * _col_g / 255;
					color4.b = color4.b * _col_b / 255;
				}
			}
		}
		quad.tl.colors =
		quad.tr.colors =
		quad.bl.colors =
		quad.br.colors = color4;


		// パーツカラーの反映  
		if (flags & PART_FLAG_PARTS_COLOR)
		{

			int typeAndFlags = reader.readU16();
			int funcNo = typeAndFlags & 0xff;
			int cb_flags = (typeAndFlags >> 8) & 0xff;
	
			sprite->_state.partsColorFunc = funcNo;  
			sprite->_state.partsColorType = cb_flags;

			if (cb_flags & VERTEX_FLAG_ONE)
			{
				reader.readColor(color4);

				quad.tl.colors =
				quad.tr.colors =
				quad.bl.colors =
				quad.br.colors = color4;
			}
			else
			{
				if (cb_flags & VERTEX_FLAG_LT)
				{
					reader.readColor(color4);
					quad.tl.colors = color4;
				}
				if (cb_flags & VERTEX_FLAG_RT)
				{
					reader.readColor(color4);
					quad.tr.colors = color4;
				}
				if (cb_flags & VERTEX_FLAG_LB)
				{
					reader.readColor(color4);
					quad.bl.colors = color4;
				}
				if (cb_flags & VERTEX_FLAG_RB)
				{
					reader.readColor(color4);
					quad.br.colors = color4;
				}
			}
		}
		//uvスクロール
		if (flags & PART_FLAG_U_MOVE)
		{
			quad.tl.texCoords.u += uv_move_X;
			quad.tr.texCoords.u += uv_move_X;
			quad.bl.texCoords.u += uv_move_X;
			quad.br.texCoords.u += uv_move_X;
		}
		if (flags & PART_FLAG_V_MOVE)
		{
			quad.tl.texCoords.v += uv_move_Y;
			quad.tr.texCoords.v += uv_move_Y;
			quad.bl.texCoords.v += uv_move_Y;
			quad.br.texCoords.v += uv_move_Y;
		}


		float u_wide = 0;
		float v_height = 0;
		float u_center = 0;
		float v_center = 0;
		float u_code = 1;
		float v_code = 1;

		if (flags & PART_FLAG_FLIP_H)
		{
			//左右反転を行う場合はテクスチャUVを逆にする
			u_wide = (quad.tl.texCoords.u - quad.tr.texCoords.u) / 2.0f;
			u_center = quad.tr.texCoords.u + u_wide;
			u_code = -1;
		}
		else
		{
			u_wide = (quad.tr.texCoords.u - quad.tl.texCoords.u) / 2.0f;
			u_center = quad.tl.texCoords.u + u_wide;
		}
		if (flags & PART_FLAG_FLIP_V)
		{
			//左右反転を行う場合はテクスチャUVを逆にする
			v_height = (quad.tl.texCoords.v - quad.bl.texCoords.v) / 2.0f;
			v_center = quad.bl.texCoords.v + v_height;
			v_code = -1;
		}
		else
		{
			v_height = (quad.bl.texCoords.v - quad.tl.texCoords.v) / 2.0f;
			v_center = quad.tl.texCoords.v + v_height;
		}

		//UVスケール
		if (flags & PART_FLAG_U_SCALE)
		{
			quad.tl.texCoords.u = u_center - (u_wide * uv_scale_X * u_code);
			quad.tr.texCoords.u = u_center + (u_wide * uv_scale_X * u_code);
			quad.bl.texCoords.u = u_center - (u_wide * uv_scale_X * u_code);
			quad.br.texCoords.u = u_center + (u_wide * uv_scale_X * u_code);
		}
		if (flags & PART_FLAG_V_SCALE)
		{
			quad.tl.texCoords.v = v_center - (v_height * uv_scale_Y * v_code);
			quad.tr.texCoords.v = v_center - (v_height * uv_scale_Y * v_code);
			quad.bl.texCoords.v = v_center + (v_height * uv_scale_Y * v_code);
			quad.br.texCoords.v = v_center + (v_height * uv_scale_Y * v_code);
		}
		//UV回転
		if (flags & PART_FLAG_UV_ROTATION)
		{
			//頂点位置を回転させる
			get_uv_rotation(&quad.tl.texCoords.u, &quad.tl.texCoords.v, u_center, v_center, uv_rotation);
			get_uv_rotation(&quad.tr.texCoords.u, &quad.tr.texCoords.v, u_center, v_center, uv_rotation);
			get_uv_rotation(&quad.bl.texCoords.u, &quad.bl.texCoords.v, u_center, v_center, uv_rotation);
			get_uv_rotation(&quad.br.texCoords.u, &quad.br.texCoords.v, u_center, v_center, uv_rotation);
		}


		//インスタンスパーツの場合
		if (partData->type == PARTTYPE_INSTANCE)
		{
			bool overWrite;
			Instance keyParam;
			sprite->_ssplayer->getInstanceParam(&overWrite, &keyParam);
			//描画
			int refKeyframe = instanceValue_curKeyframe;
			int refStartframe = instanceValue_startFrame;
			int refEndframe = instanceValue_endFrame;
			float refSpeed = instanceValue_speed;
			int refloopNum = instanceValue_loopNum;
			bool infinity = false;
			bool reverse = false;
			bool pingpong = false;
			bool independent = false;

			int lflags = instanceValue_loopflag;
			if (lflags & INSTANCE_LOOP_FLAG_INFINITY)
			{
				//無限ループ
				infinity = true;
			}
			if (lflags & INSTANCE_LOOP_FLAG_REVERSE)
			{
				//逆再生
				reverse = true;
			}
			if (lflags & INSTANCE_LOOP_FLAG_PINGPONG)
			{
				//往復
				pingpong = true;
			}
			if (lflags & INSTANCE_LOOP_FLAG_INDEPENDENT)
			{
				//独立
				independent = true;
			}
			//インスタンスパラメータを上書きする
			if (overWrite == true)
			{
				refStartframe = keyParam.refStartframe;		//開始フレーム
				refEndframe = keyParam.refEndframe;			//終了フレーム
				refSpeed = keyParam.refSpeed;				//再生速度
				refloopNum = keyParam.refloopNum;			//ループ回数
				infinity = keyParam.infinity;				//無限ループ
				reverse = keyParam.reverse;					//逆再選
				pingpong = keyParam.pingpong;				//往復
				independent = keyParam.independent;			//独立動作
			}

			//タイムライン上の時間 （絶対時間）
			int time = frameNo;

			//独立動作の場合
			if (independent)
			{
				float fdt = dt;
				float delta = fdt / (1.0f / _animefps);						//v1.0.8	独立動作時は親アニメのfpsを使用する
				//				float delta = fdt / (1.0f / sprite->_ssplayer->_animefps);	//v1.0.7	独立動作時はソースアニメのfpsを使用する

				sprite->_liveFrame += delta;
				time = (int)sprite->_liveFrame;
			}

			//このインスタンスが配置されたキーフレーム（絶対時間）
			int	selfTopKeyframe = refKeyframe;


			int	reftime = (int)((float)(time - selfTopKeyframe) * refSpeed); //開始から現在の経過時間
			if (reftime < 0) continue;							//そもそも生存時間に存在していない
			if (selfTopKeyframe > time) continue;

			int inst_scale = (refEndframe - refStartframe) + 1; //インスタンスの尺

			//尺が０もしくはマイナス（あり得ない
			if (inst_scale <= 0) continue;
			//changeInstanceAnime()でソースアニメの参照を変更した場合に尺が変わるので、超えてしまう場合がある。
			//最大を超えた場合はメモリ外を参照してしまうのでアサートで止めておく
			CCASSERT(inst_scale <= sprite->_ssplayer->_currentAnimeRef->animationData->endFrames, "_playingFrame It has more than the length of the InstanceAnimation");

			int	nowloop = (reftime / inst_scale);	//現在までのループ数

			int checkloopnum = refloopNum;

			//pingpongの場合では２倍にする
			if (pingpong) checkloopnum = checkloopnum * 2;

			//無限ループで無い時にループ数をチェック
			if (!infinity)   //無限フラグが有効な場合はチェックせず
			{
				if (nowloop >= checkloopnum)
				{
					reftime = inst_scale - 1;
					nowloop = checkloopnum - 1;
				}
			}

			int temp_frame = reftime % inst_scale;  //ループを加味しないインスタンスアニメ内のフレーム

			//参照位置を決める
			//現在の再生フレームの計算
			int _time = 0;
			if (pingpong && (nowloop % 2 == 1))
			{
				if (reverse)
				{
					reverse = false;//反転
				}
				else
				{
					reverse = true;//反転
				}
			}

			if (reverse)
			{
				//リバースの時
				_time = refEndframe - temp_frame;
			}
			else{
				//通常時
				_time = temp_frame + refStartframe;
			}
			//インスタンス用SSPlayerに再生フレームを設定する
			sprite->_ssplayer->setFrameNo(_time);
		}

		//スプライトステータスの保存
		sprite->setState(state);
		sprite->_orgState = sprite->_state;

		if (partData->type == PARTTYPE_MASK)
		{
			_maskIndexList.push_back(sprite);
		}
	}

	// 親に変更があるときは自分も更新するようフラグを設定する
	for (int partIndex = 1; partIndex < packData->numParts; partIndex++)
	{
		const PartData* partData = &parts[partIndex];
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
		CustomSprite* parent = static_cast<CustomSprite*>(_parts.at(partData->parentIndex));
		
		if (parent->_isStateChanged)
		{
			sprite->_isStateChanged = true;
		}
	}
	
	// 行列の更新
	cocos2d::Mat4 mat, t;
	for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
	{
		const PartData* partData = &parts[partIndex];
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
		
		if (sprite->_isStateChanged)
		{
			{
				int num = 1;
				if ((sprite->_state.flags & PART_FLAG_LOCALSCALE_X) || (sprite->_state.flags & PART_FLAG_LOCALSCALE_Y))
				{
					//ローカルスケール対応
					num = 2;
				}
				int matcnt;
				for (matcnt = 0; matcnt < num; matcnt++)
				{
					if (partIndex > 0)
					{
						CustomSprite* parent = static_cast<CustomSprite*>(_parts.at(partData->parentIndex));
						mat = parent->_mat;
					}
					else
					{
						mat = cocos2d::Mat4::IDENTITY;
						//rootパーツはプレイヤーからステータスを引き継ぐ
						if (_parentMatUse == true)					//プレイヤーが持つ継承されたマトリクスがあるか？
						{
							mat = _parentMat;
						}

						//rootパーツの場合はプレイヤーのフリップをみてスケールを反転する
						float scaleX = isFlippedX() ? -1.0f : 1.0f;
						float scaleY = isFlippedY() ? -1.0f : 1.0f;

						cocos2d::Mat4::createScale(scaleX, scaleY, 1.0f, &t);
						mat = mat * t;

					}

					cocos2d::Mat4::createTranslation(sprite->_state.x, sprite->_state.y, 0.0f, &t);
					mat = mat * t;

					cocos2d::Mat4::createRotationX(CC_DEGREES_TO_RADIANS(-sprite->_state.rotationX), &t);
					mat = mat * t;

					cocos2d::Mat4::createRotationY(CC_DEGREES_TO_RADIANS(-sprite->_state.rotationY), &t);
					mat = mat * t;

					cocos2d::Mat4::createRotationZ(CC_DEGREES_TO_RADIANS(-sprite->_state.rotationZ), &t);
					mat = mat * t;

					float sx = sprite->_state.scaleX;
					float sy = sprite->_state.scaleY;
					if (matcnt > 0)
					{
						//ローカルスケールを適用する
						sx *= sprite->_state.localscaleX;
						sy *= sprite->_state.localscaleY;
					}
					cocos2d::Mat4::createScale(sx, sy, 1.0f, &t);
					mat = mat * t;

				}
				if (num == 1)
				{
					//ローカルスケールが使用されていない場合は継承マトリクスをローカルマトリクスに適用
					sprite->_localmat = mat;
				}
				sprite->_mat = mat;
				sprite->_isStateChanged = false;

				// 行列を再計算させる
				sprite->setAdditionalTransform(nullptr);
				sprite->Set_transformDirty();	//Ver 3.13.1対応
				//インスタンスパーツの親を設定のマトリクスを設定する
				if (sprite->_ssplayer)
				{
					sprite->_ssplayer->setParentMatrix(sprite->_mat, true);	//プレイヤーに対してマトリクスを設定する
					//インスタンスパラメータを設定
					if (sprite->_state.flags & PART_FLAG_LOCALOPACITY)
					{
						sprite->_ssplayer->setAlpha(sprite->_state.localopacity);
					}
					else
					{
						sprite->_ssplayer->setAlpha(sprite->_state.opacity);
					}
					sprite->_ssplayer->setContentScaleEneble(_isContentScaleFactorAuto);
					sprite->_ssplayer->setColor(_col_r, _col_g, _col_b);
				}
			}
		}
	}
	if (_isPlayFirstUpdate == false)
	{
		// エフェクトのアップデート
		for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
		{
			const PartData* partData = &parts[partIndex];
			CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));

			//エフェクトのアップデート
			if (sprite->refEffect)
			{

				//エフェクトアトリビュート
				int curKeyframe = sprite->_state.effectValue_curKeyframe;
				int refStartframe = sprite->_state.effectValue_startTime;
				float refSpeed = sprite->_state.effectValue_speed;
				bool independent = false;

				int lflags = sprite->_state.effectValue_loopflag;
				if (lflags & EFFECT_LOOP_FLAG_INDEPENDENT)
				{
					independent = true;
				}

				if (sprite->effectAttrInitialized == false)
				{
					sprite->effectAttrInitialized = true;
					sprite->effectTimeTotal = refStartframe;
				}

				//パーツのステータスの更新
				sprite->partState.alpha = sprite->_state.opacity / 255.0f;
//				int matindex = 0;
//				for (matindex = 0; matindex < 16; matindex++)
//				{
//					sprite->partState.matrix[matindex] = sprite->_mat.m[matindex];
//				}
				sprite->refEffect->setContentScaleEneble(_isContentScaleFactorAuto);
				sprite->refEffect->setParentSprite(sprite);	//親スプライトの設定

				if (sprite->_state.isVisibled == true)
				{

					if (independent)
					{
						//独立動作
						if (sprite->effectAttrInitialized)
						{
							float delta = dt / (1.0f / _animefps);						//	独立動作時は親アニメのfpsを使用する
							sprite->effectTimeTotal += delta * refSpeed;
							sprite->refEffect->setLoop(true);
							sprite->refEffect->setFrame(sprite->effectTimeTotal);
							sprite->refEffect->play();
							sprite->refEffect->update();
						}
					}
					else
					{
						{
							float _time = frameNo - curKeyframe;
							if (_time < 0)
							{
							}
							else
							{
								_time *= refSpeed;
								_time = _time + refStartframe;
								sprite->effectTimeTotal = _time;

								sprite->refEffect->setSeedOffset(_seedOffset);
								sprite->refEffect->setFrame(_time);
								sprite->refEffect->play();
								sprite->refEffect->update();
							}
						}
					}
					sprite->refEffect->draw();
				}
			}
		}

		//オフスクリーンレンダリング対応
		if (_offScreentexture)
		{
			_sspman->setUseOffscreenRendering(true);
			_offScreentexture->setVisible(true);
			_offScreentexture->beginWithClear(0, 0, 0, 0);
			for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
			{
				CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(_partIndex[partIndex]));
//				if (sprite->isCustomShaderProgramEnabled() == false)	//カラーブレンドの設定されたスプライトは表示しない
				{ 
					sprite->visit();
				}
				sprite->setVisible(false);
			}
			_offScreentexture->end();	//描画開始
			_sspman->setUseOffscreenRendering(false);
		}
	}
	_isPlayFirstUpdate = false;


	_prevDrawFrameNo = frameNo;	//再生したフレームを保存

}

//ユーザーデータの取得
void Player::checkUserData(int frameNo)
{
	if (!_userDataCallback) return;
	
	ToPointer ptr(_currentRs->data);

	const AnimePackData* packData = _currentAnimeRef->animePackData;
	const AnimationData* animeData = _currentAnimeRef->animationData;
	const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

	if (!animeData->userData) return;
	const ss_offset* userDataIndex = static_cast<const ss_offset*>(ptr(animeData->userData));

	if (!userDataIndex[frameNo]) return;
	const ss_u16* userDataArray = static_cast<const ss_u16*>(ptr(userDataIndex[frameNo]));
	
	DataArrayReader reader(userDataArray);
	int numUserData = reader.readU16();

	for (int i = 0; i < numUserData; i++)
	{
		int flags = reader.readU16();
		int partIndex = reader.readU16();

		_userData.flags = 0;

		if (flags & UserData::FLAG_INTEGER)
		{
			_userData.flags |= UserData::FLAG_INTEGER;
			_userData.integer = reader.readS32();
		}
		else
		{
			_userData.integer = 0;
		}
		
		if (flags & UserData::FLAG_RECT)
		{
			_userData.flags |= UserData::FLAG_RECT;
			_userData.rect[0] = reader.readS32();
			_userData.rect[1] = reader.readS32();
			_userData.rect[2] = reader.readS32();
			_userData.rect[3] = reader.readS32();
		}
		else
		{
			_userData.rect[0] =
			_userData.rect[1] =
			_userData.rect[2] =
			_userData.rect[3] = 0;
		}
		
		if (flags & UserData::FLAG_POINT)
		{
			_userData.flags |= UserData::FLAG_POINT;
			_userData.point[0] = reader.readS32();
			_userData.point[1] = reader.readS32();
		}
		else
		{
			_userData.point[0] =
			_userData.point[1] = 0;
		}
		
		if (flags & UserData::FLAG_STRING)
		{
			_userData.flags |= UserData::FLAG_STRING;
			int size = reader.readU16();
			ss_offset offset = reader.readOffset();
			const char* str = static_cast<const char*>(ptr(offset));
			_userData.str = str;
			_userData.strSize = size;
		}
		else
		{
			_userData.str = 0;
			_userData.strSize = 0;
		}
		
		_userData.partName = static_cast<const char*>(ptr(parts[partIndex].name));
		_userData.frameNo = frameNo;
		
		_userDataCallback(this, &_userData);
	}
}

#define __PI__	(3.14159265358979323846f)
#define RadianToDegree(Radian) ((double)Radian * (180.0f / __PI__))
#define DegreeToRadian(Degree) ((double)Degree * (__PI__ / 180.0f))

void Player::get_uv_rotation(float *u, float *v, float cu, float cv, float deg)
{
	float dx = *u - cu; // 中心からの距離(X)
	float dy = *v - cv; // 中心からの距離(Y)

	float tmpX = ( dx * cosf(DegreeToRadian(deg)) ) - ( dy * sinf(DegreeToRadian(deg)) ); // 回転
	float tmpY = ( dx * sinf(DegreeToRadian(deg)) ) + ( dy * cosf(DegreeToRadian(deg)) );

	*u = (cu + tmpX); // 元の座標にオフセットする
	*v = (cv + tmpY);

}

//インスタンスパーツのアルファ値を設定
void  Player::setAlpha(int alpha)
{
	_InstanceAlpha = alpha;
}

//インスタンスパーツの回転値を設定
void  Player::set_InstanceRotation(float rotX, float rotY, float rotZ)
{
	_InstanceRotX = rotX;
	_InstanceRotY = rotY;
	_InstanceRotZ = rotZ;
}

//割合に応じた中間値を取得します
float Player::parcentVal(float val1, float val2, float parcent)
{
	float sa = val1 - val2;
	float newval = val2 + (sa * parcent);
	return (newval);
}
float Player::parcentValRot(float val1, float val2, float parcent)
{
	int ival1 = (int)(val1 * 10.0f) % 3600;
	int ival2 = (int)(val2 * 10.0f) % 3600;
	if (ival1 < 0)
	{
		ival1 += 3600;
	}
	if (ival2 < 0)
	{
		ival2 += 3600;
	}
	int islr = ival1 - ival2;
	if (islr < 0)
	{
		islr += 3600;
	}
	int inewval;
	if (islr == 0)
	{
		inewval = ival1;
	}
	else
	{
		if (islr > 1800)
		{
			int isa = 3600 - islr;
			inewval = ival2 - ((float)isa * parcent);
		}
		else
		{
			int isa = islr;
			inewval = ival2 + ((float)isa * parcent);
		}
	}


	float newval = (float)inewval / 10.0f;
	return (newval);
}

//マスク用ステンシルバッファの初期化を行うか？
//インスタンスパーツとして再生する場合のみ設定する
void Player::setMaskFuncFlag(bool flg)
{
	_maskFuncFlag = flg;
}

//親のマスク対象
//インスタンスパーツとして再生する場合のみ設定する
//各パーツのマスク対象とアンドを取って処理する
void Player::setMaskParentSetting(bool flg)
{
	_maskParentSetting = flg;
}

/**
 * CustomSprite
 */

CustomSprite::CustomSprite():
	_opacity(1.0f)
	, _liveFrame(0.0f)
	, _hasPremultipliedAlpha(0)
	, refEffect(0)
	, _ssplayer(0)
	, effectAttrInitialized(false)
	, effectTimeTotal(0)
	, _maskInfluence(true)
{}

CustomSprite::~CustomSprite()
{
	//エフェクトクラスがある場合は解放する
	if (refEffect)
	{
		delete refEffect;
		refEffect = 0;
	}
}

CustomSprite* CustomSprite::create()
{
	CustomSprite *pSprite = new CustomSprite();
	if (pSprite && pSprite->init())
	{
		pSprite->initState();
		pSprite->autorelease();
		return pSprite;
	}
	CC_SAFE_DELETE(pSprite);
	return nullptr;
}

void CustomSprite::sethasPremultipliedAlpha(int PremultipliedAlpha)
{
	_hasPremultipliedAlpha = PremultipliedAlpha;
}

cocos2d::V3F_C4B_T2F_Quad& CustomSprite::getAttributeRef()
{
	return _quad;
}

void CustomSprite::setOpacity(GLubyte opacity)
{
	cocos2d::Sprite::setOpacity(opacity);
	_opacity = static_cast<float>(opacity) / 255.0f;
}

const cocos2d::Mat4& CustomSprite::getNodeToParentTransform() const
{
    if (_transformDirty)
    {
		// 自身の行列を更新
		Sprite::getNodeToParentTransform();
		
		// 更に親の行列に掛け合わせる
		if (_parent != nullptr)
		{
			cocos2d::Mat4 mat = _parent->_mat;
			mat = mat * _transform;
			_transform = mat;
		}
	}
	return _transform;
}


int cont = 0;
void CustomSprite::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	//SSPManegerのエフェクトアップデートを設定
	auto sspman = ss::SSPManager::getInstance();
	sspman->setUpdateFlag();

	cocos2d::Sprite::draw(renderer, transform, flags);

}

/**
* SSRenderTexture
*/
void SSRenderTexture::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	//SS5Manegerのエフェクトアップデートを設定
	auto sspman = ss::SSPManager::getInstance();
	sspman->setUpdateFlag();

	cocos2d::RenderTexture::draw(renderer, transform, flags);
	return;
}

SSRenderTexture* SSRenderTexture::create(int w, int h)
{

	SSRenderTexture *ret = new (std::nothrow) SSRenderTexture();

	if (ret && ret->initWithWidthAndHeight(w, h, cocos2d::Texture2D::PixelFormat::RGBA8888, 0))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}







};

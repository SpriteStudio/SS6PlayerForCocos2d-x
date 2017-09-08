//-----------------------------------------------------------
// SS6Player For Cocos2d-x v1.0.0
//
// Copyright(C) Web Technology Corp.
// http://www.webtech.co.jp/
//-----------------------------------------------------------
//
//  SS6Player.h
//

/************************************************************
Cocos2d-X Ver3.15.1に対応しています。
対応するssbpフォーマットはバージョン5です。
Ss6ConverterのフォーマットバージョンはSpriteStudio6SDKを参照してください。
https://github.com/SpriteStudio/SpriteStudio6-SDK/wiki/%E3%82%B3%E3%83%B3%E3%83%90%E3%83%BC%E3%82%BF%E3%81%AE%E4%BD%BF%E3%81%84%E6%96%B9

- Quick start

  #include "SS6Player.h"

  //X、Y回転を使用した場合にパースをかけない場合は
  //applicationDidFinishLaunchingb内で平行投影の設定を行ってください。
  director->setProjection(kCCDirectorProjection2D);

  //プレイヤーが共有するエフェクトバッファを作成します。
  //バッファは常駐されますのでゲーム起動時等に1度行ってください。
  auto sspman = ss::SSPManager::getInstance();
  sspman->createEffectBuffer(1024);			//エフェクト用バッファの作成

  -------

  auto resman = ss::ResourceManager::getInstance();
  resman->addData("sample.ssbp");			// ssbpの読み込み

  auto ssplayer = ss::Player::create();
  ssplayer->setData("sample");				// ssbpファイル（拡張子不要）をプレイヤーに関連づけます
  ssplayer->play("NewAnimation/anime1");	// アニメーション名指定(ssae名/アニメーション名)
  ssplayer->setPosition(200, 200);			// 位置設定
  ssplayer->setAlpha(255);					// 透明度設定
  ssplayer->setScale(1.0f,1.0f);			// 拡大率設定
  ssplayer->setRotation(0.0f);				// Z回転値設定(度)
  this->addChild(ssplayer);

  プレイヤーの制限についてはこちらのページを参照してください。
  https://github.com/SpriteStudio/SS6PlayerForCocos2d-x/wiki

  使用するアニメーションに合わせて Playerクラス定義部分にある設定用定数を変更してください。

*************************************************************/


#ifndef SS6Player_h
#define SS6Player_h

#include "cocos2d.h"
#include "SS6PlayerData.h"

//エフェクト関連
#include "./Common/loader/ssloader.h"
#include "./Common/Animator/ssplayer_macro.h"
#include "./Common/Animator/ssplayer_matrix.h"
#include "./Common/Animator/ssplayer_effectfunction.h"
#include "./Common/Animator/ssplayer_cellmap.h"
#include "./Common/Animator/ssplayer_PartState.h"
//#include "./Common/Animator/MersenneTwister.h"



namespace ss
{

class CellCache;
class CellRef;
class AnimeCache;
class AnimeRef;
struct ResourceSet;
struct ProjectData;
class Player;


/**
* State
*/
struct State
{
	std::string name;				/// パーツ名
	int flags;						/// このフレームで更新が行われるステータスのフラグ
	int cellIndex;					/// パーツに割り当てられたセルの番号
	float x;						/// アトリビュート：X座標
	float y;						/// アトリビュート：Y座標
	float z;						/// アトリビュート：Z座標
	float pivotX;					/// 原点Xオフセット＋セルに設定された原点オフセットX
	float pivotY;					/// 原点Yオフセット＋セルに設定された原点オフセットY
	float rotationX;				/// X回転（親子関係計算済）
	float rotationY;				/// Y回転（親子関係計算済）
	float rotationZ;				/// Z回転（親子関係計算済）
	float scaleX;					/// Xスケール（親子関係計算済）
	float scaleY;					/// Yスケール（親子関係計算済）
	float localscaleX;				/// Xローカルスケール
	float localscaleY;				/// Yローカルスケール
	int opacity;					/// 不透明度（0～255）（親子関係計算済）
	int localopacity;				/// ローカル不透明度（0～255）
	float size_X;					/// アトリビュート：Xサイズ
	float size_Y;					/// アトリビュート：Xサイズ
	float uv_move_X;				/// アトリビュート：UV X移動
	float uv_move_Y;				/// アトリビュート：UV Y移動
	float uv_rotation;				/// アトリビュート：UV 回転
	float uv_scale_X;				/// アトリビュート：UV Xスケール
	float uv_scale_Y;				/// アトリビュート：UV Yスケール
	float boundingRadius;			/// アトリビュート：当たり半径
	int partsColorFunc;				/// SS5アトリビュート：パーツカラーのブレンド方法
	int partsColorType;				/// SS5アトリビュート：パーツカラーの単色か頂点カラーか。
	int masklimen;					/// マスク強度
	int priority;					/// 優先度
	bool flipX;						/// 横反転（親子関係計算済）
	bool flipY;						/// 縦反転（親子関係計算済）
	bool isVisibled;				/// 非表示（親子関係計算済）
	float instancerotationX;		/// インスタンスパーツに設定されたX回転
	float instancerotationY;		/// インスタンスパーツに設定されたY回転
	float instancerotationZ;		/// インスタンスパーツに設定されたZ回転
	//インスタンスアトリビュート
	int			instanceValue_curKeyframe;
	int			instanceValue_startFrame;
	int			instanceValue_endFrame;
	int			instanceValue_loopNum;
	float		instanceValue_speed;
	int			instanceValue_loopflag;
	//エフェクトアトリビュート
	int			effectValue_curKeyframe;
	int			effectValue_startTime;
	float		effectValue_speed;
	int			effectValue_loopflag;

	void init()
	{
		flags = 0;
		cellIndex = 0;
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		pivotX = 0.0f;
		pivotY = 0.0f;
		rotationX = 0.0f;
		rotationY = 0.0f;
		rotationZ = 0.0f;
		scaleX = 1.0f;
		scaleY = 1.0f;
		localscaleX = 1.0f;
		localscaleY = 1.0f;
		opacity = 255;
		localopacity = 255;
		size_X = 1.0f;
		size_Y = 1.0f;
		uv_move_X = 0.0f;
		uv_move_Y = 0.0f;
		uv_rotation = 0.0f;
		uv_scale_X = 1.0f;
		uv_scale_Y = 1.0f;
		boundingRadius = 0.0f;
		masklimen = 0;
		priority = 0;
		partsColorFunc = 0;
		partsColorType = 0;
		flipX = false;
		flipY = false;
		isVisibled = false;
		instancerotationX = 0.0f;
		instancerotationY = 0.0f;
		instancerotationZ = 0.0f;
		instanceValue_curKeyframe = 0;
		instanceValue_startFrame = 0;
		instanceValue_endFrame = 0;
		instanceValue_loopNum = 0;
		instanceValue_speed = 0;
		instanceValue_loopflag = 0;
		effectValue_curKeyframe = 0;
		effectValue_startTime = 0;
		effectValue_speed = 0;
		effectValue_loopflag = 0;
	}

	State() { init(); }
};

/**
* CustomSprite
*/
class CustomSprite : public cocos2d::Sprite
{
private:

private:
	float				_opacity;
	int					_hasPremultipliedAlpha;
public:
	cocos2d::Mat4		_mat;
	cocos2d::Mat4		_localmat;
	State				_state;
	bool				_isStateChanged;
	CustomSprite*		_parent;
	ss::Player*			_ssplayer;
	float				_liveFrame;

	PartData			_partData;
	bool				_maskInfluence;		//親パーツのマスク対象を加味したマスク対象
											
	//エフェクト用パラメータ
	SsEffectRenderV2*	refEffect;
	SsPartState			partState;

	//モーションブレンド用ステータス
	State				_orgState;

	//エフェクト制御用ワーク
	bool effectAttrInitialized;
	float effectTimeTotal;

public:
	CustomSprite();
	virtual ~CustomSprite();

	static CustomSprite* create();

	void initState()
	{
		_mat = cocos2d::Mat4::IDENTITY;
		_state.init();
		_isStateChanged = true;
	}

	void setStateValue(float& ref, float value)
	{
		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}
	void setStateValue(int& ref, int value)
	{
		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setStateValue(bool& ref, bool value)
	{
		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setState(const State& state)
	{
		_state.name = state.name;
		setStateValue(_state.flags, state.flags);
		setStateValue(_state.cellIndex, state.cellIndex);
		setStateValue(_state.x, state.x);
		setStateValue(_state.y, state.y);
		setStateValue(_state.z, state.z);
		setStateValue(_state.pivotX, state.pivotX);
		setStateValue(_state.pivotY, state.pivotY);
		setStateValue(_state.rotationX, state.rotationX);
		setStateValue(_state.rotationY, state.rotationY);
		setStateValue(_state.rotationZ, state.rotationZ);
		setStateValue(_state.scaleX, state.scaleX);
		setStateValue(_state.scaleY, state.scaleY);
		setStateValue(_state.localscaleX, state.localscaleX);
		setStateValue(_state.localscaleY, state.localscaleY);
		setStateValue(_state.opacity, state.opacity);
		setStateValue(_state.localopacity, state.localopacity);
		setStateValue(_state.size_X, state.size_X);
		setStateValue(_state.size_Y, state.size_Y);
		setStateValue(_state.uv_move_X, state.uv_move_X);
		setStateValue(_state.uv_move_Y, state.uv_move_Y);
		setStateValue(_state.uv_rotation, state.uv_rotation);
		setStateValue(_state.uv_scale_X, state.uv_scale_X);
		setStateValue(_state.uv_scale_Y, state.uv_scale_Y);
		setStateValue(_state.boundingRadius, state.boundingRadius);
		setStateValue(_state.masklimen, state.masklimen);
		setStateValue(_state.priority, state.priority);
		setStateValue(_state.isVisibled, state.isVisibled);
		setStateValue(_state.flipX, state.flipX);
		setStateValue(_state.flipY, state.flipY);
		setStateValue(_state.partsColorFunc, state.partsColorFunc);
		setStateValue(_state.partsColorType, state.partsColorType);

		setStateValue(_state.instancerotationX, state.instancerotationX);
		setStateValue(_state.instancerotationY, state.instancerotationY);
		setStateValue(_state.instancerotationZ, state.instancerotationZ);

		setStateValue(_state.instanceValue_curKeyframe, state.instanceValue_curKeyframe);
		setStateValue(_state.instanceValue_startFrame, state.instanceValue_startFrame);
		setStateValue(_state.instanceValue_endFrame, state.instanceValue_endFrame);
		setStateValue(_state.instanceValue_loopNum, state.instanceValue_loopNum);
		setStateValue(_state.instanceValue_speed, state.instanceValue_speed);
		setStateValue(_state.instanceValue_loopflag, state.instanceValue_loopflag);
		setStateValue(_state.effectValue_curKeyframe, state.effectValue_curKeyframe);
		setStateValue(_state.effectValue_startTime, state.effectValue_startTime);
		setStateValue(_state.effectValue_speed, state.effectValue_speed);
		setStateValue(_state.effectValue_loopflag, state.effectValue_loopflag);
	}

	//マトリクスの更新フラグを設定する
	void Set_transformDirty()
	{
		/*
		cocos2d-x Ver3.13.1から
		void Node::setAdditionalTransform(const Mat4* additionalTransform)の内部で
		_transformDirtyフラグが立たなくなったので、個別に設定する
		*/
		_transformDirty = true;
	}


	// override
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	virtual void setOpacity(GLubyte opacity);

	// original functions
	cocos2d::V3F_C4B_T2F_Quad& getAttributeRef();
	void sethasPremultipliedAlpha(int PremultipliedAlpha);

public:
	// override
	virtual const cocos2d::Mat4& getNodeToParentTransform() const;
};

/**
* SSRenderTexture
*/
class SSRenderTexture : public cocos2d::RenderTexture
{
public:
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	static  SSRenderTexture* create(int w, int h);
};

/**
 * ResourceManager
 */
class ResourceManager : public cocos2d::Ref
{
public:
	static const std::string s_null;

	/**
	 * デフォルトインスタンスを取得します.
	 *
	 * @return デフォルトのResourceManagerインスタンス
	 */
	static ResourceManager* getInstance();

	/**
	 * ssbpファイルを読み込み管理対象とします.
	 * dataKeyはssbpのファイル名（拡張子なし）になります.
	 *
	 * @param  ssbpFilepath  ssbpファイルのパス
	 * @param  imageBaseDir  画像ファイルの読み込み元ルートパス. 省略時はssbpのある場所をルートとします.
	 *                       解像度によって画像ファイルを差し替える時などに使用してください。
	 * @return dataKey
	 */
	std::string addData(const std::string& ssbpFilepath, const std::string& imageBaseDir = s_null);

	/**
	 * ssbpファイルを読み込み管理対象とします.
	 *
	 * @param  dataKey       dataKeyの指定
	 * @param  ssbpFilepath  ssbpファイルのパス
	 * @param  imageBaseDir  画像ファイルの読み込み元ルートパス. 省略時はssbpのある場所をルートとします.
	 * @return dataKey
	 */
	std::string addDataWithKey(const std::string& dataKey, const std::string& ssbpFilepath, const std::string& imageBaseDir = s_null);

	/**
	 * 指定されたssbpデータを管理対象とします.
	 *
	 * @param  dataKey       dataKeyの指定
	 * @param  data          ssbpデータ
	 * @param  imageBaseDir  画像ファイルの読み込み元ルートパス. 省略時はssbpのある場所をルートとします.
	 * @return dataKey
	 */
	std::string addData(const std::string& dataKey, const ProjectData* data, const std::string& imageBaseDir = s_null);
	
	/**
	 * 指定データを解放します.
	 * 指定したssbpファイルで管理しているテクスチャも解放されます。
	 * パス、拡張子を除いたssbp名を指定してください。
	 *
	 * @param  dataKey
	 */
	void removeData(const std::string& ssbpName);

	/**
	 * ssbp、テクスチャなどの全てのデータを解放します.
	 */
	void removeAllData();

	/**
	* 指定したssbpで読み込まれたテクスチャを解放します.
	*/
	bool releseTexture(char* ssbpName);

	/**
	* 名前に対応するデータ取得します.
	*/
	ResourceSet* getData(const std::string& dataKey);

	/**
	* 指定したセルのテクスチャを変更します.
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  ssceName       ssce名（拡張子を除くファイル名）
	* @param  texture        変更後のテクスチャ
    *                        テクスチャはポインタを参照するだけなので、使用する側で実体を管理する必要があります。
	* @return 変更を行ったか
	*/
	bool changeTexture(char* ssbpName, char* ssceName, cocos2d::Texture2D* texture);

	/**
	* セルとして読み込んだテクスチャを取得します.
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  ssceName       ssce名（拡張子を除くファイル名）
	* @return テクスチャのポインタ
	*/
	cocos2d::Texture2D* getTexture(char* ssbpName, char* ssceName);

	/**
	* 読み込んでいるssbpからアニメーションの開始フレーム数を取得します。
	* 読み込んでいるssbpからアニメーションの総フレーム数を取得します。
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  animeName      ssae/モーション名
	* @return アニメーションの開始フレーム（存在しない場合はアサート）
	* @return アニメーションの総フレーム（存在しない場合はアサート）
	*/
	int getStartFrame(std::string ssbpName, std::string animeName);

	/**
	* 読み込んでいるssbpからアニメーションの終了フレーム数を取得します。
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  animeName      ssae/モーション名
	* @return アニメーションの終了フレーム（存在しない場合はアサート）
	*/
	int getEndFrame(std::string ssbpName, std::string animeName);

	/**
	* 読み込んでいるssbpからアニメーションの総フレーム数を取得します。
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  animeName      ssae/モーション名
	* @return アニメーションの総フレーム（存在しない場合はアサート）
	*/
	int getTotalFrame(std::string ssbpName, std::string animeName);

	/**
	* 名前が登録されていればtrueを返します
	*
	* @param dataKey
	* @return
	*/
	bool isDataKeyExists(const std::string& dataKey);

	/**
	 * 新たなResourceManagerインスタンスを構築します.
	 *
	 * @return ResourceManagerインスタンス
	 */
	static ResourceManager* create();


public:
	ResourceManager(void);
	virtual ~ResourceManager();

protected:
	std::map<std::string, ResourceSet*>	_dataDic;
};


/**
* SSPManager
*/
class SSPManager : public cocos2d::Ref
{
public:

	/**
	* デフォルトインスタンスを取得します.
	*
	* @return デフォルトのSSPManagerインスタンス
	*/
	static SSPManager* getInstance();

	/**
	* マネージャ定時処理
	* 各シーンのアップデートで必ず呼び出してください。
	* 
	*/
	void update();

	/**
	* 新たなSSPManagerインスタンスを構築します.
	*
	* @return SSPManagerインスタンス
	*/
	static SSPManager* create();

	/**
	* エフェクト用バッファを作成します。
	* この関数を呼び出さない場合エフェクトは表示されません。
	* 作成したバッファを全プレイヤーで共用します。
	* バッファサイズを超える個数のパーティクルが表示されると途中から表示されなくなります。
	* エフェクト機能を使用する場合はゲームの初期化等で1度呼び出してください。
	* 再度呼び出すと使用するバッファをリサイズします。
	*
	* @param  buffSize       作成するバッファサイズ
	*/
	void createEffectBuffer( int buffSize = 1024);


	/**
	* 空いているエフェクト用バッファを取得します。
	* プレイヤー内部で使用されます。ゲームから呼び出す必要はありません
	*/
	CustomSprite* getEffectBuffer();

	/**
	* 次回アップデートが必要な事を設定します。
	* プレイヤー内部で使用されます。ゲームから呼び出す必要はありません
	*/
	void setUpdateFlag();

	/**
	* オフスクリーンレンダリング中かを設定します。
	* プレイヤー内部で使用されます。ゲームから呼び出す必要はありません
	*/
	void setUseOffscreenRendering(bool use);


public:
	SSPManager(void);
	virtual ~SSPManager();


protected:
	//エフェクト用データ
	cocos2d::Vector<CustomSprite*>	_effectSprite;		
	int								_effectSpriteCount;	
	bool							_isUpdate;
	bool							_useOffscreenRendering;

	void releseEffectBuffer();
};

/**
 * UserData
 */
struct UserData
{
	enum {
		FLAG_INTEGER	= 1 << 0,
		FLAG_RECT		= 1 << 1,
		FLAG_POINT		= 1 << 2,
		FLAG_STRING		= 1 << 3
	};

	const char*	partName;		// Part name
	int			frameNo;		// Frame no

	int			flags;			// Flags of valid data
	int			integer;		// Integer
	int			rect[4];		// Rectangle Left, Top, Right, Bottom
	int			point[2];		// Position X, Y
	const char*	str;			// String (zero terminated)
	int			strSize;		// String size (byte count)
};


/**
* LabelData
*/
struct LabelData
{
	std::string	str;			// String (zero terminated)
	int			strSize;		// String size (byte count)
	int			frameNo;		// Frame no
};

struct Instance
{
	int			refStartframe;		//開始フレーム
	int			refEndframe;		//終了フレーム
	float		refSpeed;			//再生速度
	int			refloopNum;			//ループ回数
	bool		infinity;			//無限ループ
	bool		reverse;			//逆再選
	bool		pingpong;			//往復
	bool		independent;		//独立動作
	void clear(void)
	{
		refStartframe = 0;			//開始フレーム
		refEndframe = 1;			//終了フレーム
		refSpeed = 1;				//再生速度
		refloopNum = 1;				//ループ回数
		infinity = false;			//無限ループ
		reverse = false;			//逆再選
		pingpong = false;			//往復
		independent = false;		//独立動作
	}
};

/**
* ResluteState
*/
struct ResluteState
{
	int flags;						/// このフレームで更新が行われるステータスのフラグ
	int cellIndex;					/// パーツに割り当てられたセルの番号
	float x;						/// 画面上での表示位置X
	float y;						/// 画面上での表示位置Y
	float z;						/// アトリビュート：Z座標
	float pivotX;					/// 原点Xオフセット＋セルに設定された原点オフセットX
	float pivotY;					/// 原点Yオフセット＋セルに設定された原点オフセットY
	float rotationX;				/// アトリビュート：X回転
	float rotationY;				/// アトリビュート：Y回転
	float rotationZ;				/// アトリビュート：Z回転
	float scaleX;					/// アトリビュート：Xスケール
	float scaleY;					/// アトリビュート：Yスケール
	float localscaleX;				/// Xローカルスケール
	float localscaleY;				/// Yローカルスケール
	int opacity;					/// 不透明度（0～255）（親子関係計算済）
	int localopacity;				/// ローカル不透明度（0～255）
	float size_X;					/// アトリビュート：Xサイズ
	float size_Y;					/// アトリビュート：Yサイズ
	float scaledsize_X;				/// 画面上のXサイズ
	float scaledsize_Y;				/// 画面上のYサイズ
	float uv_move_X;				/// アトリビュート：UV X移動
	float uv_move_Y;				/// アトリビュート：UV Y移動
	float uv_rotation;				/// アトリビュート：UV 回転
	float uv_scale_X;				/// アトリビュート：UV Xスケール
	float uv_scale_Y;				/// アトリビュート：UV Yスケール
	float boundingRadius;			/// アトリビュート：当たり半径
	int	priority;					/// SS6アトリビュート：優先度
	int partsColorFunc;				/// SS6アトリビュート：カラーブレンドのブレンド方法
	int partsColorType;				/// SS6アトリビュート：カラーブレンドの単色か頂点カラーか。
	bool flipX;						/// 横反転（親子関係計算済）
	bool flipY;						/// 縦反転（親子関係計算済）
	bool isVisibled;				/// 非表示（親子関係計算済）

	int	part_type;					//パーツ種別
	int	part_boundsType;			//当たり判定種類
	int	part_alphaBlendType;		// BlendType
	int	part_labelcolor;			// ラベルカラー
};

//含まれるパーツデータフラグ
enum {
	PART_FLAG_INVISIBLE			= 1 << 0,		/// 非表示
	PART_FLAG_FLIP_H			= 1 << 1,		/// 横反転
	PART_FLAG_FLIP_V			= 1 << 2,		/// 縦反転

	// optional parameter flags
	PART_FLAG_CELL_INDEX		= 1 << 3,		/// セル番号
	PART_FLAG_POSITION_X		= 1 << 4,		/// X座標
	PART_FLAG_POSITION_Y		= 1 << 5,		/// Y座標
	PART_FLAG_POSITION_Z		= 1 << 6,		/// Z座標
	PART_FLAG_PIVOT_X			= 1 << 7,		/// 原点オフセットX
	PART_FLAG_PIVOT_Y           = 1 << 8,		/// 原点オフセットY
	PART_FLAG_ROTATIONX			= 1 << 9,		/// X回転
	PART_FLAG_ROTATIONY			= 1 << 10,		/// Y回転
	PART_FLAG_ROTATIONZ			= 1 << 11,		/// Z回転
	PART_FLAG_SCALE_X			= 1 << 12,		/// スケールX
	PART_FLAG_SCALE_Y			= 1 << 13,		/// スケールY
	PART_FLAG_LOCALSCALE_X		= 1 << 14,		/// ローカルスケールX
	PART_FLAG_LOCALSCALE_Y		= 1 << 15,		/// ローカルスケールY
	PART_FLAG_OPACITY			= 1 << 16,		/// 不透明度
	PART_FLAG_LOCALOPACITY		= 1 << 17,		/// ローカル不透明度
	PART_FLAG_PARTS_COLOR		= 1 << 18,		/// パーツカラー
	PART_FLAG_VERTEX_TRANSFORM	= 1 << 19,		/// 頂点変形

	PART_FLAG_SIZE_X			= 1 << 20,		/// サイズX
	PART_FLAG_SIZE_Y			= 1 << 21,		/// サイズY

	PART_FLAG_U_MOVE			= 1 << 22,		/// UV移動X
	PART_FLAG_V_MOVE			= 1 << 23,		/// UV移動Y
	PART_FLAG_UV_ROTATION		= 1 << 24,		/// UV回転
	PART_FLAG_U_SCALE			= 1 << 25,		/// UVスケールX
	PART_FLAG_V_SCALE			= 1 << 26,		/// UVスケールY
	PART_FLAG_BOUNDINGRADIUS	= 1 << 27,		/// 当たり半径

	PART_FLAG_MASK				= 1 << 28,		/// マスク強度
	PART_FLAG_PRIORITY			= 1 << 29,		/// 優先度

	PART_FLAG_INSTANCE_KEYFRAME	= 1 << 30,		/// インスタンス
	PART_FLAG_EFFECT_KEYFRAME   = 1 << 31,		/// エフェクト

	NUM_PART_FLAGS
};

//頂点変形フラグ
enum {
	VERTEX_FLAG_LT = 1 << 0,
	VERTEX_FLAG_RT = 1 << 1,
	VERTEX_FLAG_LB = 1 << 2,
	VERTEX_FLAG_RB = 1 << 3,
	VERTEX_FLAG_ONE = 1 << 4	// color blend only
};

//インスタンスフラグ
enum {
	INSTANCE_LOOP_FLAG_INFINITY = 1 << 0,
	INSTANCE_LOOP_FLAG_REVERSE = 1 << 1,
	INSTANCE_LOOP_FLAG_PINGPONG = 1 << 2,
	INSTANCE_LOOP_FLAG_INDEPENDENT = 1 << 3,
};

//エフェクトアトリビュートのループフラグ
enum {
	EFFECT_LOOP_FLAG_INDEPENDENT = 1 << 0,
};

/// Animation Part Type
enum
{
	PARTTYPE_INVALID = -1,
	PARTTYPE_NULL,			/// null。領域を持たずSRT情報のみ。ただし円形の当たり判定は設定可能。
	PARTTYPE_NORMAL,		/// 通常パーツ。領域を持つ。画像は無くてもいい。
	PARTTYPE_TEXT,			/// テキスト(予約　未実装）
	PARTTYPE_INSTANCE,		/// インスタンス。他アニメ、パーツへの参照。シーン編集モードの代替になるもの
	PARTTYPE_ARMATURE,		///< ボーンパーツ
	PARTTYPE_EFFECT,		///< ss5.5対応エフェクトパーツ
	PARTTYPE_MESH,			///< メッシュパーツ
	PARTTYPE_MOVENODE,		///< 動作起点
	PARTTYPE_CONSTRAINT,	///< コンストレイント
	PARTTYPE_MASK,			///< マスク
	PARTTYPE_JOINT,			///< メッシュとボーンの関連付けパーツ
	PARTTYPE_BONEPOINT,		///< ボーンポイント
	PARTTYPE_NUM
};

//当たり判定の種類
enum
{
	INVALID = -1,
	NONE,			///< 当たり判定として使わない。
	QUAD,			///< 自在に変形する四辺形。頂点変形など適用後の４角を結んだ領域。最も重い。
	AABB,			///< 回転しない全体を囲む矩形で交差判定
	CIRCLE,			///< 真円の半径で距離により判定する
	CIRCLE_SMIN,	///< 真円の半径で距離により判定する (スケールはx,yの最小値をとる）
	CIRCLE_SMAX,	///< 真円の半径で距離により判定する (スケールはx,yの最大値をとる）
	num
};

//αブレンド方法
enum BlendType
{
	BLEND_MIX,		///< 0 ブレンド（ミックス）
	BLEND_MUL,		///< 1 乗算
	BLEND_ADD,		///< 2 加算
	BLEND_SUB,		///< 3 減算
	BLEND_MULALPHA, ///< 4 α乗算
	BLEND_SCREEN, 	///< 5 スクリーン
	BLEND_EXCLUSION,///< 6 除外
	BLEND_INVERT, 	///< 7 反転
	BLEND_NUM,
};
/*
Common\Loader\sstypes.hに実際の定義があります。
/// テクスチャラップモード
namespace SsTexWrapMode
{
	enum _enum
	{
		invalid = -1,	/// なし
		clamp,			/// クランプする
		repeat,			/// リピート
		mirror,			/// ミラー
		num
	};
};

/// テクスチャフィルターモード 画素補間方法
namespace SsTexFilterMode
{
	enum _enum
	{
		invalid = -1,
		nearlest,	///< ニアレストネイバー
		linear,		///< リニア、バイリニア
		num
	};
};
*/

//カラーラベル定数
#define COLORLABELSTR_NONE		""
#define COLORLABELSTR_RED		"Red"
#define COLORLABELSTR_ORANGE	"Orange"
#define COLORLABELSTR_YELLOW	"Yellow"
#define COLORLABELSTR_GREEN		"Green"
#define COLORLABELSTR_BLUE		"Blue"
#define COLORLABELSTR_VIOLET	"Violet"
#define COLORLABELSTR_GRAY		"Gray"
enum 
{
	COLORLABEL_NONE,		///< 0 なし
	COLORLABEL_RED,			///< 1 赤
	COLORLABEL_ORANGE,		///< 2 オレンジ
	COLORLABEL_YELLOW,		///< 3 黄色
	COLORLABEL_GREEN,		///< 4 緑
	COLORLABEL_BLUE,		///< 5 青
	COLORLABEL_VIOLET,		///< 6 紫
	COLORLABEL_GRAY,		///< 7 灰色
};

//------------------------------------------------------------------------------
//プレイヤーの設定定義
//使用するアニメーションに合わせて設定してください。

//プレイヤーで扱えるアニメに含まれるパーツの最大数
#define PART_VISIBLE_MAX (1024)

//------------------------------------------------------------------------------



/**
 * Player
 */
class Player : public cocos2d::Sprite
{
public:

	/**
	 * Playerインスタンスを構築します.
	 *
	 * @param  resman  使用するResourceManagerインスタンス. 省略時はデフォルトインスタンスが使用されます.
	 * @return Playerインスタンス
	 */
	static Player* create(ResourceManager* resman = nullptr);

	/**
	 * 使用するResourceManagerインスタンスを設定します.
	 *
	 * @param  resman  使用するResourceManagerインスタンス. 省略時はデフォルトインスタンスが使用されます.
	 */
	void setResourceManager(ResourceManager* resman = nullptr);

	/**
	 * 使用中のResourceManagerインスタンスを解放します.
	 * 再度ResourceManagerインスタンスを設定するまでは再生できなくなります.
	 */
	void releaseResourceManager();


	/**
	* 使用するSSPManagerインスタンスを設定します.
	*
	*/
	void setSSPManager();


	/**
	 * 再生するssbpデータのdataKeyを設定します.
	 *
	 * @param  dataKey  再生するデータのdataKey
	 */
	void setData(const std::string& dataKey);

	/**
	 * 設定されているssbpデータを解放します.
	 */
	void releaseData();

	/**
	 * 設定されているアニメーションを解放します.
	 */
	void releaseAnime();

	/**
	 * アニメーションの再生を開始します.
	 *
	 * @param  ssaeName      パック名(ssae名）
	 * @param  motionName    再生するモーション名
	 * @param  loop          再生ループ数の指定. 省略時は0
	 * @param  startFrameNo  再生を開始するフレームNoの指定. 省略時は0
	 */
	void play(const std::string& ssaeName, const std::string& motionName, int loop = 0, int startFrameNo = 0);

	/**
	 * アニメーションの再生を開始します.
	 * アニメーション名から再生するデータを選択します.
	 * "ssae名/モーション名で指定してください.
	 * sample.ssaeのanime_1を指定する場合、sample/anime_1となります.
	 * ※ver1.1からモーション名のみで指定する事はできなくなりました。
	 *
	 * @param  animeName     再生するアニメーション名
	 * @param  loop          再生ループ数の指定. 省略時は0
	 * @param  startFrameNo  再生を開始するフレームNoの指定. 省略時は0
	 */
	void play(const std::string& animeName, int loop = 0, int startFrameNo = 0);

	/**
	* 現在再生しているモーションとブレンドしながら再生します。
	* アニメーション名から再生するデータを選択します.
	* "ssae名/モーション名で指定してください.
	* sample.ssaeのanime_1を指定する場合、sample/anime_1となります.
	* ※ver1.1からモーション名のみで指定する事はできなくなりました。
	*
	* ブレンドするアニメーションの条件は以下になります。
	* ・同じssbp内に含まれている事
	* ・同じパーツ構成（パーツ順、パーツ数）である事
	* ・オフスクリーンレンダリングを行っていない
	* SpriteStudioのフレームコントロールに並ぶパーツを上から順にブレンドしていきます。
	* パーツ名等のチェックは行なっていませんので遷移元と遷移先アニメのパーツの順番を同じにする必要があります。
	* 遷移元と遷移先のパーツ構成があっていない場合、正しくブレンドされませんのでご注意ください。
	*
	* 合成されるアトリビュートは
	* 座標X、座標Y、X回転、Y回転、Z回転、スケールX、スケールYのみです。
	* それ以外のアトリビュートは遷移先アニメの値が適用されます。
	* インスタンスパーツが参照しているソースアニメはブレンドされません。
	* エフェクトパーツから発生したパーティクルはブレンドされません。
	*
	*
	* @param  animeName     再生するアニメーション名
	* @param  loop          再生ループ数の指定. 省略時は0
	* @param  startFrameNo  再生を開始するフレームNoの指定. 省略時は0
	* @param  blendTime		モーションブレンドを行う時間、単位は秒　省略時は1秒
	*/
	void motionBlendPlay(const std::string& animeName, int loop = 0, int startFrameNo = 0, float blendTime = 0.1f);

	/**
	 * 再生を中断します.
	 */
	void animePause();

	/**
	 * 再生を再開します.
	 */
	void animeResume();

	/**
	 * 再生を停止します.
	 * ゲーム側でアニメーションの表示フレームを制御する場合はstop()を呼び出した後
	 * ゲーム側の更新処理でsetFrameNo()を呼び出し指定のフレームを表示してください。
	 */
	void stop();

	/**
	 * 再生しているアニメーションのパック名(ssae)を返します.
	 *
	 * @return パック名(ssae)
	 */
	const std::string& getPlayPackName() const;

	/**
	 * 再生しているアニメーション名を返します.
	 *
	 * @return アニメーション名
	 */
	const std::string& getPlayAnimeName() const;
	
	/**
	* アニメーションの開始フレームを取得します.
	* アニメーションの総フレームを取得します.
	*
	* @return 開始フレーム
	* @return 総フレーム
	*/
	int getStartFrame() const;

	/**
	* アニメーションの終了フレームを取得します.
	*
	* @return 終了フレーム
	*/
	int getEndFrame() const;

	/**
	* アニメーションの総フレームを取得します.
	*
	* @return 総フレーム
	*/
	int getTotalFrame() const;

	/**
	 * 再生フレームNoを取得します.
	 * Get frame no of playing.
	 *
	 * @return 再生フレームNo. frame no.
	 */
	int getFrameNo() const;

	/**
	 * 再生フレームNoを設定します.
	 * Set frame no of playing.
	 *
	 * @param 再生フレームNo. frame no.
	 */
	void setFrameNo(int frameNo);

	/**
	 * 再生スピードを取得します. (1.0f:標準)
	 * Set speed to play. (1.0f:normal speed)
	 */
	float getStep() const;

	/**
	 * 再生スピードを設定します. (1.0f:標準)
	 * Get speed to play. (1.0f:normal speed)
	 */
	void setStep(float step);
	
	/** 
	 * 指定されている再生ループ回数を取得します. (0:指定なし)
	 * Get a playback loop count of specified. (0:not specified)
	 */
	int getLoop() const;

	/** 
	 * 再生ループ回数を設定します. (0:指定なし)
	 * Set a playback loop count.  (0:not specified)
	 */
	void setLoop(int loop);

	/** 
	 * 現在までのループ再生回数を取得します.
	 * Get repeat count a playback.
	 */
	int getLoopCount() const;

	/** 
	 * 現在までのループ再生回数をクリアします.
	 * Clear repeat count a playback.
	 */
	void clearLoopCount();

	/**
	 * フレームスキップ（フレームレートに合わせ再生フレームをスキップする）の設定をします. (default: true)
	 * Set frame-skip(to skip the playback frame according to the frame rate). (default: true)
	 */
	void setFrameSkipEnabled(bool enabled);
	
	/** 
	 * フレームスキップの設定状態を返します.
	 * Get frame-skip setting.
	 */
	bool isFrameSkipEnabled() const;

	/**
	* ラベル名からフレーム位置を取得します.
	*/
	int getLabelToFrame(char* findLabelName);

	/**
	* 再生しているアニメーションに含まれるパーツ数を取得します.
	*/
	int getPartsCount(void);

	/**
	* indexからパーツ名を取得します.
	*/
	const char* getPartName(int partId) const;

	/**
	* パーツ名からindexを取得します.
	*/
	int indexOfPart(const char* partName) const;

	/**
	* パーツの名から、パーツ情報を取得します.
	*
	* @param  result        パーツ情報を受け取るバッファ
	* @param  name          取得するパーツ名
	* @param  frameNo       取得するフレーム番号 -1の場合は現在再生しているフレームが適用される
	*/
	bool getPartState(ResluteState& result, const char* name, int frameNo = -1);

	/**
	* パーツ名からパーツの表示、非表示を設定します.
	* コリジョン用のパーツや差し替えグラフィック等、SSP上で表示を行うがゲーム中では非表示にする場合に使用します。
	* SSの非表示アトリビュート設定するわけではないので注意してください。
	*/
	void setPartVisible(std::string partsname, bool flg);

	/**
	* パーツ名からパーツに割り当たるセルを変更します.
	* この関数で設定したパーツは参照セルアトリビュートの影響をうけません。
	* アニメに設定されたセルに戻す場合は、セル名に""を指定してください。
	*
	* @param  partsname         パーツ名
	* @param  sscename          セルマップ名
	* @param  cellname          表示させたいセル名
	*/
	void setPartCell(std::string partsname, std::string sscename, std::string cellname);

	/*
	* プレイヤーの透明度を設定します(0～255).
	* setOpacityではなくこちらを使用してください。
	*/
	void setAlpha(int alpha);

	/*
	* アニメの輝度を設定します.
	* setColor(Color3B)ではなくこちらを使用してください。
	* 制限としてカラーブレンドが適用されたパーツの色は変更できませんので注意してください。
	* 
	* @param  r          赤成分(0～255)
	* @param  g          緑成分(0～255)
	* @param  b          青成分(0～255)
	*/
	void setColor(int r, int g, int b );

	/*
	* setContentScaleFactorの数値に合わせて内部のUV補正を有効にするか設定します。
	* マルチ解像度テクスチャ対応を行う際にプレイヤーの画像はそのまま使用する場合は、trueを設定してプレイヤー内UV値を変更してください.
	* 画像を差し替える場合はaddDataの第二引数でパスを指定し、解像度の違うテクスチャを読み込んでください.
	*/
	void setContentScaleEneble(bool eneble);

	/*
	* 名前を指定してパーツの再生するインスタンスアニメを変更します。
	* 指定したパーツがインスタンスパーツでない場合、falseを返します.
	* インスタンスパーツ名はディフォルトでは「ssae名:モーション名」とつけられています。
	* 再生するアニメの名前は"ssae名/アニメーション名"として再生してください。
	* 現在再生しているアニメを指定することは入れ子となり無限ループとなるためできません。
	* 変更するアニメーションは同じssbpに含まれる必要があります。
	*
	* インスタンスキーを手動で設定する事が出来ます。
	* アニメーションに合わせて開始フレーム、終了フレーム等のインスタンスアトリビュート情報を設定してください。
	* 終了フレーム最大値は総フレーム-1になります。
	* 上書きフラグがfalseの場合、SS上に設定されたインスタンスアトリビュートの設定を使用します。
	* 使用例：
	* ss::Instance param;
	* param.clear();
	* param.refEndframe = resman->getMaxFrame("ssbp名","ssae名/モーション名") - 1;	//アニメーションの長さを取得
	* param.infinity = true;														//無限ループを設定
	* ssplayer->changeInstanceAnime("再生しているアニメーションに含まれるインスタンスパーツ名", "ssae名/モーション名", true, param);
	*
	* @param  partsname			SS上のパーツ名
	* @param  animeName			参照するアニメ名
	* @param  overWrite			インスタンスキーの上書きフラグ
	* @param  keyParam			インスタンスキーのパラメータ
	*/
	bool changeInstanceAnime(std::string partsname, std::string animeName, bool overWrite, Instance keyParam);

	/*
	* プレイヤーにインスタンスパラメータを設定します。
	*
	* @param  overWrite			インスタンスキーの上書きフラグ
	* @param  keyParam			インスタンスキーのパラメータ
	*/
	void setInstanceParam(bool overWrite, Instance keyParam);

	/*
	* プレイヤーからインスタンスパラメータを取得します。
	*
	* @param  overWrite			インスタンスキーの上書きフラグ
	* @param  keyParam			インスタンスキーのパラメータ
	*/
	void getInstanceParam(bool *overWrite, Instance *keyParam);

	/*
	* アニメーションのループ範囲（再生位置）を上書きします。
	*
	* @param  frame			開始フレーム（-1で上書き解除）
	*/
	void setStartFrame(int frame);

	/*
	* アニメーションのループ範囲（終了位置）を上書きします。
	* SpriteStudioのフレーム数+1を設定してください。
	*
	* @param  frame			終了フレーム（-1で上書き解除）
	*/
	void setEndFrame(int frame);

	/*
	* アニメーションのループ範囲（再生位置）を上書きします。
	*
	* @param  labelname			開始フレームとなるラベル名（""で上書き解除）
	*/
	void setStartFrameToLabelName(char *findLabelName);

	/*
	* アニメーションのループ範囲（終了位置）を上書きします。
	*
	* @param  labelname			終了フレームとなるラベル名（""で上書き解除）
	*/
	void setEndFrameToLabelName(char *findLabelName);

	/*
	* オフスクリーンレンダリングを有効にします。
	* 有効時は指定したサイズでクリッピングされます。
	* 一度アニメーションを仮想レンダーにレンダリングしてから描画するため負荷がかかります。
	* サイズ、基準位置を省略した場合、SSで設定した基準枠の範囲が適用されます。
	* 基準位置は横方向の場合 width * pivotX の分がずれて描画されます。
	*
	* @param  flag				有効：true、無効：false
	* @param  width				クリッピングするサイズ（横幅）
	* @param  height			クリッピングするサイズ（高さ）
	* @param  pivotX			基準位置X（-0.5：左～0：中央～+0.5：右）
	* @param  pivotY			基準位置Y（-0.5：下～0：中央～+0.5：上）
	*/
	void offScreenRenderingEnable(bool enable, float width = 0.0f, float height = 0.0f, float pivotX = -1000.0f, float pivotY = -1000.0f);

	/*
	* パーツ番号に対応したスプライト情報を取得します。
	*
	* @param  partIndex			パーツ番号
	*/
	CustomSprite* getSpriteData(int partIndex);

	typedef std::function<void(Player*, const UserData*)> UserDataCallback;
	typedef std::function<void(Player*)> PlayEndCallback;
	typedef std::function<void(Player*)> ErrorCallback;

	/** 
	 * ユーザーデータを受け取るコールバックを設定します.
	 * 再生したフレームにユーザーデータが設定されている場合呼び出されます。
	 * プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
	 * 
	 * コールバック内でパーツのステータスを取得したい場合は、この時点ではアニメが更新されていないため、
	 * getPartStateに data->frameNo でフレーム数を指定して取得してください。
	 * //再生しているモーションに含まれるパーツ名「collision」のステータスを取得します。
	 * ss::ResluteState result;
	 * ssplayer->getPartState(result, "collision", data->frameNo);
	 * 
	 * コールバック内でアニメーションの再生フレーム変更したい場合は
	 * 次に行われるゲームのアップデート内でプレイヤーに対してアニメーションの操作をしてください。
	 *
	 * @param  callback  ユーザーデータ受け取りコールバック
	 *
     * @code
	 * player->setUserDataCallback(CC_CALLBACK_2(MyScene::userDataCallback, this));
	 * --
	 * void MyScene::userDataCallback(ss::Player* player, const ss::UserData* data)
	 * {
	 *   ...
	 * }
     * @endcode
	 */
	void setUserDataCallback(const UserDataCallback& callback);

	/**
	 * 再生終了時に呼び出されるコールバックを設定します.
	 * 再生したアニメーションが終了した段階で呼び出されます。
	 * プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
	 * player->getPlayAnimeName();
	 * を使用する事で再生しているアニメーション名を取得する事もできます。
	 *
	 * ループ回数分再生した後に呼び出される点に注意してください。
	 * 無限ループで再生している場合はコールバックが発生しません。
	 *
	 * コールバック内でアニメーションの再生フレーム変更したい場合は
	 * 次に行われるゲームのアップデート内でプレイヤーに対してアニメーションの操作をしてください。
	 *
	 * @param  callback  再生終了受け取りコールバック
	 *
     * @code
	 * player->setPlayEndCallback(CC_CALLBACK_1(MyScene::playEndCallback, this));
	 * --
	 * void MyScene::playEndCallback(ss::Player* player)
	 * {
	 *   ...
	 * }
     * @endcode
	 */
	void setPlayEndCallback(const PlayEndCallback& callback);

	/*
	* rootパーツの状態を決めるマトリクスを設定します。
	*
	* @param  mat			与えるマトリクス
	* @param  use			マトリクスを適用するか？
	*
	*/
	void setParentMatrix(cocos2d::Mat4, bool use);

public:
	Player(void);
	virtual ~Player();

	// override
	virtual bool init();
	virtual void update(float dt);
	virtual void setGlobalZOrder(float globalZOrder);

protected:
	void allocParts(int numParts, bool useCustomShaderProgram);
	void releaseParts();
	void setPartsParentage();

	void play(AnimeRef* animeRef, int loop, int startFrameNo);
	void updateFrame(float dt);
	void setFrame(int frameNo, float dt = 0.0f);
	void checkUserData(int frameNo);
	void get_uv_rotation(float *u, float *v, float cu, float cv, float deg);
	void set_InstanceRotation(float rotX, float rotY, float rotZ);
	float parcentVal(float val1, float val2, float parcent);
	float parcentValRot(float val1, float val2, float parcent);
	void setMaskFuncFlag(bool flg);
	void setMaskParentSetting(bool flg);

protected:
	ResourceManager*	_resman;
	ResourceSet*		_currentRs;
	std::string			_currentdataKey;
	std::string			_currentAnimename;
	AnimeRef*			_currentAnimeRef;
	cocos2d::Vector<cocos2d::Sprite*>	_parts;

	SSPManager*			_sspman;

	bool				_frameSkipEnabled;
	float				_playingFrame;
	float				_step;
	int					_loop;
	int					_loopCount;
	bool				_isPlaying;
	bool				_isPausing;
	bool				_isPlayFirstUserdataChack;
	bool				_isPlayFirstUpdate;
	bool				_isContentScaleFactorAuto;
	int					_prevDrawFrameNo;
	bool				_partVisible[PART_VISIBLE_MAX];
	int					_cellChange[PART_VISIBLE_MAX];
	int					_partIndex[PART_VISIBLE_MAX];
	int					_InstanceAlpha;
	float				_InstanceRotX;
	float				_InstanceRotY;
	float				_InstanceRotZ;
	int					_animefps;
	int					_col_r;
	int					_col_g;
	int					_col_b;
	bool				_instanceOverWrite;		//インスタンス情報を上書きするか？
	Instance			_instanseParam;			//インスタンスパラメータ
	cocos2d::RenderTexture*	_offScreentexture;
	float				_offScreenWidth;
	float				_offScreenHeight;
	float				_offScreenPivotX;
	float				_offScreenPivotY;
	Player*				_motionBlendPlayer;
	float				_blendTime;
	float				_blendTimeMax;
	int					_startFrameOverWrite;	//開始フレームの上書き設定
	int					_endFrameOverWrite;		//終了フレームの上書き設定
	int					_seedOffset;			//エフェクトシードオフセット

	cocos2d::Mat4		_parentMat;					//プレイヤーが持つ継承されたマトリクス
	bool				_parentMatUse;					//プレイヤーが持つ継承されたマトリクスがあるか？
	bool				_maskParentSetting;		//親パーツのマスク対象（インスタンスのみ使用する）
	bool				_maskFuncFlag;			//マスク機能を有効にするか？（インスタンスのソースアニメはマスクが無効になる）
	
	std::vector<CustomSprite *> _maskIndexList;	//マスク対象となるパーツ

	UserDataCallback	_userDataCallback;
	UserData			_userData;
	PlayEndCallback		_playEndCallback;
	ErrorCallback		_ErrorCallback;
};





};	// namespace ss

#endif

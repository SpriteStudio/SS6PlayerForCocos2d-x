#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
//    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
	label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);

    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    

	/**********************************************************************************

	SSアニメ表示のサンプルコード
	Visual Studio Communityで動作を確認しています。
	ssbpとpngがあれば再生する事ができますが、Resourcesフォルダにsspjも含まれています。

	**********************************************************************************/
	//--------------------------------------------------------------------------------
	//SS5.5から搭載されたエフェクト機能の最適化を行いSS5Managerクラスが追加されました。
	//プレイヤーが共有するエフェクトバッファを作成します。
	//バッファは常駐されますのでゲーム起動時等に1度行ってください。
	auto ss5man = ss::SS5Manager::getInstance();
	ss5man->createEffectBuffer(1024);			//エフェクト用バッファの作成
	//--------------------------------------------------------------------------------

	//リソースマネージャの作成
	auto resman = ss::ResourceManager::getInstance();
	//プレイヤーの作成
	ssplayer = ss::Player::create();

	//アニメデータをリソースに追加
	resman->addData("instance_sample/instance_test.ssbp");
	//プレイヤーにリソースを割り当て
	ssplayer->setData("instance_test");						// ssbpファイル名（拡張子不要）
	//アニメーションの再生
	ssplayer->play("character_template_2head/walk");				// アニメーションの指定(ssae名/アニメーション名)

	ssplayer->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	//スケールの設定
	ssplayer->setScale(0.5f, 0.5f);


	//ユーザーデータコールバックを設定
	ssplayer->setUserDataCallback(CC_CALLBACK_2(HelloWorld::userDataCallback, this));

	//アニメーション終了コールバックを設定
	ssplayer->setPlayEndCallback(CC_CALLBACK_1(HelloWorld::playEndCallback, this));

	//プレイヤーをゲームシーンに追加
	this->addChild(ssplayer, 10);


	//updeteの作成
	this->scheduleUpdate();


    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}



//メインループ
int cnt = 0;
int type = 0;
void HelloWorld::update(float delta)
{
	int max_frame = ssplayer->getMaxFrame();
	int now_frame = ssplayer->getFrameNo();
	{
		cnt++;
		if ( (cnt % 180) == 0 )
		{
			//インスタンスパーツの参照アニメーションを変更します。
			//再生条件を設定する場合はinstanceのメンバを編集してください。
			auto resman = ss::ResourceManager::getInstance();
			ss::Instance instance;
			instance.clear();

			switch (type)
			{
			case 0:
				instance.refEndframe = resman->getMaxFrame("instance_test", "face/face2") - 1;	//アニメーションの長さを取得
				ssplayer->changeInstanceAnime("face_base", "face/face2", true, instance);				 // アニメーションの指定(ssae名/アニメーション名)
				type++;
				break;
			case 1:
				instance.refEndframe = resman->getMaxFrame("instance_test", "face/face3") - 1;	//アニメーションの長さを取得
				ssplayer->changeInstanceAnime("face_base", "face/face3", true, instance);				 // アニメーションの指定(ssae名/アニメーション名)
				type++;
				break;
			case 2:
				instance.refEndframe = resman->getMaxFrame("instance_test", "face/face1") - 1;	//アニメーションの長さを取得
				ssplayer->changeInstanceAnime("face_base", "face/face1", true, instance);				 // アニメーションの指定(ssae名/アニメーション名)
				type = 0;
				break;
			}
		}
	}
	{
		auto str = String::createWithFormat("max:%d, fream:%d", max_frame, now_frame);
		label->setString(str->getCString());
	}
}

//ユーザーデータコールバック
void HelloWorld::userDataCallback(ss::Player* player, const ss::UserData* data)
{
	//再生したフレームにユーザーデータが設定されている場合呼び出されます。
	//プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
/*
	//コールバック内でパーツのステータスを取得したい場合は、この時点ではアニメが更新されていないため、
	//getPartState　に　data->frameNo　でフレーム数を指定して取得してください。
	ss::ResluteState result;
	//再生しているモーションに含まれるパーツ名「collision」のステータスを取得します。
	ssplayer->getPartState(result, "collision", data->frameNo);
*/	

}

//アニメーション終了コールバック
void HelloWorld::playEndCallback(ss::Player* player)
{
	//再生したアニメーションが終了した段階で呼び出されます。
	//プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
	//player->getPlayAnimeName();
	//を使用する事で再生しているアニメーション名を取得する事もできます。
	
	//ループ回数分再生した後に呼び出される点に注意してください。
	//無限ループで再生している場合はコールバックが発生しません。

}


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
    label = Label::create("Hello World", "Arial", 24);
    
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
	Visual Studio Express 2013 for Windows Desktopで動作を確認しています。
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
	//それぞれのプラットフォームに合わせたパスへ変更してください。
	resman->addData("character_template_comipo\\character_template1.ssbp");
	//プレイヤーにリソースを割り当て
	ssplayer->setData("character_template1");        // ssbpファイル名（拡張子不要）
	//再生するモーションを設定
	ssplayer->play("character_template_3head/stance");				 // アニメーション名を指定(ssae名/アニメーション名)

	//アニメの位置を設定
	ssplayer->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	//スケールの変更
	ssplayer->setScale(0.5f, 0.5f);

	//ユーザーデータコールバックを設定
	ssplayer->setUserDataCallback(CC_CALLBACK_2(HelloWorld::userDataCallback, this));

	//アニメーション終了コールバックを設定
	ssplayer->setPlayEndCallback(CC_CALLBACK_1(HelloWorld::playEndCallback, this));

	//プレイヤーをゲームシーンに追加
	this->addChild(ssplayer, 10);


	//updeteの作成
	this->scheduleUpdate();


	/**********************************************************************************

	アニメーションに含まるパーツがタッチされたかを取得するサンプルコードです。
	サンプルでは、パーツ名"collision"の範囲をタッチ判定します。

	**********************************************************************************/

	//イベントリスナーを作成
	auto listener = EventListenerTouchOneByOne::create();

	//タッチ開始
	listener->onTouchBegan = [](Touch* touch, Event* event)
	{
		//イベントを発生させたSSPlayerを取得します。
		auto target = (ss::Player*)event->getCurrentTarget();

		//パーツ名を検索してタッチ判定用の矩形を取得
		ss::ResluteState result;
		if ( true == target->getPartState(result, "collision") )
		{
			//Nodeの配置座標とタッチで取得される座標のY方向が逆の様なので上下を反転します。
			Size visibleSize = Director::getInstance()->getVisibleSize();
			Rect targetBox(result.x, visibleSize.height - result.y, result.scaledsize_X, result.scaledsize_Y);

			//タッチされた座標の取得
			Point touchPoint = Vec2(touch->getLocationInView().x, touch->getLocationInView().y);

			//touchPointがtargetBoxの中に含まれているか判定
			if (targetBox.containsPoint(touchPoint))
			{
				//パーツがタッチされました。
				CCLOG("touch parts");
				return true;
			}
		}


		return false;
	};

	//イベントリスナーを登録
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, ssplayer);


    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}


//メインループ
void HelloWorld::update(float delta)
{
	// ここに処理を記述
	
	//パーツ名から座標を取得します。
	//ゲームの内容に応じて必要な情報を取得できるようにプレイヤーを改造してください。
	ss::ResluteState result;
	//再生しているモーションに含まれるパーツ名「collision」のステータスを取得します。
	ssplayer->getPartState(result, "collision");
		
	//取得した座標を表示
	auto str = String::createWithFormat("x:%f, y:%f", result.x, result.y);
	label->setString(str->getCString());

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


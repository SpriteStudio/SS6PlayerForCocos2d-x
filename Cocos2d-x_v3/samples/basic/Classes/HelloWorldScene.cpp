#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
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
    
    label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 10);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    

	/**********************************************************************************

	SpriteStudioアニメーション表示のサンプルコード
	Visual Studio Community 2017で動作を確認しています。

	ssbpとpngがあれば再生する事ができますが、Resourcesフォルダにsspjも含まれています。

	**********************************************************************************/

	//プレイヤーを使用する前の初期化処理
	//この処理はアプリケーションの初期化で１度だけ行ってください。
	ss::SSPlatformInit();
	//プレイヤーを使用する前の初期化処理ここまで


	//リソースマネージャの作成
	resman = ss::ResourceManager::getInstance();
	
	//SS6Player for Cocos2d-xではSSPlayerControlを作成し、getSSPInstance()を経由してプレイヤーを操作します
	//プレイヤーの作成
	ssplayer = ss::SSPlayerControl::create();
	//アニメデータをリソースに追加
	//それぞれのプラットフォームに合わせたパスへ変更してください。
	resman->addData("character_template_comipo/character_template1.ssbp");
	//プレイヤーにリソースを割り当て
	ssplayer->getSSPInstance()->setData("character_template1");        // ssbpファイル名（拡張子不要）
	 //再生するモーションを設定
	ssplayer->getSSPInstance()->play("character_template_3head/stance");				 // アニメーション名を指定(ssae名/アニメーション名)


	//表示位置を設定
	Size size = cocos2d::Director::getInstance()->getWinSize();
	ssplayer->getSSPInstance()->setPosition(size.width / 2, size.height / 2);

	//ユーザーデータコールバックを設定
	ssplayer->getSSPInstance()->setUserDataCallback(CC_CALLBACK_2(HelloWorld::userDataCallback, this));

	//アニメーション終了コールバックを設定
	ssplayer->getSSPInstance()->setPlayEndCallback(CC_CALLBACK_1(HelloWorld::playEndCallback, this));

	//プレイヤーをゲームシーンに追加
	this->addChild(ssplayer, 1);

	//ssbpに含まれているアニメーション名のリストを取得する
	animename = resman->getAnimeName(ssplayer->getSSPInstance()->getPlayDataName());
	playindex = 0;				//現在再生しているアニメのインデックス
	playerstate = 0;

	//キーボード入力コールバックの作成
	auto listener = cocos2d::EventListenerKeyboard::create();
	listener->onKeyPressed = CC_CALLBACK_2(HelloWorld::onKeyPressed, this);
	listener->onKeyReleased = CC_CALLBACK_2(HelloWorld::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);


	//updeteの作成
	this->scheduleUpdate();

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

	ss:: SSPlatformRelese();


    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}

void HelloWorld::update(float delta)
{
	int max_frame = 0;
	int now_frame = 0;
	if (ssplayer)
	{
		max_frame = ssplayer->getSSPInstance()->getEndFrame();
		now_frame = ssplayer->getSSPInstance()->getFrameNo();
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

// キーボード入力コールバック
bool press = false;
void HelloWorld::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	log("Key with keycode %d pressed", keyCode);
	if(press == false)
	{ 
		if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW)
		{
			playindex--;
			if (playindex < 0)
			{
				playindex = animename.size() - 1;
			}
			std::string name = animename.at(playindex);
			ssplayer->getSSPInstance()->play(name);
		}
		else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
		{
			playindex++;
			if (playindex >= animename.size())
			{
				playindex = 0;
			}
			std::string name = animename.at(playindex);
			ssplayer->getSSPInstance()->play(name);
		}
		if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_Z)
		{
			if (playerstate == 0)
			{
				ssplayer->getSSPInstance()->animePause();
				playerstate = 1;
			}
			else
			{
				ssplayer->getSSPInstance()->animeResume();
				playerstate = 0;
			}
		}
	}
	press = true;
}

void HelloWorld::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	log("Key with keycode %d released", keyCode);
	press = false;
}

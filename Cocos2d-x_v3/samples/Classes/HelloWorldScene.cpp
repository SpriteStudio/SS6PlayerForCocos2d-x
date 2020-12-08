/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
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

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }
    
    // プレイヤーを使用する前の初期化処理
    // この処理はアプリケーションの初期化で１度だけ行ってください。
    // Initialize for using SS6Player
    // You must use at once at initialize of an application.
    ss::SSPlatformInit();
    
    // リソースマネージャの作成
    // create resource manager
    resman = ss::ResourceManager::getInstance();
    
    // SS6Player for Cocos2d-xではSSPlayerControlを作成し、getSSPInstance()を経由してプレイヤーを操作します
    // create SS6Player for Cocos2d-x and control the Player via getSSPInstance()

    // プレイヤーの作成
    // create Player
    ssplayer = ss::SSPlayerControl::create();
    // アニメデータをリソースに追加
    // それぞれのプラットフォームに合わせたパスへ変更してください。
    // add an animation data to resource manager
    // change path fitting each platforms
    resman->addData("character_template_comipo/character_template1.ssbp");
    // プレイヤーにリソースを割り当て
    // assign resouce to Player
    ssplayer->getSSPInstance()->setData("character_template1");
    // 再生するモーションを設定
    // set a motion for Player
    ssplayer->getSSPInstance()->play("character_template_3head/stance");

    // 表示位置を設定
    // set position
    Size size = cocos2d::Director::getInstance()->getWinSize();
    ssplayer->setPosition(size.width / 2, size.height / 4);
    ssplayer->setScale(0.5f, 0.5f);
    ssplayer->setRotation(0);
    ssplayer->setOpacity(255);
    ssplayer->getSSPInstance()->setColor(255, 255, 255);
    ssplayer->getSSPInstance()->setFlip(false, false);

    // ユーザーデータコールバックを設定
    // set a callback of user data
    ssplayer->getSSPInstance()->setUserDataCallback(CC_CALLBACK_2(HelloWorld::userDataCallback, this));

    // アニメーション終了コールバックを設定
    // set a callback of the end of an animation.
    ssplayer->getSSPInstance()->setPlayEndCallback(CC_CALLBACK_1(HelloWorld::playEndCallback, this));

    // プレイヤーをゲームシーンに追加
    // add the player to game scene
    this->addChild(ssplayer, 1);

    // ssbp に含まれているアニメーション名のリストを取得する
    // get a list of animation-name that be ssbp contains
    animename = resman->getAnimeName(ssplayer->getSSPInstance()->getPlayDataName());
    playindex = 0;  // 現在再生しているアニメのインデックス // currently index of playing a animation
    playerstate = 0;

    // キーボード入力コールバックの作成
    // create a callback of inputting keyboard
    auto listener = cocos2d::EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(HelloWorld::onKeyPressed, this);
    listener->onKeyReleased = CC_CALLBACK_2(HelloWorld::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // updeteの作成
    // create update
    this->scheduleUpdate();

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

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

    if (label)
    {
        auto str = String::createWithFormat("max:%d, fream:%d", max_frame, now_frame);
        label->setString(str->getCString());
    }
}

// ユーザーデータコールバック
// a callback of user data
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

// アニメーション終了コールバック
// a callback of the end of an animation.
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
            if (playindex >= (int)animename.size())
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

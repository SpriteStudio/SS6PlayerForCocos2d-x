#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "SSPlayer/SS5Player.h"

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);


	//アップデート
	virtual void update(float delta);

	//ユーザーデータコールバック
	void userDataCallback(ss::Player* player, const ss::UserData* data);

	//アニメーション終了コールバック
	void playEndCallback(ss::Player* player);

private:
	//SS5プレイヤー
	ss::Player *ssplayer;

	//情報表示用ラベル
	cocos2d::Label *label;
};

#endif // __HELLOWORLD_SCENE_H__

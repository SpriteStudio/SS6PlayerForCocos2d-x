#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "./SSPlayer/SS6Player.h"

class HelloWorld : public cocos2d::Scene
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

	//キーボードコールバック
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

private:
	// SSプレイヤー
	ss::SSPlayerControl *ssplayer;
	ss::ResourceManager *resman;

	//情報表示用ラベル
	cocos2d::Label *label;

	std::vector<std::string> animename;	//アニメーション名のリスト
	int playindex;						//現在再生しているアニメのインデックス
	int playerstate;					//再生状態

};

#endif // __HELLOWORLD_SCENE_H__

#ifndef __COGNITOWORLD_SCENE_H__
#define __COGNITOWORLD_SCENE_H__

#include "cocos2d.h"
#include "CognitoMenu.h"

using namespace cocos2d;

#define LEVEL_PREFIX "Level"

static int levelNum = 1;

class CognitoWorld : public cocos2d::Layer
{
private:
    ValueMap lvlConfigData;
    
    Size visibleSize;
    
    Menu* gameMenu;
    
    Sprite *greenDisc;
    Sprite *blueDisc;
    
    Sprite *endLine;
    
    Vector<Sprite*> standingObstacles;
    
    bool isGameOver = false;
    
    bool isGreenTouched = false;
    
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    void loadLevelConfigData();
    
    // a selector callback
    void menuCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(CognitoWorld);
    
    void addEndLine();
    
    void addPlayer();
    
    void addMovingObstacles(float interval);
    
    void addStandingObstacle(int count, bool startMove);
    
    bool onContactBegan(PhysicsContact &contact);
    
    bool onTouchBeganBlue(Touch *touch, Event *unused_event);
    
    bool onTouchBeganGreen(Touch *touch, Event *unused_event);
    
    void onTouchMovedGreen(Touch *touch, Event *unused_event);
    
    void onTouchEndGreen(Touch *touch, Event *unused_event);
    
    void update(float) override;
    
    void doJumpBlue(Node* node);
    
    void doActionOnGreen(Node* node);
    
    void stopGame(bool inFinished);
    
    void restartGame();
};

#endif // __COGNITOWORLD_SCENE_H__

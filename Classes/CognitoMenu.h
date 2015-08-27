#ifndef __Cognito__CognitoMenu__
#define __Cognito__CognitoMenu__

#include "cocos2d.h"
#include "CognitoWorldScene.h"

using namespace cocos2d;

#define MENU_CONFIG "Menu.plist"

class CognitoMenu : public cocos2d::Layer
{
    
private:
    ValueMap configData;
    
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();
    
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(CognitoMenu);
    
    void loadConfigData();
    
    void exitGame();
    
    void loadGame();
};

#endif /* defined(__Cognito__CognitoMenu__) */

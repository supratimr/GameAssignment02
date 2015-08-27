#include "CognitoMenu.h"


Scene* CognitoMenu::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = CognitoMenu::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool CognitoMenu::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    loadConfigData();
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // add version label
    auto gameVersion = Label::createWithSystemFont(configData.at("GameName").asString(), "Marker Felt", 18);
    gameVersion->setTextColor(Color4B::YELLOW);
    gameVersion->enableShadow();
    gameVersion->setPosition(Vec2(origin.x + visibleSize.width/2 - gameVersion->getContentSize().width/2.0f ,
                                  origin.y + visibleSize.height - gameVersion->getContentSize().height * 1.3f));
    gameVersion->setAlignment(TextHAlignment::CENTER);
    gameVersion->setAnchorPoint(Vec2(0.0f, 0.0f));
    this->addChild(gameVersion);

    
    /////////////////////////////
    // 2. add a menu item with image, which is clicked to quit the program
    
    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           configData.at("ExitBtnTexNormal").asString(),
                                           configData.at("ExitBtnTexSel").asString(),
                                           CC_CALLBACK_1(CognitoMenu::menuCloseCallback, this));
    
    closeItem->setPosition(Vec2(visibleSize.width/2 ,visibleSize.height/2));
    
    closeItem->setTag(0);
    
    // add a "load game" icon to enter the game. it's an autorelease object
    auto loadGame = MenuItemImage::create(
                                           configData.at("StartBtnTexNormal").asString(),
                                           configData.at("StartBtnTexSel").asString(),
                                           CC_CALLBACK_1(CognitoMenu::menuCloseCallback, this));
    
    loadGame->setPosition(Vec2(visibleSize.width/2 ,visibleSize.height/2 + 50.0f));
    
    loadGame->setTag(1);
    
    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem,loadGame, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
    
    /////////////////////////////
    // 4
    auto background = DrawNode::create();
    background->drawSolidRect(origin, visibleSize, Color4F(0.0,0.0,0.0,1.0));
    this->addChild(background);
    
    return true;
}


void CognitoMenu::menuCloseCallback(Ref* pSender)
{
    MenuItemImage* pMenuItem = (MenuItemImage *)pSender;
    int tag = (int)pMenuItem->getTag();
    
    switch (tag)
    {
        case 0:
            exitGame();
            break;
            
        case 1:
            loadGame();
            break;
            
        default:
            loadGame();
            break;
    }
}

void CognitoMenu::loadConfigData()
{
    std::string path = FileUtils::getInstance()->fullPathForFilename(MENU_CONFIG);
    configData = FileUtils::getInstance()->getValueMapFromFile(path);
}

void CognitoMenu::exitGame()
{
    Director::getInstance()->end();
    
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        exit(0);
    #endif
}

void CognitoMenu::loadGame()
{
    Scene* gameScene = CognitoWorld::createScene();
    Director::getInstance()->replaceScene(gameScene);
}


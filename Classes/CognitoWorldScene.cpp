#include "CognitoWorldScene.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

USING_NS_CC;

enum PhysicsCategory
{
    None = 0,
    Obstacle = (1 << 0),
    Disc = (1 << 1),
    All = PhysicsCategory::Obstacle | PhysicsCategory::Disc
};

Scene* CognitoWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0,0));
    //scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
    
    // 'layer' is an autorelease object
    auto layer = CognitoWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool CognitoWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    loadLevelConfigData();
    
    visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with image, which is clicked to quit the program

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           lvlConfigData.at("MenuBackNormal").asString(),
                                           lvlConfigData.at("MenuBackSel").asString(),
                                           CC_CALLBACK_1(CognitoWorld::menuCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));
    closeItem->setTag(0);
    
    auto restartItem = MenuItemImage::create(
                                           lvlConfigData.at("GameOverNormal").asString(),
                                           lvlConfigData.at("GameOverSel").asString(),
                                           CC_CALLBACK_1(CognitoWorld::menuCallback, this));
    
    restartItem->setPosition(Vec2(visibleSize.width/2 ,visibleSize.height/2));
    restartItem->setTag(1);
    restartItem->setVisible(false);
    
    // create menu, it's an autorelease object
    gameMenu = Menu::create(closeItem, restartItem, NULL);
    gameMenu->setPosition(Vec2::ZERO);
    this->addChild(gameMenu, 1);

    /////////////////////////////
    // 4
    auto background = DrawNode::create();
    background->drawSolidRect(origin, visibleSize, Color4F(0.6,0.6,0.6,1.0));
    this->addChild(background);
    
    // 5
    //srand((unsigned int)time(nullptr));
    
    addEndLine();
    addPlayer();
    this->schedule(schedule_selector(CognitoWorld::addMovingObstacles), lvlConfigData.at("BlueObstacleSpawnDelay").asFloat());
    addStandingObstacle(lvlConfigData.at("GreenObstacleNum").asInt(), false);
    
    isGameOver = false;
    
    auto contactListner = EventListenerPhysicsContact::create();
    contactListner->onContactBegin = CC_CALLBACK_1(CognitoWorld::onContactBegan, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListner, this);
    
    auto eventListnerBlue = EventListenerTouchOneByOne::create();
    eventListnerBlue->onTouchBegan = CC_CALLBACK_2(CognitoWorld::onTouchBeganBlue, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(eventListnerBlue, blueDisc);
    
    auto eventListnerGreen = EventListenerTouchOneByOne::create();
    eventListnerGreen->onTouchBegan = CC_CALLBACK_2(CognitoWorld::onTouchBeganGreen, this);
    eventListnerGreen->onTouchMoved = CC_CALLBACK_2(CognitoWorld::onTouchMovedGreen, this);
    eventListnerGreen->onTouchEnded = CC_CALLBACK_2(CognitoWorld::onTouchEndGreen, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(eventListnerGreen, greenDisc);
    
    //
    SimpleAudioEngine::getInstance()->playBackgroundMusic(lvlConfigData.at("BkgMusic").asString().c_str(), true);
    
    this->scheduleUpdate();
    
    return true;
}

void CognitoWorld::loadLevelConfigData()
{
    std::string path = FileUtils::getInstance()->fullPathForFilename(LEVEL_PREFIX + (std::to_string(levelNum)) + ".plist");
    
    while (path.empty())
    {
        levelNum -= 1;
        path = FileUtils::getInstance()->fullPathForFilename(LEVEL_PREFIX + (std::to_string(levelNum)) + ".plist");
    }
        
    lvlConfigData = FileUtils::getInstance()->getValueMapFromFile(path);
}

void CognitoWorld::menuCallback(Ref* pSender)
{
    MenuItemImage* pMenuItem = (MenuItemImage *)pSender;
    int tag = (int)pMenuItem->getTag();
    
    switch (tag)
    {
        case 0:
        {
            Scene* menuScene = CognitoMenu::createScene();
            Director::getInstance()->replaceScene(menuScene);
            Director::getInstance()->resume();
            break;
        }
        case 1:
            restartGame();
            break;
            
        default:
            
            break;
    }
}

void CognitoWorld::addEndLine()
{
    endLine = Sprite::create(lvlConfigData.at("EndLine").asString());
    endLine->setPosition(Vec2(visibleSize.width * 0.85f, visibleSize.height - 5.0f));
    this->addChild(endLine);
}

void CognitoWorld::addPlayer()
{
    greenDisc = Sprite::create(lvlConfigData.at("GreenPlayer").asString());
    greenDisc->setPosition(Vec2(visibleSize.width * 0.05f, visibleSize.height * 0.25));
    
    auto physicsBodyGreen = PhysicsBody::createBox(Size(greenDisc->getContentSize().width, greenDisc->getContentSize().height),PhysicsMaterial(1.0f, 1.0f, 0.0f));
    physicsBodyGreen->setDynamic(true);
    physicsBodyGreen->setCategoryBitmask((int)PhysicsCategory::Disc);
    physicsBodyGreen->setCollisionBitmask((int)PhysicsCategory::None);
    physicsBodyGreen->setContactTestBitmask((int)PhysicsCategory::Obstacle);
    
    greenDisc->setPhysicsBody(physicsBodyGreen);
    
    this->addChild(greenDisc);
    
    blueDisc = Sprite::create(lvlConfigData.at("BluePlayer").asString());
    blueDisc->setPosition(Vec2(visibleSize.width * 0.05f, visibleSize.height * 0.75));
   
    auto physicsBodyBlue = PhysicsBody::createBox(Size(blueDisc->getContentSize().width, blueDisc->getContentSize().height),PhysicsMaterial(1.0f, 1.0f, 0.0f));
    physicsBodyBlue->setDynamic(true);
    physicsBodyBlue->setCategoryBitmask((int)PhysicsCategory::Disc);
    physicsBodyBlue->setCollisionBitmask((int)PhysicsCategory::None);
    physicsBodyBlue->setContactTestBitmask((int)PhysicsCategory::Obstacle);
   
    blueDisc->setPhysicsBody(physicsBodyBlue);
    
    this->addChild(blueDisc);
    
    // 3
    float PosX = visibleSize.width - (visibleSize.width - endLine->getPositionX()) + blueDisc->getContentSize().width;
    int randomDuration = PosX / lvlConfigData.at("BluePlayerSpeed").asFloat();
    
    auto actionMove = MoveTo::create(randomDuration, Vec2(PosX, blueDisc->getPositionY()));
    blueDisc->runAction(actionMove);
}

void CognitoWorld::addMovingObstacles(float interval)
{
    auto obstacle = Sprite::create(lvlConfigData.at("BlueObstacle").asString());
    auto obstacleSize = obstacle->getContentSize();
    
    auto physicsBody = PhysicsBody::createBox(Size(obstacleSize.width, obstacleSize.height),PhysicsMaterial(1.0f, 1.0f, 0.0f));
    
    physicsBody->setDynamic(true);
    
    physicsBody->setCategoryBitmask((int)PhysicsCategory::Obstacle);
    physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
    physicsBody->setContactTestBitmask((int)PhysicsCategory::Disc);
    
    obstacle->setPhysicsBody(physicsBody);
    
    // 1
    auto obstacleContentSize = obstacle->getContentSize();
    
    float posY = visibleSize.height * 0.75;
    float posX = visibleSize.width - (visibleSize.width - endLine->getPositionX()) - obstacleContentSize.width;
    
    obstacle->setPosition(Vec2(posX, posY));
    this->addChild(obstacle);
    
    float randomDuration = obstacle->getPositionX() / lvlConfigData.at("BlueObstacleSpeed").asFloat();
    
    // 3
    auto actionMove = MoveTo::create(randomDuration, Vec2(-obstacleContentSize.width/2, posY));
    auto actionRemove = RemoveSelf::create();
    obstacle->runAction(Sequence::create(actionMove,actionRemove, nullptr));
}

void CognitoWorld::addStandingObstacle(int count, bool startMove)
{
    float posY = visibleSize.height * 0.25;
    
    int maxX = visibleSize.width - (visibleSize.width - endLine->getPositionX());
    int minX = maxX * 0.15f;
    int rangeX = maxX - minX;
    
    for (int idx=0; idx < count; idx++)
    {
        Sprite* obstacle = Sprite::create(lvlConfigData.at("GreenObstacle").asString());
        
        auto obstacleSize = obstacle->getContentSize();
        
        auto physicsBody = PhysicsBody::createBox(Size(obstacleSize.width, obstacleSize.height),PhysicsMaterial(1.0f, 1.0f, 0.0f));
        
        physicsBody->setDynamic(true);
        
        physicsBody->setCategoryBitmask((int)PhysicsCategory::Obstacle);
        physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
        physicsBody->setContactTestBitmask((int)PhysicsCategory::Disc);
        
        obstacle->setPhysicsBody(physicsBody);
        
        // 1
        auto obstacleContentSize = obstacle->getContentSize();
        
        
        int posX = ((rangeX / count) * (idx+1) ) + minX;
        
        obstacle->setPosition(Vec2(posX, posY));
        this->addChild(obstacle);
        
        standingObstacles.insert(idx, obstacle);
        
        if(startMove)
        {
            float randomDuration = obstacle->getPositionX() / lvlConfigData.at("GreenObstacleSpeed").asFloat();
            
            auto actionMove = MoveTo::create(randomDuration, Vec2(0.0f, obstacle->getPositionY()));
            obstacle->runAction(actionMove);
        }
    }
}

bool CognitoWorld::onContactBegan(PhysicsContact &contact)
{
    if(lvlConfigData.at("SetImmortal").asBool())
        return false;
    
    auto node = contact.getShapeA();
    int nodeBitMask = node->getCategoryBitmask();
    
    if(nodeBitMask == PhysicsCategory::Obstacle)
    {
        stopGame(false);
        return true;
    }
    
    node = contact.getShapeB();
    nodeBitMask = node->getCategoryBitmask();
    
    if(nodeBitMask == PhysicsCategory::Obstacle)
    {
        stopGame(false);
        return true;
    }
    
    return false;
}

bool CognitoWorld::onTouchBeganBlue(Touch *touch, Event *unused_event)
{
    if(blueDisc->getPositionY() != visibleSize.height * 0.75)
        return false;
    
    // 1
    auto node = unused_event->getCurrentTarget();
    // 2
    Vec2 touchLocation = touch->getLocation();
    
    Rect bound = node->getBoundingBox();
    
    if(bound.containsPoint(touch->getLocation()))
        doJumpBlue(node);
    
    
    SimpleAudioEngine::getInstance()->playEffect(lvlConfigData.at("BlueJumpSFX").asString().c_str());
    
    return true;
}

bool CognitoWorld::onTouchBeganGreen(Touch *touch, Event *unused_event)
{
    // 1
    auto node = unused_event->getCurrentTarget();
    // 2
    Vec2 touchLocation = touch->getLocation();
    
    Rect bound = node->getBoundingBox();
    
    if(bound.containsPoint(touch->getLocation()))
        doActionOnGreen(node);
    
    isGreenTouched = true;
    
    return true;
}

void CognitoWorld::onTouchMovedGreen(Touch *touch, Event *unused_event)
{
    auto node = unused_event->getCurrentTarget();
    
    Vec2 touchLocation = touch->getLocation();
    
    Rect bound = node->getBoundingBox();
    
    if(bound.containsPoint(touch->getLocation()))
        node->setPosition(node->getPositionX()+ touch->getDelta().x, node->getPositionY());
}

void CognitoWorld::onTouchEndGreen(Touch *touch, Event *unused_event)
{
    auto node = unused_event->getCurrentTarget();
    
    Vec2 touchLocation = touch->getLocation();
    
    Rect bound = node->getBoundingBox();
    
    if(bound.containsPoint(touch->getLocation()))
    {
        for (int idx=0; idx < lvlConfigData.at("GreenObstacleNum").asInt(); idx++)
        {
            Sprite* obstacle = standingObstacles.at(idx);
            obstacle->stopAllActions();
        }
        
        if(node->getPositionY() == visibleSize.height * 0.25f)
        {
            auto actionJump = JumpBy::create(1.0f, Vec2(lvlConfigData.at("GreenJumpXRange").asFloat(), 0), lvlConfigData.at("GreenJumpHeight").asFloat(), 1);
            node->runAction(actionJump);
            
            SimpleAudioEngine::getInstance()->playEffect(lvlConfigData.at("GreenJumpSFX").asString().c_str());
        }
    }
    
    isGreenTouched = false;
}

void CognitoWorld::update(float dt)
{
    if(!isGameOver && blueDisc->getPositionX() > endLine->getPositionX() && greenDisc->getPositionX() > endLine->getPositionX())
    {
        isGameOver = true;
        stopGame(true);
    }
    
    if(!isGameOver)
    {
        for (int idx=0; idx < lvlConfigData.at("GreenObstacleNum").asInt(); idx++)
        {
            Sprite* obstacle = standingObstacles.at(idx);
            if(obstacle->getPositionX() == 0.0f)
            {
                standingObstacles.eraseObject(obstacle);
                this->removeChild(obstacle);
                addStandingObstacle(1, isGreenTouched);
            }
        }
    }
}

void CognitoWorld::doJumpBlue(Node* node)
{
    node->stopAllActions();
    
    auto actionJump = JumpBy::create(1.0f, Vec2(lvlConfigData.at("BlueJumpXRange").asFloat(), 0), lvlConfigData.at("BlueJumpHeight").asFloat(), 1);
    
    // 3
    float PosX = visibleSize.width - (visibleSize.width - endLine->getPositionX()) + blueDisc->getContentSize().width;
    float randomDuration = PosX / lvlConfigData.at("BluePlayerSpeed").asFloat();
    
    auto actionMove = MoveTo::create(randomDuration, Vec2(PosX, blueDisc->getPositionY()));
    
    node->runAction(Sequence::create(actionJump, actionMove, nullptr));
}

void CognitoWorld::doActionOnGreen(Node* node)
{
    for (int idx=0; idx < lvlConfigData.at("GreenObstacleNum").asInt(); idx++)
    {
        Sprite* obstacle = standingObstacles.at(idx);
        
        // 1
        float randomDuration = obstacle->getPositionX() / lvlConfigData.at("GreenObstacleSpeed").asFloat();
        
        auto actionMove = MoveTo::create(randomDuration, Vec2(0.0f, obstacle->getPositionY()));
        obstacle->runAction(actionMove);
    }
}

void CognitoWorld::stopGame(bool isFinished)
{
    if(!isFinished)
        gameMenu->getChildByTag(1)->setVisible(true);
    else
    {
        levelNum += 1;
        gameMenu->getChildByTag(0)->setPosition(Vec2(visibleSize.width/2 ,visibleSize.height/2));
    }
    
    SimpleAudioEngine::getInstance()->stopAllEffects();
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    Director::getInstance()->pause();
}

void CognitoWorld::restartGame()
{
    Scene* gameScene = CognitoWorld::createScene();
    Director::getInstance()->replaceScene(gameScene);
    Director::getInstance()->resume();
}


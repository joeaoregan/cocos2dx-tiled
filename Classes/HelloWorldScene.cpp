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
#include "SimpleAudioEngine.h"

USING_NS_CC;

cocos2d::Scene* HelloWorld::createScene()
{
	cocos2d::Scene* scene = cocos2d::Scene::create();
	HelloWorld* layer = HelloWorld::create();
	scene->addChild(layer);

	HudLayer* hud = new HudLayer();
	hud->init();
	scene->addChild(hud);
	layer->_hud = hud;

    return scene;
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
    if ( !Layer::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("audio/pickup.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("audio/hit.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("audio/move.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("audio/TileMap.wav");

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
    } else {
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

    auto label = Label::createWithTTF("Tiled Game Example", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr) {
        problemLoading("'fonts/Marker Felt.ttf'");
    }  else {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr) {
        problemLoading("'HelloWorld.png'");
    } else {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }

	_tileMap = new cocos2d::TMXTiledMap();
	_tileMap->initWithTMXFile("tiles/TileMap.tmx");
	_background = _tileMap->getLayer("Background"); // layerNamed() deprecated
	this->addChild(_tileMap);

	_foreground = _tileMap->getLayer("Foreground");

	_meta = _tileMap->getLayer("Meta");
	_meta->setVisible(false);

	cocos2d::TMXObjectGroup* objectGroup = _tileMap->getObjectGroup("Objects"); // objectGroupNamed() deprecated

	if (objectGroup == NULL) {
		// CCLog("tile map has no objects in object layer"); // not working
		return false;
	}

	ValueMap spawnPoint = objectGroup->getObject("SpawnPoint");
	int x = spawnPoint.at("x").asInt();
	int y = spawnPoint.at("y").asInt();

	_player = new cocos2d::Sprite();
	_player->initWithFile("sprites/Player.png");
	_player->setPosition(cocos2d::Vec2(x, y));

	this->addChild(_player);
	this->setViewPointCenter(_player->getPosition());
	
	auto eventListener = EventListenerTouchOneByOne::create();
	eventListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	eventListener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(eventListener, _player);

    return true;
}

void HelloWorld::setViewPointCenter(cocos2d::Vec2 position) {
	cocos2d::Size winSize = cocos2d::Director::getInstance()->getWinSize();

	int x = MAX(position.x, winSize.width / 2);
	int y = MAX(position.y, winSize.height / 2);
	x = MIN(x, (_tileMap->getMapSize().width * this->_tileMap->getTileSize().width) - winSize.width / 2);
	y = MIN(y, (_tileMap->getMapSize().height * _tileMap->getTileSize().height) - winSize.height / 2);

	cocos2d::Vec2 actualPosition = cocos2d::Vec2(x, y);
	cocos2d::Vec2 centerOfView = cocos2d::Vec2(winSize.width / 2, winSize.height / 2);
	//cocos2d::Vec2 viewPoint = centerOfView - actualPosition;
	cocos2d::Vec2 viewPoint = centerOfView;
	viewPoint.subtract(actualPosition);

	this->setPosition(viewPoint);
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

bool HelloWorld::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
	return true;
}

void HelloWorld::setPlayerPosition(cocos2d::Vec2 position) {
	cocos2d::Vec2 tileCoord = this->tileCoordForPosition(position);
	int tileGid = _meta->getTileGIDAt(tileCoord);
	if (tileGid) {
		cocos2d::ValueMap properties = _tileMap->getPropertiesForGID(tileGid).asValueMap();
		
		if (properties.size() > 0) {
			cocos2d::String collisionType = properties.at("collisionType").asString();
			if (collisionType.boolValue()){ 
				if (collisionType.compare("Collide") == 0) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("audio/hit.wav");
					return; // Don't do anything else
				}
				if (collisionType.compare("Collect") == 0) {
					_meta->removeTileAt(tileCoord); // Remove collision tile on meta layer
					_foreground->removeTileAt(tileCoord); // Remove collectable tile on foreground layer
					_numCollected++;
					_hud->numCollectedChanged(_numCollected);
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("audio/pickup.wav");
				}
			}
		}		
	}

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("audio/move.wav");
	_player->setPosition(position);
}

void HelloWorld::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) {
	cocos2d::Vec2 touchLocation = touch->getLocationInView();
	touchLocation = cocos2d::Director::getInstance()->convertToGL(touchLocation);
	touchLocation = this->convertToNodeSpace(touchLocation);

	cocos2d::Vec2 playerPos = _player->getPosition();
	//cocos2d::Vec2 diff = touchLocation - playerPos;
	cocos2d::Vec2 diff = touchLocation;
	diff.subtract(playerPos);

	if (abs(diff.x) > abs(diff.y)) {
		if (diff.x > 0) {
			playerPos.x += _tileMap->getTileSize().width;
		}
		else {
			playerPos.x -= _tileMap->getTileSize().width;
		}
	}
	else {
		if (diff.y > 0) {
			playerPos.y += _tileMap->getTileSize().height;
		}
		else {
			playerPos.y -= _tileMap->getTileSize().height;
		}
	}

	// safety check on the bounds of the map
	if (playerPos.x <= (_tileMap->getMapSize().width * _tileMap->getTileSize().width) &&
		playerPos.y <= (_tileMap->getMapSize().height * _tileMap->getTileSize().height) &&
		playerPos.y >= 0 &&
		playerPos.x >= 0)
	{
		this->setPlayerPosition(playerPos);
	}

	this->setViewPointCenter(_player->getPosition());
}

cocos2d::Vec2 HelloWorld::tileCoordForPosition(cocos2d::Vec2 position) {
	int x = position.x / _tileMap->getTileSize().width; // tile x coord
	int y = ((_tileMap->getMapSize().height * _tileMap->getTileSize().height) - position.y) / _tileMap->getTileSize().height; // tile y coord
	return cocos2d::Vec2(x, y); // return tile coords
}

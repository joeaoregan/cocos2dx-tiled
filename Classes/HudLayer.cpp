#include "HudLayer.h"

//using namespace cocos2d;

bool HudLayer::init()
{
	if (cocos2d::Layer::init()) {
		cocos2d::Size winSize = cocos2d::Director::getInstance()->getWinSize();

		_label = new cocos2d::LabelTTF();
		_label->initWithString("0", "Marker Felt", 24.0);
		_label->setColor(ccc3(255, 255, 255));

		int margin = 10;
		_label->setPosition(ccp(winSize.width - (_label->getContentSize().width / 2) - margin, _label->getContentSize().height / 2 + margin));
		this->addChild(_label);
	}

	return true;
}

void HudLayer::numCollectedChanged(int numCollected)
{
	cocos2d::String *labelCollected = new cocos2d::String();
	labelCollected->initWithFormat("%d", numCollected);
	_label->setString(labelCollected->getCString());
}
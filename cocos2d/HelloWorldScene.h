//
//  HelloWorldScene.h
//  testBox2d2
//
//  Created by 王 天耀 on 13-11-7.
//  Copyright __MyCompanyName__ 2013年. All rights reserved.
//
#ifndef __HELLO_WORLD_H__
#define __HELLO_WORLD_H__

// When you import this file, you import all the cocos2d classes
#include "cocos2d.h"
#include "Box2D.h"

class HelloWorld : public cocos2d::CCLayer {
public:
    ~HelloWorld();
    HelloWorld();
    
    // returns a Scene that contains the HelloWorld as the only child
    static cocos2d::CCScene* scene();

    // touch event
    virtual void ccTouchesBegan(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);
    virtual void ccTouchesMoved(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);
    virtual void ccTouchesEnded(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);
    virtual void ccTouchesCancelled(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);
    
    // add method begin
    void initPhysics();
    void addBall();
    void update(float dt);
    
private:
    b2World*        m_world;
    b2Body*         m_edgeBody;
    b2Fixture*      m_ballFixture;
    b2Body*         m_paddleBody;
    b2Fixture*      m_paddleFixture;
    b2MouseJoint*   m_mouseJoint;
};

#endif // __HELLO_WORLD_H__

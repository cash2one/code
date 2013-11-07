//
//  HelloWorldScene.cpp
//  testBox2d2
//
//  Created by 王 天耀 on 13-11-7.
//  Copyright __MyCompanyName__ 2013年. All rights reserved.
//
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;

#define PTM_RATIO 32        // pixel to meter: 像素转米
#define BALL_MAX_SPEED 50
#define PAI 3.1415926

HelloWorld::HelloWorld()
{
    // init enabled
    setTouchEnabled( true );
    setAccelerometerEnabled( true );
    // init physics and edge
    this->initPhysics();
    this->addBall();
    // tick
    scheduleUpdate();
}

HelloWorld::~HelloWorld()
{
    delete m_world;
    m_edgeBody = NULL;
}

void HelloWorld::addBall()
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    
    // Create sprite and add it to the layer
    CCSprite *ball = CCSprite::create("Ball.png",CCRectMake(0,0,52,52));
    ball->setPosition(ccp(100,100));
    ball->setTag(1);
    addChild(ball);
    // Create ball body
    b2BodyDef ballBodyDef;
    ballBodyDef.type = b2_dynamicBody;
    ballBodyDef.position.Set(100/PTM_RATIO, 100/PTM_RATIO);
    ballBodyDef.userData = ball;
    b2Body * ballBody = m_world->CreateBody(&ballBodyDef);
    // Create circle shape
    b2CircleShape circleShape;
    circleShape.m_radius = 26.0/PTM_RATIO;
    
    // Create shape definition and add to body
    b2FixtureDef ballShapeDef;
    ballShapeDef.shape = &circleShape;
    ballShapeDef.density = 1.0f;        // 密度
    ballShapeDef.friction = 0.0f;       // 摩擦力
    ballShapeDef.restitution = 1.0f;    // 弹力
    m_ballFixture = ballBody->CreateFixture(&ballShapeDef);
    
    // Give shape initial impulse...
    b2Vec2 force = b2Vec2(50, 50);      // 创建作用力向量
    ballBody->ApplyLinearImpulse(force, ballBodyDef.position); // applyLinearImpulse(作用力方向向量,作用点)
    
    // Create paddle and add it to the layer
    CCSprite *paddle = CCSprite::create("Paddle.png",CCRectMake(0,0,120,45));
    //paddle->setPosition(ccp(winSize.width/2, 50));
    paddle->setPosition(ccp(winSize.width - 50, winSize.height/2));
    paddle->setRotation(90);// 旋转 0 - 360
//    paddle->setScale(1);    // 缩放 0 - 1
//    paddle->setOpacity(255);  // 透明度 0 - 255
//    paddle->setFlipX(false);// X轴翻转
//    paddle->setFlipY(false);// Y轴翻转
    //paddle->setColor(ccc3(255, 255, 0));// 颜色设置
    addChild(paddle);
    
    // Create paddle body
    b2BodyDef paddleBodyDef;
    paddleBodyDef.type = b2_dynamicBody;
    paddleBodyDef.position.Set((winSize.width - 50)/PTM_RATIO, winSize.height/2/PTM_RATIO);
    paddleBodyDef.angle = PAI / 2;        // 弧度设置
    paddleBodyDef.userData = paddle;
    m_paddleBody = m_world->CreateBody(&paddleBodyDef);
    
    // Create paddle shape
    b2PolygonShape paddleShape;
    paddleShape.SetAsBox(paddle->getContentSize().width/PTM_RATIO/2,
                         paddle->getContentSize().height/PTM_RATIO/2);
    
    // Create shape definition and add to body
    b2FixtureDef paddleShapeDef;
    paddleShapeDef.shape = &paddleShape;
    paddleShapeDef.density = 10.0f;
    paddleShapeDef.friction = 0.4f;
    paddleShapeDef.restitution = 0.1f;
    m_paddleFixture = m_paddleBody->CreateFixture(&paddleShapeDef);
    
    // Restrict paddle along the x axis
    b2PrismaticJointDef jointDef;
    b2Vec2 worldAxis(0.0f, 1.0f);
    jointDef.collideConnected = true;

    jointDef.Initialize(m_paddleBody, m_edgeBody, m_paddleBody->GetWorldCenter(), worldAxis);
    m_world->CreateJoint(&jointDef);
}

void HelloWorld::update(float dt)
{
    m_world->Step(dt, 10, 10);
    for(b2Body *b = m_world->GetBodyList(); b; b=b->GetNext())
    {
        if(b->GetUserData() != NULL)
        {
            CCSprite *sprite = (CCSprite *)b->GetUserData();
            if(sprite->getTag() == 1)
            {
                b2Vec2 velocity = b->GetLinearVelocity();
                float32 speed = velocity.Length();
                
                // When the ball is greater than max speed, slow it down by
                // applying linear damping.  This is better for the simulation
                // than raw adjustment of the velocity.
                if(speed > BALL_MAX_SPEED)
                {
                    b->SetLinearDamping(0.5);   // 设置线性阻尼系数
                }
                else if(speed < BALL_MAX_SPEED)
                {
                    b->SetLinearDamping(0.0);
                }
            }
            sprite->setPosition(ccp(b->GetPosition().x * PTM_RATIO,b->GetPosition().y * PTM_RATIO));
            sprite->setRotation(-1 * CC_RADIANS_TO_DEGREES(b->GetAngle()));     // 弧度转度
        }
    }
}

void HelloWorld::initPhysics()
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();    // 获取设备分辨率
    CCLOG("winSize.width = %f, height = %f",winSize.width,winSize.height);
    b2Vec2 gravity;
    gravity.Set(0.0f, 0.0f);              
    m_world = new b2World(gravity);
    m_world->SetAllowSleeping(true);        // 不受外力作用下,是否进入睡眠状态
    m_world->SetContinuousPhysics(true);    // 检测连续的碰撞
    
    // Create edges around the entire screen
    // BodyDef-> World -> Body <- Fixture <- Shape
    b2BodyDef edgeBodyDef;               // 定义刚体(在任何力作用下,形状不发生改变)
    edgeBodyDef.position.Set(0,0);
    edgeBodyDef.type = b2_staticBody;
    m_edgeBody = m_world->CreateBody(&edgeBodyDef);   // 创建刚体
    
    b2PolygonShape edgeShape;       // 多边形形状
    b2FixtureDef edgeFixture;
    edgeFixture.shape =&edgeShape;
    edgeFixture.friction = 0.0f;       // 摩擦力
    edgeFixture.restitution = 0.0f;    // 弹力
    
    // SetAsEdge:两点设置障碍线.//设置四根障碍线在屏幕边界,围起一个密闭的空间,以免球弹出屏幕外
    edgeShape.SetAsEdge(b2Vec2(0,0), b2Vec2(winSize.width/PTM_RATIO, 0));
    m_edgeBody->CreateFixture(&edgeFixture);
    
    edgeShape.SetAsEdge(b2Vec2(0,0), b2Vec2(0, winSize.height/PTM_RATIO));
    m_edgeBody->CreateFixture(&edgeFixture);
    
    edgeShape.SetAsEdge(b2Vec2(0, winSize.height/PTM_RATIO), b2Vec2(winSize.width/PTM_RATIO,winSize.height/PTM_RATIO));
    m_edgeBody->CreateFixture(&edgeFixture);
    
    edgeShape.SetAsEdge(b2Vec2(winSize.width/PTM_RATIO, winSize.height/PTM_RATIO),b2Vec2(winSize.width/PTM_RATIO, 0));
    m_edgeBody->CreateFixture(&edgeFixture);
}
// touch event
void HelloWorld::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)
{
    if (m_mouseJoint != NULL) return;
    
    CCTouch *pTouch = (CCTouch*)(pTouches->anyObject());
    CCPoint location = pTouch->getLocationInView();
    location = CCDirector::sharedDirector()->convertToGL(location);
    
    b2Vec2 locationWorld = b2Vec2(location.x/PTM_RATIO, location.y/PTM_RATIO);
    
    if (m_paddleFixture->TestPoint(locationWorld)) {
        b2MouseJointDef md;
        md.bodyA = m_edgeBody;
        md.bodyB = m_paddleBody;
        md.target = locationWorld;
        md.collideConnected = true;
        md.maxForce = 1000.0f * m_paddleBody->GetMass();
        
        m_mouseJoint = (b2MouseJoint *)m_world->CreateJoint(&md);
        m_paddleBody->SetAwake(true);
    }
}

void HelloWorld::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)
{
    if (m_mouseJoint == NULL) return;
    
    CCTouch *pTouch = (CCTouch*)(pTouches->anyObject());
    CCPoint location = pTouch->getLocationInView();
    location = CCDirector::sharedDirector()->convertToGL(location);
    b2Vec2 locationWorld = b2Vec2(location.x/PTM_RATIO, location.y/PTM_RATIO);
    
    m_mouseJoint->SetTarget(locationWorld);
}

void HelloWorld::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent)
{
    if (m_mouseJoint) {
        m_world->DestroyJoint(m_mouseJoint);
        m_mouseJoint = NULL;
    }
}

void HelloWorld::ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent)
{
    if (m_mouseJoint) {
        m_world->DestroyJoint(m_mouseJoint);
        m_mouseJoint = NULL;
    }
}

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // add layer as a child to scene
    CCLayer* layer = new HelloWorld();
    scene->addChild(layer);
    layer->release();
    
    return scene;
}

#include "HelloWorldScene.h"
#include "VisibleRect.h"
#include "2d/MGRDrawNode.h"
#include "2d/MGRSprite.h"
#include "3d/MGRSprite3D.h"
#include "ui/UISlider.h"

USING_NS_CC;
using namespace cocos2d::ui;


Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Layer::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //   /////////////////////////////
    //   // 2. add a menu item with "X" image, which is clicked to quit the program
    //   //	you may modify it.

    //   // add a "close" icon to exit the progress. it's an autorelease object
    //   auto closeItem = MenuItemImage::create(
    //										  "CloseNormal.png",
    //										  "CloseSelected.png",
    //										  CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    //   
    //closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
    //							   origin.y + closeItem->getContentSize().height/2));

    //   // create menu, it's an autorelease object
    //   auto menu = Menu::create(closeItem, NULL);
    //   menu->setPosition(Vec2::ZERO);
    //   this->addChild(menu, 1);

    //   /////////////////////////////
    //   // 3. add your codes below...

    //   // add a label shows "Hello World"
    //   // create and initialize a label
    //   
    //   auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    //   
    //   // position the label on the center of the screen
    //   label->setPosition(Vec2(origin.x + visibleSize.width/2,
    //						   origin.y + visibleSize.height - label->getContentSize().height));

    //   // add the label as a child to this layer
    //   this->addChild(label, 1);

#if 0
    // add "HelloWorld" splash screen"
    auto sprite = MGRBlurSprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);

    // add "HelloWorld" splash screen"
    auto slider = Slider::create();
    slider->loadBarTexture("sliderTrack.png");
    slider->loadSlidBallTextures("sliderThumb.png", "sliderThumb.png", "");
    slider->loadProgressBarTexture("sliderProgress.png");
    slider->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/6 + origin.y));
    slider->addEventListener([=](Ref* sender, Slider::EventType type){
        if (type == Slider::EventType::ON_PERCENTAGE_CHANGED)
        {
            Slider* slider = dynamic_cast<Slider*>(sender);
            int percent = slider->getPercent();
            sprite->setUrate((float)percent / 100.0f);
        }
    });

    this->addChild(slider);
#endif



    //// cpp-testからCCDrawPrimitiveTestをもってきた
    //auto s = Director::getInstance()->getWinSize();

    //auto draw = MGRDrawNode::create();
    //addChild(draw);

    //draw->drawPoint(Vec2(s.width / 2 - 120, s.height / 2 - 120), 10, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));

    //draw->drawPoint(Vec2(s.width / 2 + 120, s.height / 2 + 120), 10, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));

    //// draw 4 small points
    //Vec2 position[] = { Vec2(60, 60), Vec2(70, 70), Vec2(60, 70), Vec2(70, 60) };
    //draw->drawPoints(position, 4, 5, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));

    //// draw a line
    //draw->drawLine(Vec2(0, 0), Vec2(s.width, s.height), Color4F(1.0, 0.0, 0.0, 0.5));

    //// draw a rectangle
    //draw->drawRect(Vec2(23, 23), Vec2(7, 7), Color4F(1, 1, 0, 1));

    //draw->drawRect(Vec2(15, 30), Vec2(30, 15), Vec2(15, 0), Vec2(0, 15), Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));

    //// draw a circle
    //draw->drawCircle(VisibleRect::center() + Vec2(140, 0), 100, CC_DEGREES_TO_RADIANS(90), 50, true, 1.0f, 2.0f, Color4F(1.0, 0.0, 0.0, 0.5));

    //draw->drawCircle(VisibleRect::center() - Vec2(140, 0), 50, CC_DEGREES_TO_RADIANS(90), 30, false, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));

    ////// Draw some beziers
    ////draw->drawQuadBezier(Vec2(s.width - 150, s.height - 150), Vec2(s.width - 70, s.height - 10), Vec2(s.width - 10, s.height - 10), 10, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 0.5));

    ////draw->drawQuadBezier(Vec2(0, s.height), Vec2(s.width / 2, s.height / 2), Vec2(s.width, s.height), 50, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 0.5));

    ////draw->drawCubicBezier(VisibleRect::center(), Vec2(VisibleRect::center().x + 30, VisibleRect::center().y + 50), Vec2(VisibleRect::center().x + 60, VisibleRect::center().y - 50), VisibleRect::right(), 100, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 0.5));

    ////draw->drawCubicBezier(Vec2(s.width - 250, 40), Vec2(s.width - 70, 100), Vec2(s.width - 30, 250), Vec2(s.width - 10, s.height - 50), 10, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 0.5));

    ////auto array = PointArray::create(20);
    ////array->addControlPoint(Vec2(0, 0));
    ////array->addControlPoint(Vec2(80, 80));
    ////array->addControlPoint(Vec2(s.width - 80, 80));
    ////array->addControlPoint(Vec2(s.width - 80, s.height - 80));
    ////array->addControlPoint(Vec2(80, s.height - 80));
    ////array->addControlPoint(Vec2(80, 80));
    ////array->addControlPoint(Vec2(s.width / 2, s.height / 2));
    ////draw->drawCardinalSpline(array, 0.5, 50, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 0.5));

    ////auto array2 = PointArray::create(20);
    ////array2->addControlPoint(Vec2(s.width / 2, 30));
    ////array2->addControlPoint(Vec2(s.width - 80, 30));
    ////array2->addControlPoint(Vec2(s.width - 80, s.height - 80));
    ////array2->addControlPoint(Vec2(s.width / 2, s.height - 80));
    ////array2->addControlPoint(Vec2(s.width / 2, 30));
    ////draw->drawCatmullRom(array2, 50, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 0.5));

    //// open random color poly
    //Vec2 vertices[] = { Vec2(0, 0), Vec2(50, 50), Vec2(100, 50), Vec2(100, 100), Vec2(50, 100) };
    //draw->drawPoly(vertices, 5, false, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));

    //// closed random color poly
    //Vec2 vertices2[] = { Vec2(30, 130), Vec2(30, 230), Vec2(50, 200) };
    //draw->drawPoly(vertices2, 3, true, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));

    //// Draw 10 circles
    //for (int i = 0; i < 10; i++)
    //{
    //	draw->drawDot(Vec2(s.width / 2, s.height / 2), 10 * (10 - i), Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));
    //}

    //// Draw polygons
    //Vec2 points[] = { Vec2(s.height / 4, 0), Vec2(s.width, s.height / 5), Vec2(s.width / 3 * 2, s.height) };
    //draw->drawPolygon(points, sizeof(points) / sizeof(points[0]), Color4F(1, 0, 0, 0.5), 4, Color4F(0, 0, 1, 0.5));

    //// star poly (triggers buggs)
    //{
    //	const float o = 80;
    //	const float w = 20;
    //	const float h = 50;
    //	Vec2 star[] = {
    //		Vec2(o + w, o - h), Vec2(o + w * 2, o),						// lower spike
    //		Vec2(o + w * 2 + h, o + w), Vec2(o + w * 2, o + w * 2),		// right spike
    //		//			  {o +w, o+w*2+h}, {o,o+w*2},				 // top spike
    //		//			  {o -h, o+w}, {o,o},						 // left spike
    //	};

    //	draw->drawPolygon(star, sizeof(star) / sizeof(star[0]), Color4F(1, 0, 0, 0.5), 1, Color4F(0, 0, 1, 1));
    //}

    //// star poly (doesn't trigger bug... order is important un tesselation is supported.
    //{
    //	const float o = 180;
    //	const float w = 20;
    //	const float h = 50;
    //	Vec2 star[] = {
    //		Vec2(o, o), Vec2(o + w, o - h), Vec2(o + w * 2, o),		// lower spike
    //		Vec2(o + w * 2 + h, o + w), Vec2(o + w * 2, o + w * 2),	// right spike
    //		Vec2(o + w, o + w * 2 + h), Vec2(o, o + w * 2),			   // top spike
    //		Vec2(o - h, o + w),									 // left spike
    //	};

    //	draw->drawPolygon(star, sizeof(star) / sizeof(star[0]), Color4F(1, 0, 0, 0.5), 1, Color4F(0, 0, 1, 1));
    //}

    ////draw a solid polygon
    //Vec2 vertices3[] = { Vec2(60, 160), Vec2(70, 190), Vec2(100, 190), Vec2(90, 160) };
    //draw->drawSolidPoly(vertices3, 4, Color4F(1, 1, 0, 1));

    ////draw a solid rectangle
    //draw->drawSolidRect(Vec2(10, 10), Vec2(20, 20), Color4F(1, 1, 0, 1));

    ////draw a solid circle
    //draw->drawSolidCircle(VisibleRect::center() + Vec2(140, 0), 40, CC_DEGREES_TO_RADIANS(90), 50, 2.0f, 2.0f, Color4F(0.0, 1.0, 0.0, 1.0));

    ////// Draw segment
    ////draw->drawSegment(Vec2(20, s.height), Vec2(20, s.height / 2), 10, Color4F(0, 1, 0, 1));

    ////draw->drawSegment(Vec2(10, s.height / 2), Vec2(s.width / 2, s.height / 2), 40, Color4F(1, 0, 1, 0.5));

    //// Draw triangle
    //draw->drawTriangle(Vec2(10, 10), Vec2(70, 30), Vec2(100, 140), Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 0.5));

    //for (int i = 0; i < 100; i++) {
    //	draw->drawPoint(Vec2(i * 7, 5), (float)i / 5 + 1, Color4F(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1));
    //}

    auto s = Director::getInstance()->getWinSize();

    // なぜかboss.c3bとReskinGirl.c3b以外のc3bファイルは真っ赤に表示される。原因調査中。
    //auto boss = Sprite3D::create("boss1.obj");
    //boss->setTexture("boss.png");
    //auto boss = MGRSprite3D::create("orc.c3b");
    //auto boss = Sprite3D::create("boss.c3b");
    //boss->setScale(6.0f);
    //boss->setPosition(Vec2(s.width * 3 / 4, s.height * 3 / 4));
    //addChild(boss);

    ////auto orc = MGRSprite3D::create("ReskinGirl.c3b");
    //_orc = Sprite3D::create("ReskinGirl.c3b");
    //_orc->setScale(4.0f);
    ////orc->setRotation3D(Vec3(0, 180, 0));
    //_orc->setPosition(Vec2(s.width / 2, s.height / 4));
    //addChild(_orc);

    //auto animation = Animation3D::create("ReskinGirl.c3b");
    //if (animation)
    //{
    //    auto animate = Animate3D::create(animation);

    //    _orc->runAction(RepeatForever::create(animate)); // ここでretainされている
    //}

    _orc = Sprite::create("HelloWorld.png");
    _orc->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    _orc->setScaleX(2.0f);
    _orc->setScaleY(-2.0f); // RenderTextureの設定で上下逆になってたので
    addChild(_orc); // addChildしないとDirectorからのupdateが呼ばれないのでアクションしない
    RotateBy* rotate = RotateBy::create(2.0f, 360.0f);
    RepeatForever* action = RepeatForever::create(rotate);
    _orc->runAction(action);

    _blurSprite = MGRBlurSprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    _blurSprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(_blurSprite, 0);


    // create a render texture, this is what we are going to draw into
    _texture = RenderTexture::create(s.width, s.height, Texture2D::PixelFormat::RGBA8888);
    _texture->retain();
    _texture->setPosition(Vec2(s.width / 2, s.height / 2));


    auto slider = Slider::create();
    slider->loadBarTexture("sliderTrack.png");
    slider->loadSlidBallTextures("sliderThumb.png", "sliderThumb.png", "");
    slider->loadProgressBarTexture("sliderProgress.png");
    slider->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/6 + origin.y));
    slider->addEventListener([=](Ref* sender, Slider::EventType type){
        if (type == Slider::EventType::ON_PERCENTAGE_CHANGED)
        {
            Slider* slider = dynamic_cast<Slider*>(sender);
            int percent = slider->getPercent();
            if (_blurSprite != nullptr)
            {
                _blurSprite->setUrate((float)percent / 100.0f);
            }
        }
    });

    this->addChild(slider);

    return true;
}

void HelloWorld::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
{
    Node::visit(renderer, parentTransform, parentFlags);

    _orc->setVisible(true);
    _texture->beginWithClear(0, 0, 0, 0, 0, 0);
    _orc->visit();
    _texture->end();
    _orc->setVisible(false);

    // 毎フレームテクスチャを初期化する
    _blurSprite->setTexture(_texture->getSprite()->getTexture());
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

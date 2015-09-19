/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#include "2d/MGRSprite.h"

#include <algorithm>

#include "renderer/CCTextureCache.h"
#include "renderer/CCTexture2D.h"
#include "renderer/CCRenderer.h"
#include "base/CCDirector.h"

#include "deprecated/CCString.h"


NS_CC_BEGIN

#if CC_SPRITEBATCHNODE_RENDER_SUBPIXEL
#define RENDER_IN_SUBPIXEL
#else
#define RENDER_IN_SUBPIXEL(__ARGS__) (ceil(__ARGS__))
#endif

MGRSprite* MGRSprite::create(const std::string& filename)
{
    MGRSprite* sprite = new (std::nothrow) MGRSprite();
    if (sprite && sprite->initWithFile(filename))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool MGRSprite::initWithTexture(Texture2D* texture)
{
    CCASSERT(texture != nullptr, "Invalid texture for sprite");

    Rect rect = Rect::ZERO;
    rect.size = texture->getContentSize();

    return initWithTexture(texture, rect);
}

bool MGRSprite::initWithTexture(Texture2D* texture, const Rect& rect)
{
    return initWithTexture(texture, rect, false);
}

bool MGRSprite::initWithFile(const std::string& filename)
{
    CCASSERT(filename.size() > 0, "Invalid filename for sprite");

    Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(filename);
    if (texture != nullptr)
    {
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        return initWithTexture(texture, rect);
    }

    // don't release here.
    // when load texture failed, it's better to get a "transparent" sprite then a crashed program
    // this->release();
    return false;
}

bool MGRSprite::initWithTexture(Texture2D* texture, const Rect& rect, bool rotated)
{
    bool result;
    if (Node::init())
    {
        _recursiveDirty = false;
        setDirty(false);

        _opacityModifyRGB = true;

        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

        _flippedX = _flippedY = false;

        // default transform anchor: center
        setAnchorPoint(Vec2(0.5f, 0.5f));

        // zwoptex default values
        _offsetPosition.setZero();

        // clean the Quad
        memset(&_quad, 0, sizeof(_quad));

        // Atlas: Color
        _quad.bl.colors = Color4B::WHITE;
        _quad.br.colors = Color4B::WHITE;
        _quad.tl.colors = Color4B::WHITE;
        _quad.tr.colors = Color4B::WHITE;

        // shader state
        setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));

        // update texture (calls updateBlendFunc)
        setTexture(texture);
        setTextureRect(rect, rotated, rect.size);

        _polyInfo.setQuad(&_quad);
        // by default use "Self Render".
        // if the sprite is added to a batchnode, then it will automatically switch to "batchnode Render"
        result = true;
    }
    else
    {
        result = false;
    }

    _recursiveDirty = true;
    setDirty(true);
    return result;
}

MGRSprite::MGRSprite(void)
: _shouldBeHidden(false)
, _texture(nullptr)
, _insideBounds(true)
{
#if CC_SPRITE_DEBUG_DRAW
    debugDraw(true);
#endif //CC_SPRITE_DEBUG_DRAW
}

MGRSprite::~MGRSprite(void)
{
    CC_SAFE_RELEASE(_texture);
}

/*
 * Texture methods
 */

/*
 * This array is the data of a white image with 2 by 2 dimension.
 * It's used for creating a default texture when sprite's texture is set to nullptr.
 * Supposing codes as follows:
 *
 *   auto sp = new (std::nothrow) MGRSprite();
 *   sp->init();  // Texture was set to nullptr, in order to make opacity and color to work correctly, we need to create a 2x2 white texture.
 *
 * The test is in "TestCpp/MGRSpriteTest/MGRSprite without texture".
 */
static unsigned char cc_2x2_white_image[] = {
    // RGBA8888
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};

#define CC_2x2_WHITE_IMAGE_KEY  "/cc_2x2_white_image"

// MARK: texture
void MGRSprite::setTexture(Texture2D* texture)
{
    // accept texture==nil as argument
    CCASSERT(texture == nullptr || dynamic_cast<Texture2D*>(texture) != nullptr, "setTexture expects a Texture2D. Invalid argument");

    if (texture == nullptr)
    {
        // Gets the texture by key firstly.
        texture = Director::getInstance()->getTextureCache()->getTextureForKey(CC_2x2_WHITE_IMAGE_KEY);

        // If texture wasn't in cache, create it from RAW data.
        if (texture == nullptr)
        {
            Image* image = new (std::nothrow) Image();
            bool isOK = image->initWithRawData(cc_2x2_white_image, sizeof(cc_2x2_white_image), 2, 2, 8);
            CC_UNUSED_PARAM(isOK);
            CCASSERT(isOK, "The 2x2 empty texture was created unsuccessfully.");

            texture = Director::getInstance()->getTextureCache()->addImage(image, CC_2x2_WHITE_IMAGE_KEY);
            CC_SAFE_RELEASE(image);
        }
    }

    if (_texture != texture)
    {
        CC_SAFE_RETAIN(texture);
        CC_SAFE_RELEASE(_texture);
        _texture = texture;
        updateBlendFunc();
    }
}

Texture2D* MGRSprite::getTexture() const
{
    return _texture;
}

void MGRSprite::setTextureRect(const Rect& rect, bool rotated, const Size& untrimmedSize)
{
    _rectRotated = rotated;

    setContentSize(untrimmedSize);
    setVertexRect(rect);
    setTextureCoords(rect);

    _offsetPosition.x = (_contentSize.width - _rect.size.width) / 2;
    _offsetPosition.y = (_contentSize.height - _rect.size.height) / 2;

    {
        // self rendering

        // Atlas: Vertex
        float x1 = 0.0f + _offsetPosition.x;
        float y1 = 0.0f + _offsetPosition.y;
        float x2 = x1 + _rect.size.width;
        float y2 = y1 + _rect.size.height;

        // Don't update Z.
        _quad.bl.vertices.set(x1, y1, 0.0f);
        _quad.br.vertices.set(x2, y1, 0.0f);
        _quad.tl.vertices.set(x1, y2, 0.0f);
        _quad.tr.vertices.set(x2, y2, 0.0f);
    }
}

// override this method to generate "double scale" sprites
void MGRSprite::setVertexRect(const Rect& rect)
{
    _rect = rect;
}

void MGRSprite::setTextureCoords(Rect rect)
{
    rect = CC_RECT_POINTS_TO_PIXELS(rect);

    Texture2D* tex = _texture;
    if (tex == nullptr)
    {
        return;
    }

    float atlasWidth = (float)tex->getPixelsWide();
    float atlasHeight = (float)tex->getPixelsHigh();

    float left, right, top, bottom;

    if (_rectRotated)
    {
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
        left = (2 * rect.origin.x + 1) / (2 * atlasWidth)
        right = left + (rect.size.height * 2 - 2) / (2 * atlasWidth);
        top = (2 * rect.origin.y + 1) / (2 * atlasHeight);
        bottom = top + (rect.size.width * 2 - 2) / (2 * atlasHeight);
#else
        left = rect.origin.x / atlasWidth;
        right = (rect.origin.x + rect.size.height) / atlasWidth;
        top = rect.origin.y / atlasHeight;
        bottom = (rect.origin.y + rect.size.width) / atlasHeight;
#endif

        if (_flippedX)
        {
            std::swap(top, bottom);
        }

        if (_flippedY)
        {
            std::swap(left, right);
        }

        _quad.bl.texCoords.u = left;
        _quad.bl.texCoords.v = top;
        _quad.br.texCoords.u = left;
        _quad.br.texCoords.v = bottom;
        _quad.tl.texCoords.u = right;
        _quad.tl.texCoords.v = top;
        _quad.tr.texCoords.u = right;
        _quad.tr.texCoords.v = bottom;
    }
    else
    {
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
        left = (2 * rect.origin.x + 1) / (2 * atlasWidth)
        right = left + (rect.size.width * 2 - 2) / (2 * atlasWidth);
        top = (2 * rect.origin.y + 1) / (2 * atlasHeight);
        bottom = top + (rect.size.height * 2 - 2) / (2 * atlasHeight);
#else
        left = rect.origin.x / atlasWidth;
        right = (rect.origin.x + rect.size.width) / atlasWidth;
        top = rect.origin.y / atlasHeight;
        bottom = (rect.origin.y + rect.size.height) / atlasHeight;
#endif

        if (_flippedX)
        {
            std::swap(left, right);
        }

        if (_flippedY)
        {
            std::swap(top, bottom);
        }

        _quad.bl.texCoords.u = left;
        _quad.bl.texCoords.v = bottom;
        _quad.br.texCoords.u = right;
        _quad.br.texCoords.v = bottom;
        _quad.tl.texCoords.u = left;
        _quad.tl.texCoords.v = top;
        _quad.tr.texCoords.u = right;
        _quad.tr.texCoords.v = top;
    }
}

// draw

void MGRSprite::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
#if CC_USE_CULLING
    // Don't do calculate the culling if the transform was not updated
    _insideBounds = (flags & FLAGS_TRANSFORM_DIRTY) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;
    if (_insideBounds)
#endif
    {
        _trianglesCommand.init(_globalZOrder, _texture->getName(), getGLProgramState(), _blendFunc, _polyInfo.triangles, transform, flags);
        renderer->addCommand(&_trianglesCommand);
    }
}

// MARK: visit, draw, transform

void MGRSprite::addChild(Node* child, int zOrder, int tag)
{
    CCASSERT(child != nullptr, "Argument must be non-nullptr");

    //CCNode already sets isReorderChildDirty_ so this needs to be after batchNode check
    Node::addChild(child, zOrder, tag);
}

void MGRSprite::addChild(Node* child, int zOrder, const std::string &name)
{
    CCASSERT(child != nullptr, "Argument must be non-nullptr");

    //CCNode already sets isReorderChildDirty_ so this needs to be after batchNode check
    Node::addChild(child, zOrder, name);
}

void MGRSprite::reorderChild(Node* child, int zOrder)
{
    CCASSERT(child != nullptr, "child must be non null");
    CCASSERT(_children.contains(child), "child does not belong to this");

    Node::reorderChild(child, zOrder);
}

void MGRSprite::removeChild(Node *child, bool cleanup)
{
    Node::removeChild(child, cleanup);
}

void MGRSprite::removeAllChildrenWithCleanup(bool cleanup)
{
    Node::removeAllChildrenWithCleanup(cleanup);
}

void MGRSprite::sortAllChildren()
{
    if (_reorderChildDirty)
    {
        std::sort(std::begin(_children), std::end(_children), nodeComparisonLess);

        _reorderChildDirty = false;
    }
}

//
// Node property overloads
// used only when parent is SpriteBatchNode
//

void MGRSprite::setReorderChildDirtyRecursively(void)
{
    //only set parents flag the first time
    if ( ! _reorderChildDirty )
    {
        _reorderChildDirty = true;
        Node* node = static_cast<Node*>(_parent);
        while (node)
        {
            static_cast<MGRSprite*>(node)->setReorderChildDirtyRecursively();
            node=node->getParent();
        }
    }
}

void MGRSprite::setDirtyRecursively(bool bValue)
{
    _recursiveDirty = bValue;
    setDirty(bValue);

    for(const auto &child: _children) {
        MGRSprite* sp = dynamic_cast<MGRSprite*>(child);
        if (sp)
        {
            sp->setDirtyRecursively(true);
        }
    }
}

// FIXME: HACK: optimization
#define SET_DIRTY_RECURSIVELY() {                       \
                    if (! _recursiveDirty) {            \
                        _recursiveDirty = true;         \
                        setDirty(true);                 \
                        if (!_children.empty())         \
                            setDirtyRecursively(true);  \
                        }                               \
                    }

void MGRSprite::setPosition(const Vec2& pos)
{
    Node::setPosition(pos);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setPosition(float x, float y)
{
    Node::setPosition(x, y);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setRotation(float rotation)
{
    Node::setRotation(rotation);
    
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setRotationSkewX(float fRotationX)
{
    Node::setRotationSkewX(fRotationX);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setRotationSkewY(float fRotationY)
{
    Node::setRotationSkewY(fRotationY);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setSkewX(float sx)
{
    Node::setSkewX(sx);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setSkewY(float sy)
{
    Node::setSkewY(sy);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setScaleX(float scaleX)
{
    Node::setScaleX(scaleX);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setScaleY(float scaleY)
{
    Node::setScaleY(scaleY);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setScale(float fScale)
{
    Node::setScale(fScale);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setScale(float scaleX, float scaleY)
{
    Node::setScale(scaleX, scaleY);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setPositionZ(float fVertexZ)
{
    Node::setPositionZ(fVertexZ);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setAnchorPoint(const Vec2& anchor)
{
    Node::setAnchorPoint(anchor);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::ignoreAnchorPointForPosition(bool value)
{
    Node::ignoreAnchorPointForPosition(value);
}

void MGRSprite::setVisible(bool bVisible)
{
    Node::setVisible(bVisible);
    SET_DIRTY_RECURSIVELY();
}

void MGRSprite::setFlippedX(bool flippedX)
{
    if (_flippedX != flippedX)
    {
        _flippedX = flippedX;
        setTextureRect(_rect, _rectRotated, _contentSize);
    }
}

bool MGRSprite::isFlippedX(void) const
{
    return _flippedX;
}

void MGRSprite::setFlippedY(bool flippedY)
{
    if (_flippedY != flippedY)
    {
        _flippedY = flippedY;
        setTextureRect(_rect, _rectRotated, _contentSize);
    }
}

bool MGRSprite::isFlippedY(void) const
{
    return _flippedY;
}

//
// MARK: RGBA protocol
//

void MGRSprite::updateColor(void)
{
    Color4B color4(_displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity);

    if (_opacityModifyRGB)
    {
        color4.r *= _displayedOpacity / 255.0f;
        color4.g *= _displayedOpacity / 255.0f;
        color4.b *= _displayedOpacity / 255.0f;
    }

    _quad.bl.colors = color4;
    _quad.br.colors = color4;
    _quad.tl.colors = color4;
    _quad.tr.colors = color4;

    // self render
    // do nothing
}

void MGRSprite::setOpacityModifyRGB(bool modify)
{
    if (_opacityModifyRGB != modify)
    {
        _opacityModifyRGB = modify;
        updateColor();
    }
}

bool MGRSprite::isOpacityModifyRGB(void) const
{
    return _opacityModifyRGB;
}

// MARK: Texture protocol

void MGRSprite::updateBlendFunc(void)
{
    // it is possible to have an untextured spritec
    if (_texture == nullptr || !_texture->hasPremultipliedAlpha())
    {
        _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
        setOpacityModifyRGB(false);
    }
    else
    {
        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
        setOpacityModifyRGB(true);
    }
}

PolygonInfo MGRSprite::getPolygonInfo() const
{
    return _polyInfo;
}

void MGRSprite::setPolygonInfo(const PolygonInfo& info)
{
    _polyInfo = info;
}

NS_CC_END

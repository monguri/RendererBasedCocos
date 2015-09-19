#pragma once

#include <string>
#include "2d/CCNode.h"
#include "2d/MGRDrawNode.h"
#include "base/CCProtocols.h"
#include "renderer/CCTextureAtlas.h"
#include "renderer/CCTrianglesCommand.h"
#include "renderer/CCCustomCommand.h"
#include "2d/CCAutoPolygon.h"

NS_CC_BEGIN

class Animation;
class Rect;
class Size;
class Texture2D;
struct transformValues_;

/**
 * @addtogroup _2d
 * @{
 */

/**
 * MGRSprite is a 2d image ( http://en.wikipedia.org/wiki/MGRSprite_(computer_graphics) ).
 *
 * MGRSprite can be created with an image, or with a sub-rectangle of an image.
 *
 * To optimize the MGRSprite rendering, please follow the following best practices:
 *  - Put all your sprites in the same spritesheet (http://www.codeandweb.com/what-is-a-sprite-sheet).
 *  - Use the same blending function for all your sprites.
 *  - ...and the Renderer will automatically "batch" your sprites (will draw all of them in one OpenGL call).
 *
 *  To gain an additional 5% ~ 10% more in the rendering, you can parent your sprites into a `SpriteBatchNode`.
 *  But doing so carries the following limitations:
 *
 *  - The Alias/Antialias property belongs to `SpriteBatchNode`, so you can't individually set the aliased property.
 *  - The Blending function property belongs to `SpriteBatchNode`, so you can't individually set the blending function property.
 *  - `ParallaxNode` is not supported, but can be simulated with a "proxy" sprite.
 *  - MGRSprites can only have other MGRSprites (or subclasses of MGRSprite) as children.
 *
 * The default anchorPoint in MGRSprite is (0.5, 0.5).
 */
class CC_DLL MGRSprite : public Node, public TextureProtocol
{
public:
     /** MGRSprite invalid index on the SpriteBatchNode. */
    static const int INDEX_NOT_INITIALIZED = -1;

    /// @name Creators
    /// @{

    /**
     * Creates a sprite with an image filename.
     *
     * After creation, the rect of sprite will be the size of the image,
     * and the offset will be (0,0).
     *
     * @param   filename A path to image file, e.g., "scene1/monster.png".
     * @return  An autoreleased sprite object.
     */
    static MGRSprite* create(const std::string& filename);

    //  end of creators group
    /// @}

    /// @{
    /// @name Texture / Frame methods

    /** @overload
     *
     *  The Texture's rect is not changed.
     */
    virtual void setTexture(Texture2D *texture) override;

    /** Returns the Texture2D object used by the sprite. */
    virtual Texture2D* getTexture() const override;

    /** @overload
     *
     * It will update the texture coordinates and the vertex rectangle.
     */
    virtual void setTextureRect(const Rect& rect, bool rotated, const Size& untrimmedSize);

    /**
     * Sets the vertex rect.
     *
     * It will be called internally by setTextureRect.
     * Useful if you want to create 2x images from SD images in Retina Display.
     * Do not call it manually. Use setTextureRect instead.
     */
    virtual void setVertexRect(const Rect& rect);

    /// @} End of frames methods


    /// @{
    /// @name MGRSprite Properties' setter/getters.

    /**
     * Whether or not the MGRSprite needs to be updated in the Atlas.
     *
     * @return True if the sprite needs to be updated in the Atlas, false otherwise.
     */
    virtual bool isDirty() const { return _dirty; }

    /**
     * Makes the MGRSprite to be updated in the Atlas.
     */
    virtual void setDirty(bool dirty) { _dirty = dirty; }

    /**
     * Returns the quad (tex coords, vertex coords and color) information.
     * @js  NA
     * @lua NA
     */
    inline V3F_C4B_T2F_Quad getQuad() const { return _quad; }

    /**
     * Returns whether or not the texture rectangle is rotated.
     */
    inline bool isTextureRectRotated() const { return _rectRotated; }

    /**
     * Returns the rect of the MGRSprite in points.
     */
    inline const Rect& getTextureRect() const { return _rect; }

    /**
     * Gets the offset position of the sprite. Calculated automatically by editors like Zwoptex.
     */
    inline const Vec2& getOffsetPosition() const { return _offsetPosition; }


    /**
     * Returns the flag which indicates whether the sprite is flipped horizontally or not.
     *
     * It only flips the texture of the sprite, and not the texture of the sprite's children.
     * Also, flipping the texture doesn't alter the anchorPoint.
     * If you want to flip the anchorPoint too, and/or to flip the children too use:
     * sprite->setScaleX(sprite->getScaleX() * -1);
     *
     * @return true if the sprite is flipped horizontally, false otherwise.
     */
    bool isFlippedX() const;
    /**
     * Sets whether the sprite should be flipped horizontally or not.
     *
     * @param flippedX true if the sprite should be flipped horizontally, false otherwise.
     */
    void setFlippedX(bool flippedX);

    /** @deprecated Use isFlippedX() instead.
    * @js NA
    * @lua NA
    */
    CC_DEPRECATED_ATTRIBUTE bool isFlipX() { return isFlippedX(); };
    /** @deprecated Use setFlippedX() instead
     * @js NA
     */
    CC_DEPRECATED_ATTRIBUTE void setFlipX(bool flippedX) { setFlippedX(flippedX); };

    /**
     * Return the flag which indicates whether the sprite is flipped vertically or not.
     *
     * It only flips the texture of the sprite, and not the texture of the sprite's children.
     * Also, flipping the texture doesn't alter the anchorPoint.
     * If you want to flip the anchorPoint too, and/or to flip the children too use:
     * sprite->setScaleY(sprite->getScaleY() * -1);
     *
     * @return true if the sprite is flipped vertically, false otherwise.
     */
    bool isFlippedY() const;
    /**
     * Sets whether the sprite should be flipped vertically or not.
     *
     * @param flippedY true if the sprite should be flipped vertically, false otherwise.
     */
    void setFlippedY(bool flippedY);

    /// @} End of MGRSprite properties getter/setters

    /** @deprecated Use isFlippedY() instead.
     * @js NA
     */
    CC_DEPRECATED_ATTRIBUTE bool isFlipY() { return isFlippedY(); };
    /** @deprecated Use setFlippedY() instead.
     * @js NA
     */
    CC_DEPRECATED_ATTRIBUTE void setFlipY(bool flippedY) { setFlippedY(flippedY); };

    //
    // Overrides
    //
    /// @{
    /// @name Functions inherited from TextureProtocol.
    /**
    *@code
    *When this function bound into js or lua,the parameter will be changed.
    *In js: var setBlendFunc(var src, var dst).
    *In lua: local setBlendFunc(local src, local dst).
    *@endcode
    */
    inline void setBlendFunc(const BlendFunc &blendFunc) override { _blendFunc = blendFunc; }
    /**
    * @js  NA
    * @lua NA
    */
    inline const BlendFunc& getBlendFunc() const override { return _blendFunc; }
    /// @}

    /// @{
    /// @name Functions inherited from Node.
    virtual void setScaleX(float scaleX) override;
    virtual void setScaleY(float scaleY) override;
    virtual void setScale(float scaleX, float scaleY) override;
    /**
    * @js  NA
    * @lua NA
    */
    virtual void setPosition(const Vec2& pos) override;
    virtual void setPosition(float x, float y) override;
    virtual void setRotation(float rotation) override;
    virtual void setRotationSkewX(float rotationX) override;
    virtual void setRotationSkewY(float rotationY) override;
    virtual void setSkewX(float sx) override;
    virtual void setSkewY(float sy) override;
    virtual void removeChild(Node* child, bool cleanup) override;
    virtual void removeAllChildrenWithCleanup(bool cleanup) override;
    virtual void reorderChild(Node *child, int zOrder) override;
    using Node::addChild;
    virtual void addChild(Node *child, int zOrder, int tag) override;
    virtual void addChild(Node *child, int zOrder, const std::string &name) override;
    virtual void sortAllChildren() override;
    virtual void setScale(float scale) override;
    virtual void setPositionZ(float positionZ) override;
    virtual void setAnchorPoint(const Vec2& anchor) override;
    virtual void ignoreAnchorPointForPosition(bool value) override;
    virtual void setVisible(bool bVisible) override;
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    virtual void setOpacityModifyRGB(bool modify) override;
    virtual bool isOpacityModifyRGB() const override;
    /// @}

CC_CONSTRUCTOR_ACCESS:
	/**
     * @js ctor
     */
    MGRSprite();
    virtual ~MGRSprite();

    /**
     * Initializes a sprite with a texture.
     *
     * After initialization, the rect used will be the size of the texture, and the offset will be (0,0).
     *
     * @param   texture    A pointer to an existing Texture2D object.
     *                      You can use a Texture2D object for many sprites.
     * @return  True if the sprite is initialized properly, false otherwise.
     */
    virtual bool initWithTexture(Texture2D *texture);

    /**
     * Initializes a sprite with a texture and a rect.
     *
     * After initialization, the offset will be (0,0).
     *
     * @param   texture    A pointer to an exisiting Texture2D object.
     *                      You can use a Texture2D object for many sprites.
     * @param   rect        Only the contents inside rect of this texture will be applied for this sprite.
     * @return  True if the sprite is initialized properly, false otherwise.
     */
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect);

    /**
     * Initializes a sprite with a texture and a rect in points, optionally rotated.
     *
     * After initialization, the offset will be (0,0).
     * @note    This is the designated initializer.
     *
     * @param   texture    A Texture2D object whose texture will be applied to this sprite.
     * @param   rect        A rectangle assigned the contents of texture.
     * @param   rotated     Whether or not the texture rectangle is rotated.
     * @return  True if the sprite is initialized properly, false otherwise.
     */
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated);

    /**
     * Initializes a sprite with an image filename.
     *
     * This method will find filename from local file system, load its content to Texture2D,
     * then use Texture2D to create a sprite.
     * After initialization, the rect used will be the size of the image. The offset will be (0,0).
     *
     * @param   filename The path to an image file in local file system.
     * @return  True if the sprite is initialized properly, false otherwise.
     * @lua     init
     */
    virtual bool initWithFile(const std::string& filename);
    
    /**
     * returns a copy of the polygon information associated with this sprite
     * because this is a copy process it is slower than getting the reference, so use wisely
     *
     * @return a copy of PolygonInfo
     */
    PolygonInfo getPolygonInfo() const;
    
    /**
     * set the sprite to use this new PolygonInfo
     *
     * @param PolygonInfo the polygon information object
     */
    void setPolygonInfo(const PolygonInfo& info);
protected:

    void updateColor() override;
    virtual void setTextureCoords(Rect rect);
    virtual void updateBlendFunc();
    virtual void setReorderChildDirtyRecursively();
    virtual void setDirtyRecursively(bool value);


    bool                _dirty;             /// Whether the sprite needs to be updated
    bool                _recursiveDirty;    /// Whether all of the sprite's children needs to be updated
    bool                _shouldBeHidden;    /// should not be drawn because one of the ancestors is not visible
    Mat4              _transformToBatch;

    //
    // Data used when the sprite is self-rendered
    //
    BlendFunc        _blendFunc;            /// It's required for TextureProtocol inheritance
    Texture2D*       _texture;              /// Texture2D object that is used to render the sprite
    TrianglesCommand _trianglesCommand;     ///


    //
    // Shared data
    //

    // texture
    Rect _rect;                             /// Retangle of Texture2D
    bool   _rectRotated;                    /// Whether the texture is rotated

    // Offset Position (used by Zwoptex)
    Vec2 _offsetPosition;
    Vec2 _unflippedOffsetPositionFromCenter;

    // vertex coords, texture coords and color info
    V3F_C4B_T2F_Quad _quad;
    PolygonInfo  _polyInfo;

    // opacity and RGB protocol
    bool _opacityModifyRGB;

    // image is flipped
    bool _flippedX;                         /// Whether the sprite is flipped horizontally or not
    bool _flippedY;                         /// Whether the sprite is flipped vertically or not

    bool _insideBounds;                     /// whether or not the sprite was inside bounds the previous frame
private:
    CC_DISALLOW_COPY_AND_ASSIGN(MGRSprite);
};


// end of sprite_nodes group
/// @}

NS_CC_END

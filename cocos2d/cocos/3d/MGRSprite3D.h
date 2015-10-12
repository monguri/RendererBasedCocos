#pragma once

#include <unordered_map>

#include "base/CCVector.h"
#include "base/ccTypes.h"
#include "base/CCProtocols.h"
#include "2d/CCNode.h"
#include "renderer/CCMeshCommand.h"
#include "renderer/CCGLProgramState.h"
#include "3d/CCSkeleton3D.h" // need to include for lua-binding
#include "3d/CCAABB.h"
#include "3d/CCBundle3DData.h"
#include "3d/CCMeshVertexIndexData.h"


NS_CC_BEGIN

/**
 * @addtogroup _3d
 * @{
 */

class Mesh;
class Texture2D;
class MeshSkin;
class AttachNode;
struct NodeData;
/** @brief MGRSprite3D: A sprite can be loaded from 3D model files, .obj, .c3t, .c3b, then can be drawed as sprite */
class CC_DLL MGRSprite3D : public Node, public BlendProtocol
{
public:
    /**
     * Creates an empty sprite3D without 3D model and texture.
     *
     * @return An autoreleased sprite3D object.
     */
    static MGRSprite3D* create();
    
    /** creates a MGRSprite3D*/
    static MGRSprite3D* create(const std::string &modelPath);
  
    // creates a MGRSprite3D. It only supports one texture, and overrides the internal texture with 'texturePath'
    static MGRSprite3D* create(const std::string &modelPath, const std::string &texturePath);
    
    /**set texture, set the first if multiple textures exist*/
    void setTexture(const std::string& texFile);
    void setTexture(Texture2D* texture);
    
    /**get Mesh by index*/
    Mesh* getMeshByIndex(int index) const;
    
    /**get Mesh by Name, it returns the first one if there are more than one mesh with the same name */
    Mesh* getMeshByName(const std::string& name) const;
    
    /** 
     * get mesh array by name, returns all meshes with the given name
     *
     * @lua NA
     */
    std::vector<Mesh*> getMeshArrayByName(const std::string& name) const;

    /**get mesh*/
    Mesh* getMesh() const { return _meshes.at(0); }
    
    /** get mesh count */
    ssize_t getMeshCount() const { return _meshes.size(); }
    
    /**get skin*/
    CC_DEPRECATED_ATTRIBUTE MeshSkin* getSkin() const;
    
    Skeleton3D* getSkeleton() const { return _skeleton; }
    
    /**get AttachNode by bone name, return nullptr if not exist*/
    AttachNode* getAttachNode(const std::string& boneName);
    
    /**remove attach node*/
    void removeAttachNode(const std::string& boneName);
    
    /**remove all attach nodes*/
    void removeAllAttachNode();

    // overrides
    virtual void setBlendFunc(const BlendFunc &blendFunc) override;
    virtual const BlendFunc &getBlendFunc() const override;
    
    // overrides
    /** set GLProgramState, you should bind attributes by yourself */
    virtual void setGLProgramState(GLProgramState *glProgramState) override;
    /** just rember bind attributes */
    virtual void setGLProgram(GLProgram *glprogram) override;
    
    /*
     * Get AABB
     * If the sprite has animation, it can't be calculated accuratly,
     * because bone can drive the vertices, we just use the origin vertices
     * to calculate the AABB.
     */
    const AABB& getAABB() const;
    
    /* 
     * Get AABB Recursively
     * Because some times we may have an empty MGRSprite3D Node as parent, but
     * the MGRSprite3D don't contain any meshes, so getAABB()
     * will return a wrong value at that time.
     */
    AABB getAABBRecursively();
    
    /**
     * Executes an action, and returns the action that is executed. For MGRSprite3D special logic are needed to take care of Fading.
     *
     * This node becomes the action's target. Refer to Action::getTarget()
     * @warning Actions don't retain their target.
     *
     * @return An Action pointer
     */
    virtual Action* runAction(Action* action) override;
    
    /**
     * Force to write to depth buffer, this is useful if you want to achieve effects like fading.
     */
    void setForceDepthWrite(bool value) { _forceDepthWrite = value; }
    bool isForceDepthWrite() const { return _forceDepthWrite;};
    
    /**
     * Returns 2d bounding-box
     * Note: the bouding-box is just get from the AABB which as Z=0, so that is not very accurate.
     */
    virtual Rect getBoundingBox() const override;

    // set which face is going to cull, GL_BACK, GL_FRONT, GL_FRONT_AND_BACK, default GL_BACK
    void setCullFace(GLenum cullFace);
    // set cull face enable or not
    void setCullFaceEnabled(bool enable);
    
    /** light mask getter & setter, light works only when _lightmask & light's flag is true, default value of _lightmask is 0xffff */
    void setLightMask(unsigned int mask) { _lightMask = mask; }
    unsigned int getLightMask() const { return _lightMask; }
    
    /**draw*/
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;

    /** Adds a new material to the sprite.
     The Material will be applied to all the meshes that belong to the sprite.
     Internally it will call `setMaterial(material,-1)`
     */
    void setMaterial(Material* material);

    /** Adds a new material to a particular mesh of the sprite.
     meshIndex is the mesh that will be applied to.
     if meshIndex == -1, then it will be applied to all the meshes that belong to the sprite.
     */
    void setMaterial(Material* material, int meshIndex);

    /** Adds a new material to a particular mesh of the sprite.
     meshIndex is the mesh that will be applied to.
     if meshIndex == -1, then it will be applied to all the meshes that belong to the sprite.
     */
    Material* getMaterial(int meshIndex) const;
    
    /**
    * force set this MGRSprite3D to 2D render queue
    */
    void setForce2DQueue(bool force2D);

CC_CONSTRUCTOR_ACCESS:
    
    MGRSprite3D();
    virtual ~MGRSprite3D();
    
    virtual bool init() override;
    
    bool initWithFile(const std::string &path);
    
    bool initFrom(const NodeDatas& nodedatas, const MeshDatas& meshdatas, const MaterialDatas& materialdatas);
    
    /** load file and set it to meshedatas, nodedatas and materialdatas, obj file .mtl file should be at the same directory if exist */
    bool loadFromFile(const std::string& path, NodeDatas* nodedatas, MeshDatas* meshdatas,  MaterialDatas* materialdatas);

    /**
     * Visits this MGRSprite3D's children and draw them recursively.
     * Note: all its children will rendered as 3D objects
     */
    virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags) override;
    
    /**generate default GLProgramState*/
    void genGLProgramState(bool useLight = false);

    void createNode(NodeData* nodedata, Node* root, const MaterialDatas& matrialdatas, bool singleSprite);
    void createAttachSprite3DNode(NodeData* nodedata,const MaterialDatas& matrialdatas);
    MGRSprite3D* createSprite3DNode(NodeData* nodedata,ModelData* modeldata,const MaterialDatas& matrialdatas);

    /**get MeshIndexData by Id*/
    MeshIndexData* getMeshIndexData(const std::string& indexId) const;
    
    void addMesh(Mesh* mesh);
    
    void onAABBDirty() { _aabbDirty = true; }
    
protected:

    Skeleton3D*                  _skeleton; //skeleton
    
    Vector<MeshVertexData*>      _meshVertexDatas;
    
    std::unordered_map<std::string, AttachNode*> _attachments;

    BlendFunc                    _blend;
    
    Vector<Mesh*>              _meshes;

    mutable AABB                 _aabb;                 // cache current aabb
    mutable Mat4                 _nodeToWorldTransform; // cache the matrix
    mutable bool                 _aabbDirty;
    unsigned int                 _lightMask;
    bool                         _shaderUsingLight; // is current shader using light ?
    bool                         _forceDepthWrite; // Always write to depth buffer
    bool                         _usingAutogeneratedGLProgram;
    
};


// end of 3d group
/// @}

NS_CC_END


#include "2d/MGRDrawNode.h"
#include "base/CCEventType.h"
#include "base/CCConfiguration.h"
#include "renderer/CCRenderer.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCGLProgramState.h"
#include "renderer/CCGLProgramCache.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventDispatcher.h"
#include "2d/CCActionCatmullRom.h"
#include "platform/CCGL.h"

NS_CC_BEGIN

// Vec2 == CGPoint in 32-bits, but not in 64-bits (OS X)
// that's why the "v2f" functions are needed
static Vec2 v2fzero(0.0f,0.0f);

static inline Vec2 v2f(float x, float y)
{
    Vec2 ret(x, y);
    return ret;
}

static inline Vec2 v2fadd(const Vec2 &v0, const Vec2 &v1)
{
    return v2f(v0.x+v1.x, v0.y+v1.y);
}

static inline Vec2 v2fsub(const Vec2 &v0, const Vec2 &v1)
{
    return v2f(v0.x-v1.x, v0.y-v1.y);
}

static inline Vec2 v2fmult(const Vec2 &v, float s)
{
    return v2f(v.x * s, v.y * s);
}

static inline Vec2 v2fperp(const Vec2 &p0)
{
    return v2f(-p0.y, p0.x);
}

static inline Vec2 v2fneg(const Vec2 &p0)
{
    return v2f(-p0.x, - p0.y);
}

static inline float v2fdot(const Vec2 &p0, const Vec2 &p1)
{
    return  p0.x * p1.x + p0.y * p1.y;
}

static inline Vec2 v2fforangle(float _a_)
{
    return v2f(cosf(_a_), sinf(_a_));
}

static inline Vec2 v2fnormalize(const Vec2 &p)
{
    Vec2 r(p.x, p.y);
    r.normalize();
    return v2f(r.x, r.y);
}

static inline Vec2 __v2f(const Vec2 &v)
{
//#ifdef __LP64__
    return v2f(v.x, v.y);
// #else
//     return * ((Vec2*) &v);
// #endif
}

static inline Tex2F __t(const Vec2 &v)
{
    return *(Tex2F*)&v;
}

// implementation of MGRDrawNode

MGRDrawNode::MGRDrawNode()
: _vao(0)
, _vbo(0)
, _vaoGLPoint(0)
, _vboGLPoint(0)
, _vaoGLLine(0)
, _vboGLLine(0)
, _bufferCapacity(0)
, _bufferCount(0)
, _buffer(nullptr)
, _bufferCapacityGLPoint(0)
, _bufferCountGLPoint(0)
, _bufferGLPoint(nullptr)
, _bufferCapacityGLLine(0)
, _bufferCountGLLine(0)
, _bufferGLLine(nullptr)
, _dirty(false)
, _dirtyGLPoint(false)
, _dirtyGLLine(false)
{
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
}

MGRDrawNode::~MGRDrawNode()
{
    free(_buffer);
    _buffer = nullptr;
    free(_bufferGLPoint);
    _bufferGLPoint = nullptr;
    free(_bufferGLLine);
    _bufferGLLine = nullptr;
    
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_vboGLLine);
    glDeleteBuffers(1, &_vboGLPoint);
    _vbo = 0;
    _vboGLPoint = 0;
    _vboGLLine = 0;
    
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(0);
        glDeleteVertexArrays(1, &_vao);
        glDeleteVertexArrays(1, &_vaoGLLine);
        glDeleteVertexArrays(1, &_vaoGLPoint);
        _vao = _vaoGLLine = _vaoGLPoint = 0;
    }
}

MGRDrawNode* MGRDrawNode::create()
{
    MGRDrawNode* ret = new (std::nothrow) MGRDrawNode();
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

void MGRDrawNode::ensureCapacity(int count)
{
    CCASSERT(count >= 0, "capacity must be >= 0");

    if (_bufferCount + count > _bufferCapacity)
    {
        _bufferCapacity += MAX(_bufferCapacity, count);
        _buffer = (V2F_C4B_T2F*)realloc(_buffer, _bufferCapacity*sizeof(V2F_C4B_T2F));
    }
}

void MGRDrawNode::ensureCapacityGLPoint(int count)
{
    CCASSERT(count >= 0, "capacity must be >= 0");

    if (_bufferCountGLPoint + count > _bufferCapacityGLPoint)
    {
        _bufferCapacityGLPoint += MAX(_bufferCapacityGLPoint, count);
        _bufferGLPoint = (V2F_C4B_T2F*)realloc(_bufferGLPoint, _bufferCapacityGLPoint*sizeof(V2F_C4B_T2F));
    }
}

void MGRDrawNode::ensureCapacityGLLine(int count)
{
    CCASSERT(count >= 0, "capacity must be >= 0");

    if (_bufferCountGLLine + count > _bufferCapacityGLLine)
    {
        _bufferCapacityGLLine += MAX(_bufferCapacityGLLine, count);
        _bufferGLLine = (V2F_C4B_T2F*)realloc(_bufferGLLine, _bufferCapacityGLLine*sizeof(V2F_C4B_T2F));
    }
}

bool MGRDrawNode::init()
{
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR));
    
    ensureCapacity(512);
    ensureCapacityGLPoint(64);
    ensureCapacityGLLine(256);
    
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacity, _buffer, GL_STREAM_DRAW);

    glGenBuffers(1, &_vboGLLine);
    glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);

    glGenBuffers(1, &_vboGLPoint);
    glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERROR_DEBUG();
    
    _dirty = true;
    _dirtyGLLine = true;
    _dirtyGLPoint = true;
    
    return true;
}

void MGRDrawNode::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    if (_bufferCount > 0)
    {
        _customCommand.init(_globalZOrder, transform, flags);
        _customCommand.func = CC_CALLBACK_0(MGRDrawNode::onDraw, this, transform, flags);
        renderer->addCommand(&_customCommand);
    }
    
    if (_bufferCountGLPoint > 0)
    {
        _customCommandGLPoint.init(_globalZOrder, transform, flags);
        _customCommandGLPoint.func = CC_CALLBACK_0(MGRDrawNode::onDrawGLPoint, this, transform, flags);
        renderer->addCommand(&_customCommandGLPoint);
    }
    
    if (_bufferCountGLLine > 0)
    {
        _customCommandGLLine.init(_globalZOrder, transform, flags);
        _customCommandGLLine.func = CC_CALLBACK_0(MGRDrawNode::onDrawGLLine, this, transform, flags);
        renderer->addCommand(&_customCommandGLLine);
    }
}

void MGRDrawNode::onDraw(const Mat4 &transform, uint32_t flags)
{
    auto glProgram = getGLProgram();
    glProgram->use();
    glProgram->setUniformsForBuiltins(transform);

    GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirty)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacity, _buffer, GL_STREAM_DRAW);

        _dirty = false;
    }

    GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    // vertex
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, vertices));
    // color
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, colors));
    // texcoord
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, texCoords));

    glDrawArrays(GL_TRIANGLES, 0, _bufferCount);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _bufferCount);
    CHECK_GL_ERROR_DEBUG();
}

void MGRDrawNode::onDrawGLLine(const Mat4 &transform, uint32_t flags)
{
    auto glProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR);
    glProgram->use();
    glProgram->setUniformsForBuiltins(transform);

    GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirtyGLLine)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
        glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);
        _dirtyGLLine = false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
    GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
    // vertex
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, vertices));
    // color
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, colors));
    // texcoord
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, texCoords));

    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, _bufferCountGLLine);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _bufferCountGLLine);
    CHECK_GL_ERROR_DEBUG();
}

void MGRDrawNode::onDrawGLPoint(const Mat4 &transform, uint32_t flags)
{
    auto glProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE);
    glProgram->use();
    glProgram->setUniformsForBuiltins(transform);

    GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirtyGLPoint)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
        glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);

        _dirtyGLPoint = false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
    GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
    // vertex
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, vertices));
    // color
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, colors));
    // texcoord
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offsetof(V2F_C4B_T2F, texCoords));

    glDrawArrays(GL_POINTS, 0, _bufferCountGLPoint);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _bufferCountGLPoint);
    CHECK_GL_ERROR_DEBUG();
}

void MGRDrawNode::drawPoint(const Vec2& position, const float pointSize, const Color4F& color)
{
    ensureCapacityGLPoint(1);

    V2F_C4B_T2F* point = (V2F_C4B_T2F*)_bufferGLPoint + _bufferCountGLPoint;
    V2F_C4B_T2F a = {position, Color4B(color), Tex2F(pointSize, 0)};
    *point = a;

    _bufferCountGLPoint += 1;
    _dirtyGLPoint = true;
}

void MGRDrawNode::drawPoints(const Vec2* position, unsigned int numberOfPoints, const Color4F& color)
{
    drawPoints(position, numberOfPoints, 1.0, color);
}

void MGRDrawNode::drawPoints(const Vec2* position, unsigned int numberOfPoints, const float pointSize, const Color4F& color)
{
    ensureCapacityGLPoint(numberOfPoints);

    V2F_C4B_T2F* point = (V2F_C4B_T2F*)_bufferGLPoint + _bufferCountGLPoint;

    for (unsigned int i = 0; i < numberOfPoints; i++, point++)
    {
        V2F_C4B_T2F a = {position[i], Color4B(color), Tex2F(pointSize, 0)};
        *point = a;
    }

    _bufferCountGLPoint += numberOfPoints;
    _dirtyGLPoint = true;
}

void MGRDrawNode::drawLine(const Vec2& origin, const Vec2& destination, const Color4F& color)
{
    ensureCapacityGLLine(2);

    V2F_C4B_T2F* point = (V2F_C4B_T2F*)_bufferGLLine + _bufferCountGLLine;
    V2F_C4B_T2F a = {origin, Color4B(color), Tex2F(0.0, 0.0)};
    V2F_C4B_T2F b = {destination, Color4B(color), Tex2F(0.0, 0.0)};

    *point = a;
    *(point + 1) = b;

    _bufferCountGLLine += 2;
    _dirtyGLLine = true;
}

void MGRDrawNode::drawRect(const Vec2& origin, const Vec2& destination, const Color4F& color)
{
    drawLine(Vec2(origin.x, origin.y), Vec2(destination.x, origin.y), color);
    drawLine(Vec2(destination.x, origin.y), Vec2(destination.x, destination.y), color);
    drawLine(Vec2(destination.x, destination.y), Vec2(origin.x, destination.y), color);
    drawLine(Vec2(origin.x, destination.y), Vec2(origin.x, origin.y), color);
}

void MGRDrawNode::drawPoly(const Vec2* poli, unsigned int numberOfPoints, bool closePolygon, const Color4F& color)
{
    unsigned int vertex_count;
    if (closePolygon)
    {
        vertex_count = 2 * numberOfPoints;
        ensureCapacityGLLine(vertex_count);
    }
    else
    {
        vertex_count = 2 * (numberOfPoints - 1);
        ensureCapacityGLLine(vertex_count);
    }

    V2F_C4B_T2F* point = (V2F_C4B_T2F*)(_bufferGLLine + _bufferCountGLLine);

    for (unsigned int i = 0; i < numberOfPoints - 1; i++)
    {
        V2F_C4B_T2F a = {poli[i], Color4B(color), Tex2F(0.0, 0.0)};
        V2F_C4B_T2F b = {poli[i + 1], Color4B(color), Tex2F(0.0, 0.0)};

        *point = a;
        *(point + 1) = b;
        point += 2;
    }
    if (closePolygon)
    {
        V2F_C4B_T2F a = {poli[numberOfPoints - 1], Color4B(color), Tex2F(0.0, 0.0)};
        V2F_C4B_T2F b = {poli[0], Color4B(color), Tex2F(0.0, 0.0)};

        *point = a;
        *(point + 1) = b;
    }

    _bufferCountGLLine += vertex_count;
}

void MGRDrawNode::drawCircle(const Vec2& center, float radius, float angle, unsigned int segments, bool drawLineToCenter, float scaleX, float scaleY, const Color4F& color)
{
    const float coef = 2.0f * (float)M_PI / segments;

    Vec2* vertices = new (std::nothrow) Vec2[segments + 2];
    if (vertices == nullptr)
    {
        return;
    }

    for (unsigned int i = 0; i <= segments; i++)
    {
        float rads = i * coef;
        GLfloat j = radius * cosf(rads + angle) * scaleX + center.x;
        GLfloat k = radius * sinf(rads + angle) * scaleY + center.y;

        vertices[i].x = j;
        vertices[i].y = k;
    }

    if (drawLineToCenter)
    {
        vertices[segments + 1].x = center.x;
        vertices[segments + 1].y = center.y;
        drawPoly(vertices, segments + 2, true, color);
    }
    else
    {
        drawPoly(vertices, segments + 1, true, color);
    }

    CC_SAFE_DELETE_ARRAY(vertices);
}

void MGRDrawNode::drawCircle(const Vec2& center, float radius, float angle, unsigned int segments, bool drawLineToCenter, const Color4F& color) {
    drawCircle(center, radius, angle, segments, drawLineToCenter, 1.0f, 1.0f, color);
}

void MGRDrawNode::drawDot(const Vec2& pos, float radius, const Color4F& color)
{
    unsigned int vertex_count = 2 * 3;
    ensureCapacity(vertex_count);

    V2F_C4B_T2F a = {Vec2(pos.x - radius, pos.y - radius), Color4B(color), Tex2F(-1.0, -1.0)};
    V2F_C4B_T2F b = {Vec2(pos.x - radius, pos.y + radius), Color4B(color), Tex2F(-1.0, 1.0)};
    V2F_C4B_T2F c = {Vec2(pos.x + radius, pos.y + radius), Color4B(color), Tex2F(1.0, 1.0)};
    V2F_C4B_T2F d = {Vec2(pos.x + radius, pos.y - radius), Color4B(color), Tex2F(1.0, -1.0)};

    V2F_C4B_T2F_Triangle* triangles = (V2F_C4B_T2F_Triangle*)(_buffer + _bufferCount);
    V2F_C4B_T2F_Triangle triangle0 = {a, b, c};
    V2F_C4B_T2F_Triangle triangle1 = {a, c, d};
    triangles[0] = triangle0;
    triangles[1] = triangle1;

    _bufferCount += vertex_count;

    _dirty = true;
}

void MGRDrawNode::drawRect(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, const Color4F& color)
{
    drawLine(p1, p2, color);
    drawLine(p2, p3, color);
    drawLine(p3, p4, color);
    drawLine(p4, p1, color);
}
void MGRDrawNode::drawPolygon(const Vec2* verts, int count, const Color4F& fillColor, float borderWidth, const Color4F& borderColor)
{
    CCASSERT(count >= 0, "invalid count value");
    // –Ê“|‚È‚Ì‚Åborder‚Ì•”•ª‚ÍƒRƒsƒy
    bool outline = (borderColor.a > 0.0 && borderWidth > 0.0);
    
    auto  triangle_count = outline ? (3*count - 2) : (count - 2);

    auto vertex_count = 3 * triangle_count;
    ensureCapacity(vertex_count);

    V2F_C4B_T2F_Triangle* triangles = (V2F_C4B_T2F_Triangle*)(_buffer + _bufferCount);
    V2F_C4B_T2F_Triangle* cursor = triangles;

    for (int i = 0; i < count - 2; i++)
    {
        V2F_C4B_T2F_Triangle tmp = {
            {verts[0], Color4B(fillColor), __t(v2fzero)},
            {verts[i + 1], Color4B(fillColor), __t(v2fzero)},
            {verts[i + 2], Color4B(fillColor), __t(v2fzero)},
        };

        *cursor++ = tmp;
    }

    if(outline)
    {
        struct ExtrudeVerts {Vec2 offset, n;};
        struct ExtrudeVerts* extrude = (struct ExtrudeVerts*)malloc(sizeof(struct ExtrudeVerts)*count);
        memset(extrude, 0, sizeof(struct ExtrudeVerts)*count);
        
        for (int i = 0; i < count; i++)
        {
            Vec2 v0 = __v2f(verts[(i-1+count)%count]);
            Vec2 v1 = __v2f(verts[i]);
            Vec2 v2 = __v2f(verts[(i+1)%count]);
            
            Vec2 n1 = v2fnormalize(v2fperp(v2fsub(v1, v0)));
            Vec2 n2 = v2fnormalize(v2fperp(v2fsub(v2, v1)));
            
            Vec2 offset = v2fmult(v2fadd(n1, n2), 1.0/(v2fdot(n1, n2) + 1.0));
            struct ExtrudeVerts tmp = {offset, n2};
            extrude[i] = tmp;
        }
        
        for(int i = 0; i < count; i++)
        {
            int j = (i+1)%count;
            Vec2 v0 = __v2f(verts[i]);
            Vec2 v1 = __v2f(verts[j]);
            
            Vec2 n0 = extrude[i].n;
            
            Vec2 offset0 = extrude[i].offset;
            Vec2 offset1 = extrude[j].offset;
            
            Vec2 inner0 = v2fsub(v0, v2fmult(offset0, borderWidth));
            Vec2 inner1 = v2fsub(v1, v2fmult(offset1, borderWidth));
            Vec2 outer0 = v2fadd(v0, v2fmult(offset0, borderWidth));
            Vec2 outer1 = v2fadd(v1, v2fmult(offset1, borderWidth));
            
            V2F_C4B_T2F_Triangle tmp1 = {
                {inner0, Color4B(borderColor), __t(v2fneg(n0))},
                {inner1, Color4B(borderColor), __t(v2fneg(n0))},
                {outer1, Color4B(borderColor), __t(n0)}
            };
            *cursor++ = tmp1;
            
            V2F_C4B_T2F_Triangle tmp2 = {
                {inner0, Color4B(borderColor), __t(v2fneg(n0))},
                {outer0, Color4B(borderColor), __t(n0)},
                {outer1, Color4B(borderColor), __t(n0)}
            };
            *cursor++ = tmp2;
        }
        
        free(extrude);
    }

    _bufferCount += vertex_count;
    _dirty = true;
}

void MGRDrawNode::drawSolidRect(const Vec2& origin, const Vec2& destination, const Color4F& color)
{
    Vec2 vertices[] = {
        origin,
        Vec2(destination.x, origin.y),
        destination,
        Vec2(origin.x, destination.y),
    };

    drawSolidPoly(vertices, 4, color);
}

void MGRDrawNode::drawSolidPoly(const Vec2* poli, unsigned int numberOfPoints, const Color4F& color)
{
    drawPolygon(poli, numberOfPoints, color, 0.0, Color4F(0.0, 0.0, 0.0, 0.0));
}

void MGRDrawNode::drawSolidCircle(const Vec2& center, float radius, float angle, unsigned int segments, float scaleX, float scaleY, const Color4F& color)
{
    const float coef = 2.0f * (float)M_PI / segments;

    Vec2* vertices = new (std::nothrow) Vec2[segments + 2];
    if (vertices == nullptr)
    {
        return;
    }

    for (unsigned int i = 0; i <= segments; i++)
    {
        float rads = i * coef;
        GLfloat j = radius * cosf(rads + angle) * scaleX + center.x;
        GLfloat k = radius * sinf(rads + angle) * scaleY + center.y;

        vertices[i].x = j;
        vertices[i].y = k;
    }

    drawSolidPoly(vertices, segments, color);

    CC_SAFE_DELETE_ARRAY(vertices);
}

void MGRDrawNode::drawSolidCircle(const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color) {
    drawSolidCircle(center, radius, angle, segments, 1.0f, 1.0f, color);
}

void MGRDrawNode::drawTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Color4F& color)
{
    unsigned int vertex_count = 3;
    ensureCapacity(vertex_count);

    Color4B col = Color4B(color);
    V2F_C4B_T2F a = {p1, col, Tex2F(0.0, 0.0)};
    V2F_C4B_T2F b = {p2, col, Tex2F(0.0, 0.0)};
    V2F_C4B_T2F c = {p3, col, Tex2F(0.0, 0.0)};

    V2F_C4B_T2F_Triangle* triangles = (V2F_C4B_T2F_Triangle*)(_buffer + _bufferCount);
    V2F_C4B_T2F_Triangle triangle = {a, b, c};
    triangles[0] = triangle;

    _bufferCount += vertex_count;
    _dirty = true;
}

void MGRDrawNode::clear()
{
    _bufferCount = 0;
    _dirty = true;
    _bufferCountGLLine = 0;
    _dirtyGLLine = true;
    _bufferCountGLPoint = 0;
    _dirtyGLPoint = true;
}

NS_CC_END

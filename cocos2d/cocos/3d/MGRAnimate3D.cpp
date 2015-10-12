/****************************************************************************
Copyright (c) 2014 Chukong Technologies Inc.

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

#include "3d/MGRAnimate3D.h"
#include "3d/CCAnimate3D.h" // Animate3DQualityのため
#include "3d/MGRSprite3D.h"
#include "3d/CCSkeleton3D.h"
#include "platform/CCFileUtils.h"
#include "base/CCConfiguration.h"

NS_CC_BEGIN

std::unordered_map<Node*, MGRAnimate3D*> MGRAnimate3D::s_fadeInAnimates;
std::unordered_map<Node*, MGRAnimate3D*> MGRAnimate3D::s_fadeOutAnimates;
std::unordered_map<Node*, MGRAnimate3D*> MGRAnimate3D::s_runningAnimates;
float      MGRAnimate3D::_transTime = 0.1f;

//create MGRAnimate3D using Animation.
MGRAnimate3D* MGRAnimate3D::create(Animation3D* animation)
{
    auto animate = new (std::nothrow) MGRAnimate3D();
    animate->init(animation);
    animate->autorelease();

    return animate;
}

MGRAnimate3D* MGRAnimate3D::create(Animation3D* animation, float fromTime, float duration)
{
    auto animate = new (std::nothrow) MGRAnimate3D();
    animate->init(animation, fromTime, duration);
    animate->autorelease();

    return  animate;
}

MGRAnimate3D* MGRAnimate3D::createWithFrames(Animation3D* animation, int startFrame, int endFrame, float frameRate)
{
    auto animate = new (std::nothrow) MGRAnimate3D();
    animate->initWithFrames(animation, startFrame, endFrame, frameRate);
    animate->autorelease();

    return  animate;
}

bool MGRAnimate3D::init(Animation3D* animation)
{
    _animation = animation;
    animation->retain();
    setDuration(animation->getDuration());
    setOriginInterval(animation->getDuration());
    setQuality(Configuration::getInstance()->getAnimate3DQuality());
    return true;
}

bool MGRAnimate3D::init(Animation3D* animation, float fromTime, float duration)
{
    float fullDuration = animation->getDuration();
    if (duration > fullDuration - fromTime)
        duration = fullDuration - fromTime;

    _start = fromTime / fullDuration;
    _last = duration / fullDuration;
    setDuration(duration);
    setOriginInterval(duration);
    _animation = animation;
    animation->retain();
    setQuality(Configuration::getInstance()->getAnimate3DQuality());
    return true;
}

bool MGRAnimate3D::initWithFrames(Animation3D* animation, int startFrame, int endFrame, float frameRate)
{
    float perFrameTime = 1.f / frameRate;
    float fromTime = startFrame * perFrameTime;
    float duration = (endFrame - startFrame) * perFrameTime;
    init(animation, fromTime, duration);
    return true;
}

/** returns a clone of action */
MGRAnimate3D* MGRAnimate3D::clone() const
{
    auto animate = const_cast<MGRAnimate3D*>(this);
    auto copy = MGRAnimate3D::create(animate->_animation);

    copy->_absSpeed = _absSpeed;
    copy->_weight = _weight;
    copy->_elapsed = _elapsed;
    copy->_start = _start;
    copy->_last = _last;
    copy->_playReverse = _playReverse;
    copy->setDuration(animate->getDuration());
    copy->setOriginInterval(animate->getOriginInterval());
    return copy;
}

/** returns a new action that performs the exactly the reverse action */
MGRAnimate3D* MGRAnimate3D::reverse() const
{
    auto animate = clone();
    animate->_playReverse = !animate->_playReverse;
    return animate;
}

// Animate3Dクラスと定義重複するので名前を変えている
Node* findChildByNameRecursivelyMGR(Node* node, const std::string &childName)
{
    const std::string& name = node->getName();
    if (name == childName)
        return node;

    const Vector<Node*>& children = node->getChildren();
    for (const auto& child : children)
    {
        Node* findNode = findChildByNameRecursivelyMGR(child, childName);
        if (findNode)
            return findNode;
    }
    return nullptr;
}

//! called before the action start. It will also set the target.
void MGRAnimate3D::startWithTarget(Node *target)
{
    bool needReMap = (_target != target);
    ActionInterval::startWithTarget(target);

    if (needReMap)
    {
        _boneCurves.clear();
        _nodeCurves.clear();

        bool hasCurve = false;
        MGRSprite3D* sprite = dynamic_cast<MGRSprite3D*>(target);

        if (sprite)
        {
            if (_animation)
            {
                const std::unordered_map<std::string, Animation3D::Curve*>& boneCurves = _animation->getBoneCurves();
                for (const auto& iter : boneCurves)
                {
                    const std::string& boneName = iter.first;
                    auto skin = sprite->getSkeleton();
                    if (skin)
                    {
                        auto bone = skin->getBoneByName(boneName);
                        if (bone)
                        {
                            auto curve = _animation->getBoneCurveByName(boneName);
                            _boneCurves[bone] = curve;
                            hasCurve = true;
                        }
                        else
                        {
                            Node* node = nullptr;
                            if (target->getName() == boneName)
                                node = target;
                            else
                                node = findChildByNameRecursivelyMGR(target, boneName);

                            if (node)
                            {
                                auto curve = _animation->getBoneCurveByName(boneName);
                                if (curve)
                                {
                                    _nodeCurves[node] = curve;
                                    hasCurve = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            const std::unordered_map<std::string, Animation3D::Curve*>& boneCurves = _animation->getBoneCurves();
            for (const auto& iter : boneCurves)
            {
                const std::string& boneName = iter.first;
                Node* node = nullptr;
                if (target->getName() == boneName)
                    node = target;
                else
                    node = findChildByNameRecursivelyMGR(target, boneName);

                if (node)
                {
                    auto curve = _animation->getBoneCurveByName(boneName);
                    if (curve)
                    {
                        _nodeCurves[node] = curve;
                        hasCurve = true;
                    }
                }

            }
        }

        if (!hasCurve)
        {
            CCLOG("warning: no animation finde for the skeleton");
        }
    }

    auto runningAction = s_runningAnimates.find(target);
    if (runningAction != s_runningAnimates.end())
    {
        //make the running action fade out
        auto action = (*runningAction).second;
        if (action != this)
        {
            if (_transTime < 0.001f)
            {
                s_runningAnimates[target] = this;
                _state = MGRAnimate3D::MGRAnimate3DState::Running;
                _weight = 1.0f;
            }
            else
            {
                s_fadeOutAnimates[target] = action;
                action->_state = MGRAnimate3D::MGRAnimate3DState::FadeOut;
                action->_accTransTime = 0.0f;
                action->_weight = 1.0f;
                action->_lastTime = 0.f;

                s_fadeInAnimates[target] = this;
                _accTransTime = 0.0f;
                _state = MGRAnimate3D::MGRAnimate3DState::FadeIn;
                _weight = 0.f;
                _lastTime = 0.f;
            }
        }
    }
    else
    {
        auto it = s_fadeInAnimates.find(target);
        if (it != s_fadeInAnimates.end())
        {
            s_fadeInAnimates.erase(it);
        }
        s_runningAnimates[target] = this;
        _state = MGRAnimate3D::MGRAnimate3DState::Running;
        _weight = 1.0f;
    }
}

void MGRAnimate3D::stop()
{
    removeFromMap();

    ActionInterval::stop();
}

//! called every frame with it's delta time. DON'T override unless you know what you are doing.
void MGRAnimate3D::step(float dt)
{
    ActionInterval::step(dt);
}

void MGRAnimate3D::update(float t)
{
    if (_target)
    {
        if (_state == MGRAnimate3D::MGRAnimate3DState::FadeIn && _lastTime > 0.f)
        {
            _accTransTime += (t - _lastTime) * getDuration();

            _weight = _accTransTime / _transTime;
            if (_weight >= 1.0f)
            {
                _accTransTime = _transTime;
                _weight = 1.0f;
                _state = MGRAnimate3D::MGRAnimate3DState::Running;
                s_fadeInAnimates.erase(_target);
                s_runningAnimates[_target] = this;
            }
        }
        else if (_state == MGRAnimate3D::MGRAnimate3DState::FadeOut && _lastTime > 0.f)
        {
            _accTransTime += (t - _lastTime) * getDuration();

            _weight = 1 - _accTransTime / _transTime;
            if (_weight <= 0.0f)
            {
                _accTransTime = _transTime;
                _weight = 0.0f;

                s_fadeOutAnimates.erase(_target);
            }
        }
        _lastTime = t;

        if (_quality != Animate3DQuality::QUALITY_NONE)
        {
            if (_weight > 0.0f)
            {
                float transDst[3], rotDst[4], scaleDst[3];
                float* trans = nullptr, *rot = nullptr, *scale = nullptr;
                if (_playReverse)
                    t = 1 - t;

                t = _start + t * _last;

                for (const auto& it : _boneCurves) {
                    auto bone = it.first;
                    auto curve = it.second;
                    if (curve->translateCurve)
                    {
                        curve->translateCurve->evaluate(t, transDst, _translateEvaluate);
                        trans = &transDst[0];
                    }
                    if (curve->rotCurve)
                    {
                        curve->rotCurve->evaluate(t, rotDst, _roteEvaluate);
                        rot = &rotDst[0];
                    }
                    if (curve->scaleCurve)
                    {
                        curve->scaleCurve->evaluate(t, scaleDst, _scaleEvaluate);
                        scale = &scaleDst[0];
                    }
                    bone->setAnimationValue(trans, rot, scale, this, _weight);
                }

                for (const auto& it : _nodeCurves)
                {
                    auto node = it.first;
                    auto curve = it.second;
                    Mat4 transform;
                    if (curve->translateCurve)
                    {
                        curve->translateCurve->evaluate(t, transDst, _translateEvaluate);
                        transform.translate(transDst[0], transDst[1], transDst[2]);
                    }
                    if (curve->rotCurve)
                    {
                        curve->rotCurve->evaluate(t, rotDst, _roteEvaluate);
                        Quaternion qua(rotDst[0], rotDst[1], rotDst[2], rotDst[3]);
                        transform.rotate(qua);
                    }
                    if (curve->scaleCurve)
                    {
                        curve->scaleCurve->evaluate(t, scaleDst, _scaleEvaluate);
                        transform.scale(scaleDst[0], scaleDst[1], scaleDst[2]);
                    }
                    node->setAdditionalTransform(&transform);
                }
            }
        }
    }
}

float MGRAnimate3D::getSpeed() const
{
    return _playReverse ? -_absSpeed : _absSpeed;
}
void MGRAnimate3D::setSpeed(float speed)
{
    _absSpeed = fabsf(speed);
    _playReverse = speed < 0;
    _duration = _originInterval / _absSpeed;
}

void MGRAnimate3D::setWeight(float weight)
{
    CCASSERT(weight >= 0.0f, "invalid weight");
    _weight = fabsf(weight);
}

void MGRAnimate3D::setOriginInterval(float interval)
{
    _originInterval = interval;
}

void MGRAnimate3D::setQuality(Animate3DQuality quality)
{
    if (quality == Animate3DQuality::QUALITY_HIGH)
    {
        _translateEvaluate = EvaluateType::INT_LINEAR;
        _roteEvaluate = EvaluateType::INT_QUAT_SLERP;
        _scaleEvaluate = EvaluateType::INT_LINEAR;
    }
    else if (quality == Animate3DQuality::QUALITY_LOW)
    {
        _translateEvaluate = EvaluateType::INT_NEAR;
        _roteEvaluate = EvaluateType::INT_NEAR;
        _scaleEvaluate = EvaluateType::INT_NEAR;
    }
    _quality = quality;
}

Animate3DQuality MGRAnimate3D::getQuality() const
{
    return _quality;
}

MGRAnimate3D::MGRAnimate3D()
    : _state(MGRAnimate3D::MGRAnimate3DState::Running)
    , _animation(nullptr)
    , _absSpeed(1.f)
    , _weight(1.f)
    , _start(0.f)
    , _last(1.f)
    , _playReverse(false)
    , _accTransTime(0.0f)
    , _lastTime(0.0f)
    , _originInterval(0.0f)
{
    setQuality(Animate3DQuality::QUALITY_HIGH);
}
MGRAnimate3D::~MGRAnimate3D()
{
    removeFromMap();

    CC_SAFE_RELEASE(_animation);
}

void MGRAnimate3D::removeFromMap()
{
    //remove this action from map
    if (_target)
    {
        MGRSprite3D* sprite = static_cast<MGRSprite3D*>(_target);
        s_fadeInAnimates.erase(sprite);
        s_fadeOutAnimates.erase(sprite);
        s_runningAnimates.erase(sprite);
    }
}

NS_CC_END

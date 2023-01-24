#pragma once


namespace ShadowEngine::Entities {

#define Entity_Base(Type, BaseType)		\
public:											\
Type(ShadowEngine::Entities::Scene* scene) : BaseType(scene) {}	\
Type() : BaseType() {}	\
ShadowEngine::Entities::Entity* Type::Create(ShadowEngine::Entities::Scene* scene) override {			\
Type* res = new Type(scene);							\
return res;									    \
}												\
private:

#define Entity_Base_NoCtor(Type, BaseType)		\
public:											\
ShadowEngine::Entities::Entity* Create(ShadowEngine::Entities::Scene* scene) {			\
Type* res = new Type(scene);							\
return res;									    \
}												\
private:

}
#ifndef UMBRA_MODULE_H
#define UMBRA_MODULE_H

#include "../../../shadow-reflection/src/SHObject.h"

namespace ShadowEngine {

    /// <summary>
    /// ShadowModules are the base of the engine. They add core abilities.
    /// </summary>
    class Module : public SHObject
    {
        SHObject_Base(Module)

    public:

        /// <summary>
        /// Pre Init is called when the module is added to the engine
        /// </summary>
        virtual void PreInit() = 0;


        /// <summary>
        /// Init is called after all the modules are added
        /// </summary>
        virtual void Init() = 0;

        /// <summary>
        /// update is called each frame
        /// </summary>
        virtual void Update() = 0;

        virtual void Render() = 0;

        virtual void LateRender() = 0;

        virtual void AfterFrameEnd() {};

        /// <summary>
        /// Returns the name of the module
        /// </summary>
        /// <returns></returns>
        virtual std::string GetName() = 0;


        Module();
        virtual ~Module();
    };

} // ShadowEngine

#endif //UMBRA_MODULE_H

#include <shader/compiler/ShaderCompiler.h>

#ifdef WIN32
#include <wrl/client.h>
#define CComPtr Microsoft::WRL::ComPtr
#endif

#ifdef __linux__
#define __RPC_FAR
#include <dxc/WinAdapter.h>
#endif
#ifndef _Maybenull_
#define _Maybenull_
#endif
#include <dylib.hpp>
#include <unordered_set>
#include <dxc/dxcapi.h>
#include <shadow/assets/fs/file.h>
#include <shadow/core/Libraries.h>
#include <shadow/platform/Common.h>
#include <shadow/util/string-helpers.h>
#include <spdlog/spdlog.h>

namespace rx::shader {
    struct DXCInternal {
        DxcCreateInstanceProc CreateInstance = nullptr;
        DXCInternal(const std::string& modifier = "") {
            const std::string library = ifsystem("./libdxcompiler" + modifier + ".so", "dxcompiler" + modifier + ".dll", "./libdxcompiler" + modifier + ".dylib");
            SH::Path path(library);
            dylib* lib = (dylib*)ShadowEngine::Library::load(path);

            if (lib != nullptr) {
                CreateInstance = (DxcCreateInstanceProc) lib->get_symbol("DxcCreateInstance");
                if (CreateInstance != nullptr) {
                    CComPtr<IDxcCompiler3> compiler;
                    HRESULT hr = CreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
                    assert(SUCCEEDED(hr));

                    CComPtr<IDxcVersionInfo> version;
                    hr = compiler->QueryInterface(IID_PPV_ARGS(&version));
                    assert(SUCCEEDED(hr));

                    uint32_t major = 0, minor = 0;
                    hr = version->GetVersion(&major, &minor);
                    assert(SUCCEEDED(hr));

                    spdlog::info("Shader Compiler loaded " + library + " version " + std::to_string(major) + "." + std::to_string(minor));
                }
            } else {
                spdlog::error("Shader Compiler could not load " + library + ".");
#ifdef __linux__
                spdlog::error(dlerror());
#endif
            }
        }
    };

    struct DXCIncludeHandler : IDxcIncludeHandler {
        const CompilerInput* input = nullptr;
        CompilerOutput* output = nullptr;
        CComPtr<IDxcIncludeHandler> handler;

        HRESULT STDMETHODCALLTYPE LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override {
            HRESULT hr = handler->LoadSource(pFilename, ppIncludeSource);
            if (SUCCEEDED(hr))
                SH::Util::Str::StringConvert(pFilename, output->deps.emplace_back());
            return hr;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override {
            return handler->QueryInterface(riid, ppvObject);
        }

        ULONG STDMETHODCALLTYPE AddRef(void) override {
            return 0;
        }

        ULONG STDMETHODCALLTYPE Release(void) override {
            return 0;
        }
    };

    inline DXCInternal& DXCompiler() {
        static DXCInternal internal;
        return internal;
    }

    void CompileWithDXC(const CompilerInput& in, CompilerOutput& out) {
        DXCInternal& internal = DXCompiler();
        if (internal.CreateInstance == nullptr) return;

        CComPtr<IDxcUtils> utils;
        CComPtr<IDxcCompiler3> compiler;

        HRESULT hr = internal.CreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        assert(SUCCEEDED(hr));
        hr = internal.CreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        assert(SUCCEEDED(hr));

        if (compiler == nullptr) return;

        std::vector<uint8_t> sourceData;
        SH::FileInput file;
        if (!file.open(in.sourceName)) return;
        sourceData.resize(file.size());
        file.read(sourceData.data(), file.size());
        file.close();

        std::vector<std::wstring> args = { L"-Wno-conversion" };
        if (has_flag(in.flags, Flags::DISABLE_OPTIMIZATION))
            args.push_back(L"-0d");

        switch (in.format) {
            case ShaderFormat::HLSL6:
                args.push_back(L"-rootsig-define");
                args.push_back(L"UMBRA_RX_ROOT");
                break;
            case ShaderFormat::SPIRV:
                args.push_back(L"-spirv");
                args.push_back(L"-fspv-target-env=vulkan1.2");
                args.push_back(L"-fvk-use-dx-layout");
                args.push_back(L"-fvk-use-fx-position-w");
                args.push_back(L"-fvk-t-shift"); args.push_back(L"1000"); args.push_back(L"0");
                args.push_back(L"-fvk-u-shift"); args.push_back(L"2000"); args.push_back(L"0");
                args.push_back(L"-fvk-s-shift"); args.push_back(L"3000"); args.push_back(L"0");
                break;
        }

        ShaderModel minShader = in.model;

        args.push_back(L"-T");

        std::stringstream shaderVersion;

        switch (in.stage) {
            case ShaderStage::AS: shaderVersion << "as"; break;
            case ShaderStage::MS: shaderVersion << "ms"; break;
            case ShaderStage::VS: shaderVersion << "vs"; break;
            case ShaderStage::HS: shaderVersion << "hs"; break;
            case ShaderStage::DS: shaderVersion << "ds"; break;
            case ShaderStage::GS: shaderVersion << "gs"; break;
            case ShaderStage::FS: shaderVersion << "ps"; break;
            case ShaderStage::CS: shaderVersion << "cs"; break;
            case ShaderStage::LIB: shaderVersion << "lib"; break;
        }

        shaderVersion << "_";

        switch (minShader) {
            case ShaderModel::SM65: shaderVersion << "6_5"; break;
            case ShaderModel::SM66: shaderVersion << "6_6"; break;
            case ShaderModel::SM67: shaderVersion << "6_7"; break;

            case ShaderModel::SM64: shaderVersion << (in.stage == ShaderStage::LIB || in.stage == ShaderStage::AS || in.stage == ShaderStage::MS) ? "6_5" : "6_4"; break;
            case ShaderModel::SM63: shaderVersion << (in.stage == ShaderStage::LIB || in.stage == ShaderStage::AS || in.stage == ShaderStage::MS) ? "6_5" : "6_3"; break;
            case ShaderModel::SM62: shaderVersion << (in.stage == ShaderStage::LIB || in.stage == ShaderStage::AS || in.stage == ShaderStage::MS) ? "6_5" : "6_2"; break;
            case ShaderModel::SM61: shaderVersion << (in.stage == ShaderStage::LIB || in.stage == ShaderStage::AS || in.stage == ShaderStage::MS) ? "6_5" : "6_1"; break;
            case ShaderModel::SM60: shaderVersion << (in.stage == ShaderStage::LIB || in.stage == ShaderStage::AS || in.stage == ShaderStage::MS) ? "6_5" : "6_0"; break;
        }

        for (auto& x : in.defines) {
            args.push_back(L"-D");
            SH::Util::Str::StringConvert(x, args.emplace_back());
        }

        for (auto& x : in.includes) {
            args.push_back(L"-I");
            SH::Util::Str::StringConvert(x, args.emplace_back());
        }

        args.push_back(L"-E");
        SH::Util::Str::StringConvert(in.entryPoint, args.emplace_back());

        SH::Util::Str::StringConvert(SH::Path::getFilename((std::string&) in.sourceName), args.emplace_back());

        DxcBuffer source = { sourceData.data(), sourceData.size(), DXC_CP_ACP };

        DXCIncludeHandler handler;
        handler.input = &in;
        handler.output = &out;

        hr = utils->CreateDefaultIncludeHandler(&handler.handler);
        assert(SUCCEEDED(hr));

        std::vector<const wchar_t*> rawArgs;
        rawArgs.reserve(args.size());
        for (auto& x : args)
            rawArgs.push_back(x.c_str());

        CComPtr<IDxcResult> result;
        hr = compiler->Compile(&source, rawArgs.data(), (uint32_t) args.size(), &handler, IID_PPV_ARGS(&result));
        assert(SUCCEEDED(hr));

        CComPtr<IDxcBlobUtf8> errors = nullptr;
        hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        assert(SUCCEEDED(hr));
        if (errors != nullptr && errors->GetStringLength() != 0)
            out.error = errors->GetStringPointer();

        HRESULT status;
        hr = result->GetStatus(&status);
        assert(SUCCEEDED(hr));
        if (FAILED(status)) return;

        CComPtr<IDxcBlob> shader = nullptr;
        hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);
        assert(SUCCEEDED(hr));
        if (shader != nullptr) {
            out.deps.push_back(in.sourceName);
            out.data = (const uint8_t*)shader->GetBufferPointer();
            out.dataLen = shader->GetBufferSize();

            auto internal = std::make_shared<CComPtr<IDxcBlob>>();
            *internal = shader;
            out.internal = internal;
        }

        if (in.format == ShaderFormat::HLSL6) {
            CComPtr<IDxcBlob> hash = nullptr;
            hr = result->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&hash), nullptr);
            assert(SUCCEEDED(hr));
            if (hash != nullptr) {
                DxcShaderHash* hashBuf = (DxcShaderHash*) hash->GetBufferPointer();
                for (int i = 0; i < _countof(hashBuf->HashDigest); i++)
                    out.hash.push_back(hashBuf->HashDigest[i]);
            }
        }
    }

    void Compile(const CompilerInput& input, CompilerOutput& out) {
        out = CompilerOutput();

        switch (input.format) {
            default: break;
            case ShaderFormat::HLSL6:
            case ShaderFormat::SPIRV:
                CompileWithDXC(input, out);
                break;
        }
    }

    constexpr const char* shaderMeta = "rxshadermeta";
    bool SaveData(const std::string& filename, const CompilerOutput& out) {
        // TODO: Save dependency data

        SH::FileOutput shaderFile;
        if (!shaderFile.open(filename)) return false;
        shaderFile.write(out.data, out.dataLen);
        shaderFile.close();
        return true;
    }

    bool IsMetadataOutdated(const std::string& filename) {

        if (!std::filesystem::exists(std::filesystem::absolute(filename))) return true;
        // TODO: Load dependency data

        return false;
    }


    std::mutex lock;
    std::unordered_set<std::string> registered;

    void RegisterShader(const std::string& filename) {
        std::scoped_lock locker(lock);
        registered.insert(filename);
    }

    size_t GetRegisteredShaders() {
        std::scoped_lock locker(lock);
        return registered.size();
    }

    bool CheckRegisteredShadersOutdated() {
        std::scoped_lock locker(lock);
        for (auto& x : registered) {
            if (IsMetadataOutdated(x)) return true;
        }

        return false;
    }





}

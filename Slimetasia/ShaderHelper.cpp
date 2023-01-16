#include "ShaderHelper.h"

#include <Windows.h>
#include <atlcomcli.h>
#include <dxc/dxcapi.h>

#include <filesystem>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Logger.h"

namespace ShaderHelper
{

    std::vector<char> CompileToSpirv(const std::filesystem::path& filePath)
    {
        HRESULT hres {};

        CComPtr<IDxcLibrary> library {};
        hres = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
        ASSERT(!FAILED(hres));

        CComPtr<IDxcCompiler3> compiler {};
        hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        ASSERT(!FAILED(hres));

        CComPtr<IDxcUtils> utils {};
        hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        ASSERT(!FAILED(hres));

        uint32_t codePage = DXC_CP_ACP;
        CComPtr<IDxcBlobEncoding> sourceBlob {};
        hres = utils->LoadFile(filePath.c_str(), &codePage, &sourceBlob);
        ASSERT(!FAILED(hres));

        LPCWSTR targetProfile {};
        const std::wstring& filePathString = filePath.wstring();
        const std::size_t idx = filePathString.rfind('.');
        if (idx != std::string::npos)
        {
            const std::wstring& extension = filePathString.substr(idx + 1);
            if (extension == L"vert")
            {
                targetProfile = L"vs_6_3";
            }
            if (extension == L"frag")
            {
                targetProfile = L"ps_6_3";
            }
            if (extension == L"geom")
            {
                targetProfile = L"gs_6_3";
            }
        }

        std::vector<LPCWSTR> args = {
            filePath.c_str(), L"-E", L"main", L"-T", targetProfile, L"-spirv",
        };

        DxcBuffer buffer {};
        buffer.Encoding = DXC_CP_ACP;
        buffer.Ptr = sourceBlob->GetBufferPointer();
        buffer.Size = sourceBlob->GetBufferSize();

        CComPtr<IDxcResult> result {};
        hres = compiler->Compile(&buffer, args.data(), static_cast<uint32_t>(args.size()), nullptr, IID_PPV_ARGS(&result));
        ASSERT(!FAILED(hres));

        result->GetStatus(&hres);

        std::vector<char> spirvCode {};

        if (FAILED(hres) && result)
        {
            CComPtr<IDxcBlobEncoding> errorBlob {};
            hres = result->GetErrorBuffer(&errorBlob);
            if (SUCCEEDED(hres) && errorBlob)
            {
                std::cerr << "Shader compilation failed:\n\n" << static_cast<const char*>(errorBlob->GetBufferPointer());
            }
        }
        else
        {
            CComPtr<IDxcBlob> code {};
            result->GetResult(&code);
            const char* codeBufferPointer = static_cast<char*>(code->GetBufferPointer());
            spirvCode = std::vector<char>(codeBufferPointer, codeBufferPointer + code->GetBufferSize());
        }

        return spirvCode;
    }
}  // namespace ShaderHelper
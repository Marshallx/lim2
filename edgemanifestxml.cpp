
/*

#include <AppxPackaging.h>
#include <shtypes.h>
#include <atlbase.h>

void load(std::filesystem::path const & file)
{
    auto const & xmlFile = file.wstring();
    auto && pStream = CComPtr<IStream>();

    auto hr = SHCreateStreamOnFileEx(
        xmlFile.c_str(),
        STGM_FAILIFTHERE,
        0,
        FALSE,
        NULL,
        &pStream
    );

    MX_THROW_ON_FALSE(SUCCEEDED(hr), ErrorType::HResult, hr);

    auto && pAppxFactory = CComPtr<IAppxFactory>();
    hr = pAppxFactory.CoCreateInstance(__uuidof(AppxFactory));
    MX_THROW_ON_FALSE(SUCCEEDED(hr), ErrorType::HResult, hr);

    auto && pAppxManifestReader = CComPtr<IAppxManifestReader>();
    hr = pAppxFactory->CreateManifestReader(
        pStream,
        &pAppxManifestReader
    );
    MX_THROW_ON_FALSE(SUCCEEDED(hr), ErrorType::HResult, hr);

    auto && pAppxManifestPackageId = CComPtr<IAppxManifestPackageId>();
    hr = pAppxManifestReader->GetPackageId(&pAppxManifestPackageId);
    MX_THROW_ON_FALSE(SUCCEEDED(hr), ErrorType::HResult, hr);

    auto appxVersion = UINT64{ 0 };
    hr = pAppxManifestPackageId->GetVersion(&appxVersion);
    MX_THROW_ON_FALSE(SUCCEEDED(hr), ErrorType::HResult, hr);
}
*/
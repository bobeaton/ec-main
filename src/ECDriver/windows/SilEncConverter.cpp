// 12-Jun-2013 JDK  Return error code if exception is caught during conversion.

#include "stdafx.h"
#include "silencconverter.h"

#define strCaption  _T("SilEncConverters Error")

CSilEncConverter::CSilEncConverter(void)
: DirectionForward(TRUE)
{
}

CSilEncConverter::~CSilEncConverter(void)
{
}

bool CSilEncConverter::IsInputLegacy() const
{
    if( !!(*this) )
    {
        // then we're all set: call Convert
        ECInterfaces::ConvType eConvType;
        if (IsSEC30())
            ProcessHResult(m_aEC30->get_ConversionType(&eConvType), m_aEC30, __uuidof(ECInterfaces::IEncConverter));
        else if (IsSEC22())
            ProcessHResult(m_aEC22->get_ConversionType((SilEncConverters22::ConvType*)&eConvType), m_aEC22, __uuidof(SilEncConverters22::IEncConverter));

        if (DirectionForward)
            return ((eConvType == ECInterfaces::ConvType_Legacy_to_from_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_from_Unicode)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_Unicode));
        else
            return ((eConvType == ECInterfaces::ConvType_Unicode_to_from_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_from_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Unicode_to_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_Legacy));
    }

    return false;
}

bool CSilEncConverter::IsOutputLegacy() const
{
    if( !!(*this) )
    {
        // then we're all set: call Convert
        ECInterfaces::ConvType eConvType;
        if (IsSEC30())
            ProcessHResult(m_aEC30->get_ConversionType(&eConvType), m_aEC30, __uuidof(ECInterfaces::IEncConverter));
        else if (IsSEC22())
            ProcessHResult(m_aEC22->get_ConversionType((SilEncConverters22::ConvType*)&eConvType), m_aEC22, __uuidof(SilEncConverters22::IEncConverter));

        if (DirectionForward)
            return ((eConvType == ECInterfaces::ConvType_Unicode_to_from_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_from_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Unicode_to_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_Legacy));
        else
            return ((eConvType == ECInterfaces::ConvType_Legacy_to_from_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_from_Unicode)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_Legacy)
                ||  (eConvType == ECInterfaces::ConvType_Legacy_to_Unicode));
    }

    return false;
}

int CSilEncConverter::CodePageInput() const
{
    long nCodePage = CP_ACP;
    if( !!(*this) )
    {
        // then we're all set: call Convert
        if (IsSEC30())
            ProcessHResult(m_aEC30->get_CodePageInput(&nCodePage), m_aEC30, __uuidof(ECInterfaces::IEncConverter));
        else if (IsSEC22())
            ProcessHResult(m_aEC22->get_CodePageInput(&nCodePage), m_aEC22, __uuidof(SilEncConverters22::IEncConverter));
    }

    return (int)nCodePage;
}

int CSilEncConverter::CodePageOutput() const
{
    long nCodePage = CP_ACP;
    if( !!(*this) )
    {
        // then we're all set: call Convert
        if (IsSEC30())
            ProcessHResult(m_aEC30->get_CodePageOutput(&nCodePage), m_aEC30, __uuidof(ECInterfaces::IEncConverter));
        else if (IsSEC22())
            ProcessHResult(m_aEC22->get_CodePageOutput(&nCodePage), m_aEC22, __uuidof(SilEncConverters22::IEncConverter));
    }

    return (int)nCodePage;
}

HRESULT CSilEncConverter::Convert(const CStringW& strInput, CStringW& strOutput)
{
    // if it's connected (now) AND if the string isn't empty...
	HRESULT hr = S_OK;
    CComBSTR bstrOutput;
    if( !!(*this) && !strInput.IsEmpty() )
    {
        // then we're all set: call Convert
        if (IsSEC30())
        {
            hr = ProcessHResult(m_aEC30->Convert(CComBSTR(strInput), &bstrOutput), m_aEC30, __uuidof(ECInterfaces::IEncConverter));
        }
        else if (IsSEC22())
        {
            hr = ProcessHResult(m_aEC22->Convert(CComBSTR(strInput), &bstrOutput), m_aEC22, __uuidof(SilEncConverters22::IEncConverter));
        }
    }

    strOutput = CStringW(bstrOutput);
    return hr;
}

CStringW CSilEncConverter::Description()
{
    CComBSTR str;
    if (IsSEC30())
        m_aEC30->get_ToString(&str);
    else if (IsSEC22())
        m_aEC22->get_ToString(&str);

    return CStringW(str);
}

void CSilEncConverter::Detach()
{
    if (IsSEC30())
        m_aEC30.Detach();
    if (IsSEC22())
        m_aEC22.Detach();
}

HRESULT CSilEncConverter::Initialize(const CStringW& strFriendlyName, BOOL bDirectionForward, int eNormalizeFlag)
{
    Detach();

    ConverterName = strFriendlyName;
    DirectionForward = bDirectionForward;
    NormalizeOutput = eNormalizeFlag;

    // first see if we can access the new v3.0 interface
    HRESULT hrRet = S_OK;
    IEC30s  pEC30s;
    CStringW strProgId;
    CRegKey keyRegEC;
    if (keyRegEC.Open(HKEY_CLASSES_ROOT, _T("SilEncConverters40.EncConverters"), KEY_READ) == ERROR_SUCCESS)
        strProgId = L"SilEncConverters40.EncConverters";
    else if (keyRegEC.Open(HKEY_CLASSES_ROOT, _T("SilEncConverters31.EncConverters"), KEY_READ) == ERROR_SUCCESS)
        strProgId = L"SilEncConverters31.EncConverters";
    else
        strProgId = L"SilEncConverters30.EncConverters";

    pEC30s.CoCreateInstance(strProgId);
    if( !!pEC30s )
    {
        CComVariant varName(ConverterName);
        pEC30s->get_Item(varName, &m_aEC30);
        if( !!m_aEC30 )
        {
            // initialize the other run-time parameters needed for the Convert call
            m_aEC30->put_DirectionForward((DirectionForward) ? VARIANT_TRUE : VARIANT_FALSE);
            m_aEC30->put_NormalizeOutput((ECInterfaces::NormalizeFlags)NormalizeOutput);
        }
        else
        {
            // must no longer be in the repository!
            hrRet = /*NameNotFound*/ -7;
        }
    }
    else
    {
        // otherwise, try the older 22 interface
        IEC22s  pEC22s;
        pEC22s.CoCreateInstance(L"SilEncConverters22.EncConverters");
        if (!!pEC22s)
        {
            CComVariant varName(ConverterName);
            pEC22s->get_Item(varName, &m_aEC22);
            if( !!m_aEC22 )
            {
                // initialize the other run-time parameters needed for the Convert call
                m_aEC22->put_DirectionForward((DirectionForward) ? VARIANT_TRUE : VARIANT_FALSE);
                m_aEC22->put_NormalizeOutput((SilEncConverters22::NormalizeFlags)NormalizeOutput);
            }
            else
            {
                // must no longer be in the repository!
                hrRet = /*NameNotFound*/ -7;
            }
        }
        else
        {
            hrRet = /* RegistryCorrupt */ -18;
        }
    }
    return hrRet;
}

HRESULT CSilEncConverter::AutoSelect()
{
    // if the converter *was* configured, then release it (since we're going to fetch a new one)
    Detach();

    // first see if we can access the new v3.0 interface
    IEC30s  pEC30s;

    CStringW strProgId;
    CRegKey keyRegEC;
    if (keyRegEC.Open(HKEY_CLASSES_ROOT, _T("SilEncConverters40.EncConverters"), KEY_READ) == ERROR_SUCCESS)
        strProgId = L"SilEncConverters40.EncConverters";
    else if (keyRegEC.Open(HKEY_CLASSES_ROOT, _T("SilEncConverters31.EncConverters"), KEY_READ) == ERROR_SUCCESS)
        strProgId = L"SilEncConverters31.EncConverters";
    else
        strProgId = L"SilEncConverters30.EncConverters";

    HRESULT hr = pEC30s.CoCreateInstance(strProgId);
	if (hr == 0x800401f0)
	{
		// means the calling program didn't call CoInitialize, so we need to do it here...
		CoInitialize ( NULL );
		pEC30s.CoCreateInstance(strProgId);
	}

    if( !!pEC30s )
    {
        // now that we have the repository... now ask for the Configuration UI
        // For the non-roman version, we're *probably* only looking for Unicode_to(_from)_Unicode converters, 
        // however, just in case the user wants to do a Legacy->Unicode (e.g. encoding conversion) simultaneously, 
        // leave the type ambiguous to allow for this possibility. For the Ansi version, though, Unicode is out, 
        // so limit the display of converters to only those that make sense.
        ECInterfaces::ConvType eConvType = ECInterfaces::ConvType_Unknown;   // this means show all converters

        // call the self-selection UI (NOTE: only in SC 2.2 and newer!)
        if(     (ProcessHResult(pEC30s->AutoSelect(eConvType, &m_aEC30), pEC30s, __uuidof(ECInterfaces::IEncConverters)) == S_OK)
            &&  !!m_aEC30 )
        {
            // get the name of the configured converter
            CComBSTR str;
            m_aEC30->get_Name(&str);  
            ConverterName = str;
            
            // get the direction
            VARIANT_BOOL bVal = VARIANT_TRUE;
            m_aEC30->get_DirectionForward(&bVal); 
            DirectionForward = (bVal ==  VARIANT_FALSE) ? FALSE : TRUE;

            // get the normalize output flag
            m_aEC30->get_NormalizeOutput((ECInterfaces::NormalizeFlags*)&NormalizeOutput);
            return S_OK;
        }
    }
    else
    {
        // otherwise, try the older 22 interface
        IEC22s  pEC22s;
        pEC22s.CoCreateInstance(L"SilEncConverters22.EncConverters");
        if (!!pEC22s)
        {
            // now that we have the repository... now ask for the Configuration UI
            // For the non-roman version, we're *probably* only looking for Unicode_to(_from)_Unicode converters, 
            // however, just in case the user wants to do a Legacy->Unicode (e.g. encoding conversion) simultaneously, 
            // leave the type ambiguous to allow for this possibility. For the Ansi version, though, Unicode is out, 
            // so limit the display of converters to only those that make sense.
            SilEncConverters22::ConvType eConvType22 = SilEncConverters22::ConvType_Unknown;   // this means show all converters

            // call the self-selection UI (NOTE: only in SC 2.2 and newer!)
            if(     (ProcessHResult(pEC22s->AutoSelect(eConvType22, &m_aEC22), pEC22s, __uuidof(SilEncConverters22::IEncConverters)) == S_OK)
                &&  !!m_aEC22 )
            {
                // get the name of the configured converter
                CComBSTR str;
                m_aEC22->get_Name(&str);  
                ConverterName = str;
                
                // get the direction
                VARIANT_BOOL bVal = VARIANT_TRUE;
                m_aEC22->get_DirectionForward(&bVal); 
                DirectionForward = (bVal ==  VARIANT_FALSE) ? FALSE : TRUE;

                // get the normalize output flag
                m_aEC22->get_NormalizeOutput((SilEncConverters22::NormalizeFlags*)&NormalizeOutput);
                return S_OK;
            }
        }
    }
    return /* RegistryCorrupt */ -18;
}

HRESULT CSilEncConverter::Add(
    const CStringW & converterName,
    const CStringW & converterSpec,
    int conversionType,
    const CStringW & leftEncoding,
    const CStringW & rightEncoding,
    int processType)
{
    Detach();

    // first see if we can access the new v3.0 interface
    IEC30s  pEC30s;
    CStringW strProgId;
    CRegKey keyRegEC;
    if (keyRegEC.Open(HKEY_CLASSES_ROOT, _T("SilEncConverters40.EncConverters"), KEY_READ) == ERROR_SUCCESS)
        strProgId = L"SilEncConverters40.EncConverters";
    else if (keyRegEC.Open(HKEY_CLASSES_ROOT, _T("SilEncConverters31.EncConverters"), KEY_READ) == ERROR_SUCCESS)
        strProgId = L"SilEncConverters31.EncConverters";
    else
        strProgId = L"SilEncConverters30.EncConverters";

    pEC30s.CoCreateInstance(strProgId);
    if( !!pEC30s )
    {
        if( !!m_aEC30 )
        {
            // converter has already been used
            return S_OK;
        }
        CComVariant varName(converterName);
        pEC30s->get_Item(varName, &m_aEC30);
        if( !!m_aEC30 )
        {
            // converter already exists in the database
            return S_OK;
        }
        CComBSTR varSpec(converterSpec);
        CComBSTR varLeftEnc(leftEncoding);
        CComBSTR varRightEnc(rightEncoding);
        if(ProcessHResult(pEC30s->Add(
                CComBSTR(converterName), varSpec, (ECInterfaces::ConvType)conversionType, varLeftEnc, varRightEnc, (ECInterfaces::ProcessTypeFlags) processType),
                pEC30s, __uuidof(ECInterfaces::IEncConverters)) != S_OK)
        {
            // An exception occurred.
            return /*ErrStatus.Exception*/ -6;
        }
        pEC30s->get_Item(varName, &m_aEC30);
        if(!m_aEC30 )
        {
            // must no longer be in the repository!
            return /*NameNotFound*/ -7;
        }
    }
    else
    {
        // otherwise, try the older 22 interface
        IEC22s  pEC22s;
        pEC22s.CoCreateInstance(L"SilEncConverters22.EncConverters");
        if (!!pEC22s)
        {
            if( !!m_aEC22 )
            {
                // converter has already been used
                return S_OK;
            }
            CComVariant varName(converterName);
            pEC22s->get_Item(varName, &m_aEC22);
            if( !!m_aEC22 )
            {
                // converter already exists in the database
                return S_OK;
            }
            CComBSTR varSpec(converterSpec);
            CComBSTR varLeftEnc(leftEncoding);
            CComBSTR varRightEnc(rightEncoding);
            if(ProcessHResult(pEC22s->Add(
                    CComBSTR(converterName), varSpec, (SilEncConverters22::ConvType)conversionType, varLeftEnc, varRightEnc, (SilEncConverters22::ProcessTypeFlags) processType),
                    pEC22s, __uuidof(ECInterfaces::IEncConverters)) != S_OK)
            {
                // An exception occurred.
                return /*ErrStatus.Exception*/ -6;
            }
            pEC22s->get_Item(varName, &m_aEC22);
            if(!m_aEC22 )
            {
                // must no longer be in the repository!
                return /*NameNotFound*/ -7;
            }
        }
        else
        {
            return /* RegistryCorrupt */ -18;
        }
    }
    return S_OK;
}

// here's a helper function for displaying useful error messages
HRESULT ProcessHResult(HRESULT hr, IUnknown* p, const IID& iid)
{
    if( hr == S_OK )
        return hr;

    // otherwise, throw a _com_issue_errorex and catch it (so we can use it to get
    //  the error description out of it for us.
    try 
    {
        _com_issue_errorex(hr, p, iid);
    }
    catch(_com_error & er) 
    {
        if( er.Description().length() > 0)
        {
            ::MessageBox(NULL, er.Description(), strCaption, MB_OK);
        } 
        else 
        {
            ::MessageBox(NULL, er.ErrorMessage(), strCaption, MB_OK);
        }
    }

    return hr;
}


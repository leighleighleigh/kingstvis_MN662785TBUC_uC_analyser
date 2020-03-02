#ifndef MN662785_ANALYZER_SETTINGS
#define MN662785_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class MN662785AnalyzerSettings : public AnalyzerSettings

{
public:
    MN662785AnalyzerSettings();
    virtual ~MN662785AnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings(const char *settings);
    virtual const char *SaveSettings();

    Channel mDataChannel;
    Channel mClockChannel;
    Channel mLoadChannel;
    // bool mUseAutobaud;

protected:
    std::auto_ptr< AnalyzerSettingInterfaceChannel >    mDataChannelInterface;
    std::auto_ptr< AnalyzerSettingInterfaceChannel >    mClockChannelInterface;
    std::auto_ptr< AnalyzerSettingInterfaceChannel >    mLoadChannelInterface;
    // std::auto_ptr< AnalyzerSettingInterfaceBool >   mUseAutobaudInterface;
};

#endif //MN662785_ANALYZER_SETTINGS

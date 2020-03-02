#include "MN662785AnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

#pragma warning(disable: 4800) //warning C4800: 'U32' : forcing value to bool 'true' or 'false' (performance warning)

MN662785AnalyzerSettings::MN662785AnalyzerSettings()
    :   mDataChannel(UNDEFINED_CHANNEL),
        mClockChannel(UNDEFINED_CHANNEL),
        mLoadChannel(UNDEFINED_CHANNEL)
{
    mDataChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
    mDataChannelInterface->SetTitleAndTooltip("MDATA", "Data Line");
    mDataChannelInterface->SetChannel(mDataChannel);

    mClockChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
    mClockChannelInterface->SetTitleAndTooltip("MCLK", "Clock Line");
    mClockChannelInterface->SetChannel(mClockChannel);

    mLoadChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
    mLoadChannelInterface->SetTitleAndTooltip("MLD", "Load Data Line");
    mLoadChannelInterface->SetChannel(mLoadChannel);

    AddInterface(mDataChannelInterface.get());
    AddInterface(mClockChannelInterface.get());
    AddInterface(mLoadChannelInterface.get());
    
    AddExportOption(0, "Export as text/csv file");
    AddExportExtension(0, "Text file", "txt");
    AddExportExtension(0, "CSV file", "csv");

    ClearChannels();
    AddChannel(mDataChannelInterface, "MDATA", false);
    AddChannel(mClockChannelInterface, "MCLK", false);
    AddChannel(mLoadChannelInterface, "MLD", false);
}

MN662785AnalyzerSettings::~MN662785AnalyzerSettings()
{
}

bool MN662785AnalyzerSettings::SetSettingsFromInterfaces()
{
    mDataChannel = mDataChannelInterface->GetChannel();
    mClockChannel = mClockChannelInterface->GetChannel();
    mLoadChannel = mLoadChannelInterface->GetChannel();
    
    ClearChannels();
    AddChannel(mDataChannel, "MDATA", true);
    AddChannel(mClockChannel, "MCLK", true);
    AddChannel(mLoadChannel, "MLD", true);

    return true;
}

void MN662785AnalyzerSettings::UpdateInterfacesFromSettings()
{
    mDataChannelInterface->SetChannel(mDataChannel);
    mClockChannelInterface->SetChannel(mClockChannel);
    mLoadChannelInterface->SetChannel(mLoadChannel);
}

void MN662785AnalyzerSettings::LoadSettings(const char *settings)
{
    SimpleArchive text_archive;
    text_archive.SetString(settings);

    const char *name_string;    //the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if (strcmp(name_string, "MN662785Analyzer") != 0) {
        AnalyzerHelpers::Assert("MN662785Analyzer: Provided with a settings string that doesn't belong to us;");
    }

    text_archive >> mDataChannel;
    text_archive >> mClockChannel;
    text_archive >> mLoadChannel;
    
    ClearChannels();
    AddChannel(mDataChannel, "MDATA", true);
    AddChannel(mClockChannel, "MCLK", true);
    AddChannel(mLoadChannel, "MLD", true);

    UpdateInterfacesFromSettings();
}

const char *MN662785AnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "MN662785Analyzer";
    text_archive << mDataChannel;
    text_archive << mClockChannel;
    text_archive << mLoadChannel;
    
    return SetReturnString(text_archive.GetString());
}

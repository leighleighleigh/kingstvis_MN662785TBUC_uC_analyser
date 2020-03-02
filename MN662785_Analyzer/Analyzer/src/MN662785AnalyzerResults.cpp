#include "MN662785AnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "MN662785Analyzer.h"
#include "MN662785AnalyzerSettings.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

MN662785AnalyzerResults::MN662785AnalyzerResults(MN662785Analyzer *analyzer, MN662785AnalyzerSettings *settings)
    :   AnalyzerResults(),
        mSettings(settings),
        mAnalyzer(analyzer)
{
}

MN662785AnalyzerResults::~MN662785AnalyzerResults()
{
}

void MN662785AnalyzerResults::GenerateBubbleText(U64 frame_index, Channel & /*channel*/, DisplayBase display_base)   //unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    Frame frame = GetFrame(frame_index);
    
    char number_str[128];
    AnalyzerHelpers::GetNumberString(frame.mData1,display_base,8,number_str,8);

    AddResultString(number_str);
}

void MN662785AnalyzerResults::GenerateExportFile(const char *file, DisplayBase display_base, U32 /*export_type_user_id*/)
{

}

void MN662785AnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
    ClearTabularText();
    Frame frame = GetFrame(frame_index);
    
    char number_str[128];
    AnalyzerHelpers::GetNumberString(frame.mData1,display_base,8,number_str,8);
    AddTabularText(number_str);
}

void MN662785AnalyzerResults::GeneratePacketTabularText(U64 /*packet_id*/, DisplayBase /*display_base*/)    //unrefereced vars commented out to remove warnings.
{
}

void MN662785AnalyzerResults::GenerateTransactionTabularText(U64 /*transaction_id*/, DisplayBase /*display_base*/)    //unrefereced vars commented out to remove warnings.
{

}

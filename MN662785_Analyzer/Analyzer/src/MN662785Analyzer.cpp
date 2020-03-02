#include "MN662785Analyzer.h"
#include "MN662785AnalyzerSettings.h"
#include <AnalyzerChannelData.h>

MN662785Analyzer::MN662785Analyzer()
    : Analyzer(),
      mSettings(new MN662785AnalyzerSettings()),
      mSimulationInitilized(false)
{
    SetAnalyzerSettings(mSettings.get());
}

MN662785Analyzer::~MN662785Analyzer()
{
    KillThread();
}

void MN662785Analyzer::SetupResults()
{
    //Unlike the worker thread, this function is called from the GUI thread
    //we need to reset the Results object here because it is exposed for direct access by the GUI, and it can't be deleted from the WorkerThread

    mResults.reset(new MN662785AnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());

    mResults->AddChannelBubblesWillAppearOn(mSettings->mDataChannel);
}

void MN662785Analyzer::WorkerThread()
{
    
}

bool MN662785Analyzer::NeedsRerun()
{
    
}

U32 MN662785Analyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    if (mSimulationInitilized == false) {
        mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index, device_sample_rate, simulation_channels);
}

U32 MN662785Analyzer::GetMinimumSampleRateHz()
{
    return 1;
}

const char *MN662785Analyzer::GetAnalyzerName() const
{
    return "MN662785";
}

const char *GetAnalyzerName()
{
    return "MN662785";
}

Analyzer *CreateAnalyzer()
{
    return new MN662785Analyzer();
}

void DestroyAnalyzer(Analyzer *analyzer)
{
    delete analyzer;
}


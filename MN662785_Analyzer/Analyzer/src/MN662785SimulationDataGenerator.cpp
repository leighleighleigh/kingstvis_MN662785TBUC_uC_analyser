#include "MN662785SimulationDataGenerator.h"
#include "MN662785AnalyzerSettings.h"

MN662785SimulationDataGenerator::MN662785SimulationDataGenerator()
{
}

MN662785SimulationDataGenerator::~MN662785SimulationDataGenerator()
{
}

void MN662785SimulationDataGenerator::Initialize(U32 simulation_sample_rate, MN662785AnalyzerSettings *settings)
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mClockGenerator.Init(simulation_sample_rate/200, simulation_sample_rate);
    
    if(settings->mDataChannel != UNDEFINED_CHANNEL)
    {
        mData = mMN662785SimulationChannels.Add(settings->mDataChannel, mSimulationSampleRateHz, BIT_LOW);
    }else{
        mData = NULL;
    }

    if(settings->mClockChannel != UNDEFINED_CHANNEL)
    {
        mClock = mMN662785SimulationChannels.Add(settings->mClockChannel,mSimulationSampleRateHz, BIT_LOW);
    }else{
        mClock = NULL;
    }

    if(settings->mLoadChannel != UNDEFINED_CHANNEL)
    {
        mLoad = mMN662785SimulationChannels.Add(settings->mLoadChannel,mSimulationSampleRateHz,BIT_LOW);
    }else{
        mLoad = NULL:
    }
}

U32 MN662785SimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

    while (mClock->GetCurrentSampleNumber() < adjusted_largest_sample_requested)
    {
        CreateMN662785Sequence(mClock->GetCurrentSampleNumber());

        mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(20.0));
    }

    *simulation_channels = mMN662785SimulationChannels.GetArray();
    return mMN662785SimulationChannels.GetCount();
}

void MN662785SimulationDataGenerator::CreateMN662785Sequence(U64 value)
{
    mData->Transition();
}

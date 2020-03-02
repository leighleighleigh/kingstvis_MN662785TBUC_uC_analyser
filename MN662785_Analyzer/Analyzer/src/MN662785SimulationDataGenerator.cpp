#include "MN662785SimulationDataGenerator.h"
#include "MN662785AnalyzerSettings.h"

MN662785SimulationDataGenerator::MN662785SimulationDataGenerator()
{
}

MN662785SimulationDataGenerator::~MN662785SimulationDataGenerator()
{
}

// Setup the channels
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
        mClock = mMN662785SimulationChannels.Add(settings->mClockChannel,mSimulationSampleRateHz, BIT_HIGH);
    }else{
        mClock = NULL;
    }

    if(settings->mLoadChannel != UNDEFINED_CHANNEL)
    {
        mLoad = mMN662785SimulationChannels.Add(settings->mLoadChannel,mSimulationSampleRateHz,BIT_HIGH);
    }else{
        mLoad = NULL;
    }
}

// Generate the data!
U32 MN662785SimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    // Get the amount of samples we need to make
    U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

    // While we are below this sample target, we generate data!
    while (mClock->GetCurrentSampleNumber() < adjusted_largest_sample_requested)
    {
        // Move all of the channels forward by 20 half periods or whatever
        mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(20.0));

        // Create the sequence
        CreateMN662785Sequence();
    }

    *simulation_channels = mMN662785SimulationChannels.GetArray();
    return mMN662785SimulationChannels.GetCount();
}

void MN662785SimulationDataGenerator::WriteMN662785Byte(U8 data)
{
    // Get those bits
    BitExtractor mBits = BitExtractor(data,AnalyzerEnums::ShiftOrder::MsbFirst,8);
    // BitState mBit = mBits.GetNextBit();

    // Write the 8 bits, I guess
    for(uint8_t i = 0; i<8; i++)
    {
        mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(1));
        mClock->TransitionIfNeeded(BIT_LOW);    
        mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(1));
        mData->TransitionIfNeeded(mBits.GetNextBit());
        mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(1));
        mClock->TransitionIfNeeded(BIT_HIGH);
    }

    // Gap between bytes
    mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(2));

}

void MN662785SimulationDataGenerator::CreateMN662785Sequence()
{
    // Write three bytes to the data line, then set MLD low for a bit.
    mLoad->TransitionIfNeeded(BIT_HIGH);
    WriteMN662785Byte(0x10);
    WriteMN662785Byte(0x20);
    WriteMN662785Byte(0x30);
    mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(4));
    mLoad->TransitionIfNeeded(BIT_LOW);
    mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(8));
    mLoad->TransitionIfNeeded(BIT_HIGH);
    mMN662785SimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(4));
}
